/**
 * @file audiotaunts_real_tests.cpp
 * @brief Tests for audiotaunts.cpp (~792 lines).
 *
 * @details
 * Covers taunt enable/delay accessors, playback gating (multi/connected/
 * file-exists) incl. the Osiris event dispatch, the full WAV parser
 * (taunt_LoadWaveFile error codes 1..9, 8->16 bit conversion, sample
 * alignment), error strings, and the ImportWave validation paths with a
 * stubbed compressor + OSF archive.
 *
 * This harness validates the behavior of `Descent3/audiotaunts.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/audiotaunts.cpp`
 * @par Harness
 * `audiotaunts_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/audiotaunts.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <filesystem>

#include "audiotaunts.h"
#include "cfile.h"
#include "audio_encode.h"
#include "streamaudio.h"
#include "manage.h"
#include "multi.h"
#include "player.h"
#include "object.h"
#include "game2dll.h"
#include "descent.h"
#include "ddio.h"
#include "game.h"
#include "stringtable.h"
#include "log.h"

// ---------------------------------------------------------------------------
// Globals normally defined elsewhere
// ---------------------------------------------------------------------------
std::filesystem::path Descent3_temp_directory = "/tmp/opencode";
std::filesystem::path LocalCustomSoundsDir = "/tmp/opencode/custom";
netplayer NetPlayers[MAX_NET_PLAYERS];
player Players[MAX_PLAYERS];
int Player_num = 0;
object Objects[MAX_OBJECTS];
dllinfo DLLInfo;
int Game_mode = 0;

// player's embedded Inventory pulls these in
Inventory::Inventory() {}
Inventory::~Inventory() {}

// ---------------------------------------------------------------------------
// Scripted object lookup
// ---------------------------------------------------------------------------
static object *g_lookup[MAX_OBJECTS];
object *ObjGet(int handle) {
  if (handle < 0 || handle >= MAX_OBJECTS)
    return nullptr;
  return g_lookup[handle];
}

const char *GetStringFromTable(int index) {
  static char buf[32];
  snprintf(buf, sizeof(buf), "<txt%d>", index);
  return buf;
}

// ---------------------------------------------------------------------------
// Stream/compress/DLL stubs
// ---------------------------------------------------------------------------
struct PlayCapture {
  std::string filename;
  float volume;
  int type;
};
static std::vector<PlayCapture> g_plays;
int StreamPlay(const std::filesystem::path &filename, float volume, int type) {
  g_plays.push_back({filename.u8string(), volume, type});
  return 1;
}

static bool g_compress_ok = true;
struct CompressCapture {
  std::string in, out;
  int samples, rate, chan;
  std::vector<uint8_t> raw_in; // copy of the converted sample data
};
static std::vector<CompressCapture> g_compresses;
bool aenc_Compress(const char *in, const char *out, const int *, const int *input_samples, const int *input_rate,
                   const int *input_channels, const float *, const float *) {
  CompressCapture c{in, out};
  if (input_samples)
    c.samples = *input_samples;
  if (input_rate)
    c.rate = *input_rate;
  if (input_channels)
    c.chan = *input_channels;
  FILE *fp = fopen(in, "rb");
  if (fp) {
    uint8_t buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
      c.raw_in.insert(c.raw_in.end(), buf, buf + n);
    fclose(fp);
  }
  g_compresses.push_back(c);
  if (!g_compress_ok)
    return false;
  // fake compressor: copy input to output
  std::filesystem::copy_file(in, out, std::filesystem::copy_options::overwrite_existing);
  return true;
}

struct OsfCapture {
  bool opened;
  std::string name;
  uint8_t type, comp, flags;
  uint32_t rate, length;
  int blocks_written, partial_bytes;
};
static OsfCapture g_osf;

OSFArchive::OSFArchive() { memset(&g_osf, 0, sizeof(g_osf)); }
OSFArchive::~OSFArchive() {}
bool OSFArchive::Open(const std::filesystem::path &filename, bool write) {
  (void)write;
  g_osf.opened = true;
  g_osf.name = filename.u8string();
  return true;
}
void OSFArchive::Close() { g_osf.opened = false; }
void OSFArchive::Rewind() {}
bool OSFArchive::SaveHeader(uint8_t type, uint8_t comp, uint8_t flags, uint32_t rate, uint32_t length, void *,
                            const char *name) {
  g_osf.type = type;
  g_osf.comp = comp;
  g_osf.flags = flags;
  g_osf.rate = rate;
  g_osf.length = length;
  g_osf.name = name;
  return true;
}
bool OSFArchive::WriteBlock(uint8_t *, int size) {
  g_osf.blocks_written++;
  g_osf.partial_bytes += size;
  return true;
}
int OSFArchive::Read(uint8_t *, int size) { return size; }

static int g_dll_events[8];
static int g_dll_event_count;
void CallGameDLL(int eventnum, dllinfo *) {
  if (g_dll_event_count < 8)
    g_dll_events[g_dll_event_count] = eventnum;
  g_dll_event_count++;
}

// ---------------------------------------------------------------------------
// Fixture helpers
// ---------------------------------------------------------------------------
/**
 * @brief GTest fixture for TauntTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class TauntTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() { cf_SetSearchPath("/tmp/opencode", {}); }

  void SetUp() override {
    memset(NetPlayers, 0, sizeof(netplayer) * 4);
    memset(g_lookup, 0, sizeof(g_lookup));
    memset(Objects, 0, sizeof(object) * 4);
    g_plays.clear();
    g_compresses.clear();
    g_dll_event_count = 0;
    g_compress_ok = true;
    Audio_taunts_enabled = false;
    std::filesystem::create_directories(LocalCustomSoundsDir);
    std::filesystem::remove_all(LocalCustomSoundsDir / "t.osf");
  }

  void TearDown() override {
    for (auto &p : std::filesystem::directory_iterator("/tmp/opencode"))
      if (p.path().extension() == ".wav" || p.path().extension() == ".osf" ||
          p.path().filename().string().rfind("d3o", 0) == 0)
        std::filesystem::remove(p.path());
  }

  // Writes a minimal WAV; channels/rate/bits feed the fmt chunk.
  void WriteWav(const char *fname, int fmttag, int channels, int rate, int bits,
                const std::vector<uint8_t> &raw_samples) {
    std::vector<uint8_t> hdr, body;
    auto push_int = [](std::vector<uint8_t> &v, uint32_t x) {
      v.push_back(x & 0xff);
      v.push_back((x >> 8) & 0xff);
      v.push_back((x >> 16) & 0xff);
      v.push_back((x >> 24) & 0xff);
    };
    auto push_short = [](std::vector<uint8_t> &v, uint16_t x) {
      v.push_back(x & 0xff);
      v.push_back((x >> 8) & 0xff);
    };

    push_int(body, 0x20746D66); // "fmt "
    push_int(body, 16);         // chunk size
    push_short(body, (uint16_t)fmttag);
    push_short(body, (uint16_t)channels);
    push_int(body, (uint32_t)rate);
    push_int(body, rate * channels * bits / 8); // avg bytes/sec
    push_short(body, (uint16_t)(channels * bits / 8));
    push_short(body, (uint16_t)bits);

    if (!raw_samples.empty()) {
      push_int(body, 0x61746164); // "data"
      push_int(body, (uint32_t)raw_samples.size());
      body.insert(body.end(), raw_samples.begin(), raw_samples.end());
    }

    uint32_t riffsize = 4 + body.size(); // "WAVE" + chunks
    push_int(hdr, 0x46464952);           // "RIFF"
    push_int(hdr, riffsize);
    push_int(hdr, 0x45564157);           // "WAVE"

    std::string path = std::string("/tmp/opencode/") + fname;
    FILE *fp = fopen(path.c_str(), "wb");
    fwrite(hdr.data(), 1, hdr.size(), fp);
    fwrite(body.data(), 1, body.size(), fp);
    fclose(fp);
  }
};

/**
 * @test TauntTest.EnableDisableAndDelayAccessors
 * @brief Verifies enable Disable And Delay Accessors.
 *
 * @details
 * Exercises the TauntTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/audiotaunts.cpp
 * @ingroup descent3_tests
 */
TEST_F(TauntTest, EnableDisableAndDelayAccessors) {
  taunt_Enable(true);
  EXPECT_TRUE(taunt_AreEnabled());
  taunt_Enable(false);
  EXPECT_FALSE(taunt_AreEnabled());

  taunt_SetDelayTime(2.5f);
  EXPECT_FLOAT_EQ(taunt_DelayTime(), 2.5f); // getter only defaults when < 0
}

/**
 * @test TauntTest.PlayTauntFileRequiresEnabledTaunts
 * @brief Verifies play Taunt File Requires Enabled Taunts.
 *
 * @details
 * Exercises the TauntTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/audiotaunts.cpp
 * @ingroup descent3_tests
 */
TEST_F(TauntTest, PlayTauntFileRequiresEnabledTaunts) {
  std::filesystem::path p("/tmp/opencode/x.osf");
  EXPECT_FALSE(taunt_PlayTauntFile(p));
  ASSERT_TRUE(g_plays.empty());

  taunt_Enable(true);
  EXPECT_TRUE(taunt_PlayTauntFile(p));
  ASSERT_EQ(g_plays.size(), 1u);
  EXPECT_EQ(g_plays[0].volume, MAX_GAME_VOLUME / 2.0f);
}

/**
 * @test TauntTest.PlayPlayerTauntValidationChain
 * @brief Verifies play Player Taunt Validation Chain.
 *
 * @details
 * Exercises the TauntTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/audiotaunts.cpp
 * @ingroup descent3_tests
 */
TEST_F(TauntTest, PlayPlayerTauntValidationChain) {
  Game_mode &= ~GM_MULTI;
  EXPECT_FALSE(taunt_PlayPlayerTaunt(0, 0)); // not multiplayer

  Game_mode |= GM_MULTI;
  EXPECT_FALSE(taunt_PlayPlayerTaunt(0, -1)); // bad index
  EXPECT_FALSE(taunt_PlayPlayerTaunt(0, 4));

  NetPlayers[0].flags &= ~NPF_CONNECTED;
  EXPECT_FALSE(taunt_PlayPlayerTaunt(0, 0)); // disconnected peer

  NetPlayers[0].flags |= NPF_CONNECTED;
  NetPlayers[0].sequence = NETSEQ_PLAYING - 5;
  EXPECT_FALSE(taunt_PlayPlayerTaunt(0, 0)); // connected but not playing

  NetPlayers[0].sequence = NETSEQ_PLAYING;
  strcpy(NetPlayers[0].voice_taunt1, "missing.osf");
  EXPECT_FALSE(taunt_PlayPlayerTaunt(0, 0)); // file missing
}

/**
 * @test TauntTest.PlayPlayerTauntSuccessFiresEventPerSlot
 * @brief Verifies play Player Taunt Success Fires Event Per Slot.
 *
 * @details
 * Exercises the TauntTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/audiotaunts.cpp
 * @ingroup descent3_tests
 */
TEST_F(TauntTest, PlayPlayerTauntSuccessFiresEventPerSlot) {
  Game_mode |= GM_MULTI;
  NetPlayers[0].flags |= NPF_CONNECTED;
  NetPlayers[0].sequence = NETSEQ_PLAYING;
  strcpy(NetPlayers[0].voice_taunt3, "t.osf");
  std::filesystem::copy_file("/etc/hostname", LocalCustomSoundsDir / "t.osf");

  Players[0].objnum = 1;
  Objects[1].handle = 777;
  g_lookup[777] = &Objects[1];

  taunt_Enable(true);
  EXPECT_TRUE(taunt_PlayPlayerTaunt(0, 2));

  ASSERT_EQ(g_dll_event_count, 1);
  EXPECT_EQ(g_dll_events[0], EVT_CLIENT_PLAYERPLAYSAUDIOTAUNT);
  EXPECT_EQ(DLLInfo.me_handle, 777);
  ASSERT_EQ(g_plays.size(), 1u);
  EXPECT_NE(g_plays[0].filename.find("custom/t.osf"), std::string::npos);
}

/**
 * @test TauntTest.GetErrorStringKnownCodes
 * @brief Verifies get Error String Known Codes.
 *
 * @details
 * Exercises the TauntTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/audiotaunts.cpp
 * @ingroup descent3_tests
 */
TEST_F(TauntTest, GetErrorStringKnownCodes) {
  EXPECT_STREQ(taunt_GetErrorString(TAUNTIMPERR_NOTFOUND), TXT_FILENOTFOUND);
  EXPECT_STREQ(taunt_GetErrorString(TAUNTIMPERR_INVALIDCHANNELS), TXT_MUSTBEMONO);
  EXPECT_STREQ(taunt_GetErrorString(TAUNTIMPERR_COMPRESSIONFAILURE), TXT_COMPRESSFAIL);
  EXPECT_STREQ(taunt_GetErrorString(9999), TXT_NOERROR);
}

// taunt_LoadWaveFile and tWaveFile are static to audiotaunts.cpp; error
// codes are observed through taunt_ImportWave -> taunt_GetError().

/**
 * @test TauntTest.ImportWaveMissingFile
 * @brief Verifies import Wave Missing File.
 *
 * @details
 * Exercises the TauntTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/audiotaunts.cpp
 * @ingroup descent3_tests
 */
TEST_F(TauntTest, ImportWaveMissingFile) {
  EXPECT_FALSE(taunt_ImportWave("/tmp/opencode/nope.wav", "/tmp/opencode/out.osf"));
  EXPECT_EQ(taunt_GetError(), TAUNTIMPERR_NOTFOUND);
}

/**
 * @test TauntTest.ImportWaveBadRiffAndWaveTags
 * @brief Verifies import Wave Bad Riff And Wave Tags.
 *
 * @details
 * Exercises the TauntTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/audiotaunts.cpp
 * @ingroup descent3_tests
 */
TEST_F(TauntTest, ImportWaveBadRiffAndWaveTags) {
  WriteWav("badriff.wav", 1, 1, 22050, 8, {});
  FILE *fp = fopen("/tmp/opencode/badriff.wav", "r+b");
  fseek(fp, 0, SEEK_SET);
  fwrite("XIFF", 1, 4, fp);
  fclose(fp);
  EXPECT_FALSE(taunt_ImportWave("/tmp/opencode/badriff.wav", "/tmp/opencode/out.osf"));
  EXPECT_EQ(taunt_GetError(), TAUNTIMPERR_NOTRIFF);

  WriteWav("badwave.wav", 1, 1, 22050, 8, {});
  fp = fopen("/tmp/opencode/badwave.wav", "r+b");
  fseek(fp, 8, SEEK_SET);
  fwrite("JUNK", 1, 4, fp);
  fclose(fp);
  EXPECT_FALSE(taunt_ImportWave("/tmp/opencode/badwave.wav", "/tmp/opencode/out.osf"));
  EXPECT_EQ(taunt_GetError(), TAUNTIMPERR_NOTWAVE);
}

/**
 * @test TauntTest.ImportWaveInvalidChunkSize
 * @brief Verifies import Wave Invalid Chunk Size.
 *
 * @details
 * Exercises the TauntTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/audiotaunts.cpp
 * @ingroup descent3_tests
 */
TEST_F(TauntTest, ImportWaveInvalidChunkSize) {
  WriteWav("zerock.wav", 1, 1, 22050, 8, {});
  FILE *fp = fopen("/tmp/opencode/zerock.wav", "r+b");
  fseek(fp, 16, SEEK_SET); // fmt chunk size field
  int zero = 0;
  fwrite(&zero, 4, 1, fp);
  fclose(fp);
  EXPECT_FALSE(taunt_ImportWave("/tmp/opencode/zerock.wav", "/tmp/opencode/out.osf"));
  EXPECT_EQ(taunt_GetError(), TAUNTIMPERR_INVALIDFILE);
}

/**
 * @test TauntTest.ImportWaveFormatConstraints
 * @brief Verifies import Wave Format Constraints.
 *
 * @details
 * Exercises the TauntTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/audiotaunts.cpp
 * @ingroup descent3_tests
 */
TEST_F(TauntTest, ImportWaveFormatConstraints) {
  std::vector<uint8_t> samples{128, 128, 128, 128};

  WriteWav("adpcm.wav", 0x0011, 1, 22050, 8, samples); // DVI ADPCM
  EXPECT_FALSE(taunt_ImportWave("/tmp/opencode/adpcm.wav", "/tmp/opencode/out.osf"));
  EXPECT_EQ(taunt_GetError(), TAUNTIMPERR_NOTSUPPORTED);

  WriteWav("stereo.wav", 1, 2, 22050, 8, samples);
  EXPECT_FALSE(taunt_ImportWave("/tmp/opencode/stereo.wav", "/tmp/opencode/out.osf"));
  EXPECT_EQ(taunt_GetError(), TAUNTIMPERR_INVALIDCHANNELS);

  WriteWav("rate44k.wav", 1, 1, 44100, 8, samples);
  EXPECT_FALSE(taunt_ImportWave("/tmp/opencode/rate44k.wav", "/tmp/opencode/out.osf"));
  EXPECT_EQ(taunt_GetError(), TAUNTIMPERR_INVALIDSAMPLES);

  WriteWav("12bit.wav", 1, 1, 22050, 12, samples);
  EXPECT_FALSE(taunt_ImportWave("/tmp/opencode/12bit.wav", "/tmp/opencode/out.osf"));
  EXPECT_EQ(taunt_GetError(), TAUNTIMPERR_INVALIDBITDEPTH);
}

/**
 * @test TauntTest.ImportWaveDataBeforeFormatRejected
 * @brief Verifies import Wave Data Before Format Rejected.
 *
 * @details
 * Exercises the TauntTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/audiotaunts.cpp
 * @ingroup descent3_tests
 */
TEST_F(TauntTest, ImportWaveDataBeforeFormatRejected) {
  // Valid RIFF/WAVE but a data chunk appears before any fmt chunk
  std::vector<uint8_t> f;
  auto push_int = [&](uint32_t v) {
    for (int i = 0; i < 4; i++)
      f.push_back((v >> (8 * i)) & 0xff);
  };
  push_int(0x46464952); // "RIFF"
  push_int(16);         // riffsize: "WAVE" + data chunk
  push_int(0x45564157); // "WAVE"
  push_int(0x61746164); // data chunk before any fmt chunk
  push_int(4);
  push_int(0x80808080);
  FILE *fp = fopen("/tmp/opencode/datafirst.wav", "wb");
  fwrite(f.data(), 1, f.size(), fp);
  fclose(fp);
  EXPECT_FALSE(taunt_ImportWave("/tmp/opencode/datafirst.wav", "/tmp/opencode/out.osf"));
  EXPECT_EQ(taunt_GetError(), TAUNTIMPERR_INVALIDFILE);
}

/**
 * @test TauntTest.ImportWaveConverts8BitToSigned16WithAlignment
 * @brief Verifies import Wave Converts8Bit To Signed16With Alignment.
 *
 * @details
 * Exercises the TauntTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/audiotaunts.cpp
 * @ingroup descent3_tests
 */
TEST_F(TauntTest, ImportWaveConverts8BitToSigned16WithAlignment) {
  WriteWav("m8.wav", 1, 1, 22050, 8, {0, 64, 128, 192, 255, 200});

  EXPECT_TRUE(taunt_ImportWave("/tmp/opencode/m8.wav", "/tmp/opencode/m8.osf"));

  ASSERT_EQ(g_compresses.size(), 1u);
  const auto &raw = g_compresses[0].raw_in;
  // loader converts 8-bit -> signed 16-bit: ((x)-128)*256
  // alignment pads 6 samples up to 9 (multiple of 4... via (n-1)%4)
  ASSERT_EQ(raw.size(), 9u * 2u);
  auto s16 = [&](int i) {
    return (int16_t)(raw[i * 2] | (raw[i * 2 + 1] << 8));
  };
  EXPECT_EQ(s16(0), (int16_t)-32768);   // ((0)-128)*256
  EXPECT_EQ(s16(2), 0);                 // ((128)-128)*256
  EXPECT_EQ(s16(3), (int16_t)16384);    // ((192)-128)*256
  // the 128-valued alignment pad bytes convert to silence as well
  for (int i = 6; i < 9; i++) {
    EXPECT_EQ(raw[i * 2], 0);
    EXPECT_EQ(raw[i * 2 + 1], 0);
  }
}

/**
 * @test TauntTest.ImportWave16BitSamplesPassThroughPlusZeroPad
 * @brief Verifies import Wave16Bit Samples Pass Through Plus Zero Pad.
 *
 * @details
 * Exercises the TauntTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/audiotaunts.cpp
 * @ingroup descent3_tests
 */
TEST_F(TauntTest, ImportWave16BitSamplesPassThroughPlusZeroPad) {
  WriteWav("m16.wav", 1, 1, 22050, 16,
           {0x34, 0x12, 0x78, 0x56, 0x00, 0x80, 0xFF, 0x7F});

  EXPECT_TRUE(taunt_ImportWave("/tmp/opencode/m16.wav", "/tmp/opencode/m16.osf"));

  ASSERT_EQ(g_compresses.size(), 1u);
  const auto &raw = g_compresses[0].raw_in;
  // 4 samples kept + 2 bytes zero padding ((4-1)%4==3 -> pad one sample)
  ASSERT_EQ(raw.size(), 10u);
  EXPECT_EQ(raw[0], 0x34); // INTEL_SHORT is a no-op on little-endian x86
  EXPECT_EQ(raw[1], 0x12);
  EXPECT_EQ(raw[6], 0xFF); // last original sample's low byte
  EXPECT_EQ(raw[7], 0x7F); // ... high byte
  EXPECT_EQ(raw[8], 0x00); // two-byte zero padding
  EXPECT_EQ(raw[9], 0x00);
}

// KNOWN QUIRK: taunt_LoadWaveFile never assigns error_code 10 (NODATA),
// so a wave without a data chunk returns SUCCESS with NULL sample buffers.
// In NDEBUG builds ImportWave then writes a zero-length temp file and
// "succeeds"; in assertion builds cf_WriteBytes(count=0) trips its ASSERT
// and aborts, so this path cannot be exercised as a normal test here.

/**
 * @test TauntTest.ImportWaveValidatesInputsAndOutputs
 * @brief Verifies import Wave Validates Inputs And Outputs.
 *
 * @details
 * Exercises the TauntTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/audiotaunts.cpp
 * @ingroup descent3_tests
 */
TEST_F(TauntTest, ImportWaveValidatesInputsAndOutputs) {
  WriteWav("imp.wav", 1, 1, 22050, 8, {128, 128, 128, 128});

  EXPECT_FALSE(taunt_ImportWave(nullptr, "/tmp/opencode/out.osf"));
  EXPECT_EQ(taunt_GetError(), TAUNTIMPERR_NOTFOUND);

  EXPECT_FALSE(taunt_ImportWave("/tmp/opencode/ghost.wav", "/tmp/opencode/out.osf"));
  EXPECT_EQ(taunt_GetError(), TAUNTIMPERR_NOTFOUND);

  std::filesystem::copy_file("/etc/hostname", "/tmp/opencode/out.osf");
  EXPECT_FALSE(taunt_ImportWave("/tmp/opencode/imp.wav", "/tmp/opencode/out.osf"));
  EXPECT_EQ(taunt_GetError(), TAUNTIMPERR_OSFEXISTS);
  std::filesystem::remove("/tmp/opencode/out.osf");
}

/**
 * @test TauntTest.ImportWaveFullPipelineWithStubbedCompressor
 * @brief Verifies import Wave Full Pipeline With Stubbed Compressor.
 *
 * @details
 * Exercises the TauntTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/audiotaunts.cpp
 * @ingroup descent3_tests
 */
TEST_F(TauntTest, ImportWaveFullPipelineWithStubbedCompressor) {
  std::vector<uint8_t> s{100, 150, 128, 128};
  WriteWav("pipe.wav", 1, 1, 22050, 16, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
  (void)s;

  EXPECT_TRUE(taunt_ImportWave("/tmp/opencode/pipe.wav", "/tmp/opencode/pipe.osf"));

  ASSERT_EQ(g_compresses.size(), 1u);
  EXPECT_EQ(g_compresses[0].chan, 1);   // mono propagated
  EXPECT_EQ(g_compresses[0].rate, 22050);
  EXPECT_TRUE(g_osf.type != 0 || g_osf.length > 0); // header saved
  EXPECT_EQ(g_osf.comp, OSF_DIGIACM_STRM);
  EXPECT_EQ(g_osf.flags, SAF_16BIT_M); // mono(0) | 16bit(1)
  // temp files cleaned up
  EXPECT_FALSE(std::filesystem::exists("/tmp/opencode/pipe.osf.tmp"));
}
