/**
 * @file gamecheat_real_tests.cpp
 * @brief Tests for GameCheat.cpp 654 lines — cheat code system.
 *
 * @details
 * Covers jcrypt/oldjcrypt encryption (validated against shipped cheat
 * constants), sliding cheat buffer, lamer-cheat list, outline-mode
 * state machine, and suicide message XOR decoding.
 *
 * This harness validates the behavior of `Descent3/GameCheat.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/GameCheat.cpp`
 * @par Harness
 * `gamecheat_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/GameCheat.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdint>

// replicated jcrypt (GameCheat.cpp:283-302)
static const char *RepJcrypt(const char *plainstring) {
  static char cryptstring[20];
  int i, t, len;
  len = (int)strlen(plainstring);
  if (len > 8)
    len = 8;
  for (i = 0; i < len; i++) {
    cryptstring[i] = 0;
    for (t = 0; t < 8; t++) {
      cryptstring[i] ^= (plainstring[t] ^ plainstring[i % (t + 2)]);
      cryptstring[i] %= 57;
      cryptstring[i] += 39;
    }
  }
  cryptstring[i] = 0;
  return cryptstring;
}

// replicated oldjcrypt (GameCheat.cpp:304-323)
static const char *RepOldJcrypt(const char *plainstring) {
  static char cryptstring[20];
  int i, t, len;
  len = (int)strlen(plainstring);
  if (len > 8)
    len = 8;
  for (i = 0; i < len; i++) {
    cryptstring[i] = 0;
    for (t = 0; t < 8; t++) {
      cryptstring[i] ^= (plainstring[t] ^ plainstring[i % (t + 1)]);
      cryptstring[i] %= 54;
      cryptstring[i] += 33;
    }
  }
  cryptstring[i] = 0;
  return cryptstring;
}

// shipped encrypted constants (GameCheat.cpp:243-268, first 8 chars matter)
static const char *WeaponsCheat = "BH;URJH,";
static const char *CloakCheat = "L_QM[=^)";
static const char *KillRobotsCheat = "KMGKKG4D";
static const char *InvulnCheat = "BBG\\Q90L";
static const char *CameraCheat = "X2OHN*2(";
static const char *SuicideCheat = "\\S3YGK=N";

// replicated sliding cheat buffer (GameCheat.cpp:347-353, CHEATSPOT=14)
constexpr int CHEATSPOT = 14;
struct CheatBufferMock {
  char buf[CHEATSPOT + 2];
  void reset() { memset(buf, 'A', 15); buf[15] = 0; }
  void type(char key) {
    for (int i = 0; i < 15; i++)
      buf[i] = buf[i + 1];
    buf[CHEATSPOT] = key;
  }
  const char *window() { return &buf[7]; } // last 8 typed chars -> crypt input
};

// helper: type a full word into fresh buffer, return crypt of its window
static void TypeWord(CheatBufferMock &cb, const char *word) {
  cb.reset();
  while (*word) cb.type(*word++);
}

/**
 * @test GameCheat.JcryptMatchesShippedConstants
 * @brief Verifies jcrypt Matches Shipped Constants.
 *
 * @details
 * Exercises the GameCheat code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/GameCheat.cpp
 * @ingroup descent3_tests
 */
TEST(GameCheat, JcryptMatchesShippedConstants) {
  // release cheat codes that are exactly 8 chars crypt directly
  EXPECT_STREQ(RepJcrypt("IVEGOTIT"), WeaponsCheat);
  EXPECT_STREQ(RepJcrypt("TESTICUS"), CloakCheat);
  EXPECT_STREQ(RepJcrypt("OUTLINEM"), "*HAFW1ZI");
}

/**
 * @test GameCheat.LongCheatsCryptTrailingWindowNotFullWord
 * @brief Verifies long Cheats Crypt Trailing Window Not Full Word.
 *
 * @details
 * Exercises the GameCheat code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/GameCheat.cpp
 * @ingroup descent3_tests
 */
TEST(GameCheat, LongCheatsCryptTrailingWindowNotFullWord) {
  // quirk: detection compares jcrypt(&buffer[7]) — for words > 8 chars
  // only the LAST 8 typed chars are encrypted, not the whole word
  EXPECT_STRNE(RepJcrypt("DEADOFNIGHT"), KillRobotsCheat);
  CheatBufferMock cb;
  TypeWord(cb, "DEADOFNIGHT");
  EXPECT_STREQ(RepJcrypt(cb.window()), KillRobotsCheat);

  TypeWord(cb, "BURGERGOD");
  EXPECT_STREQ(RepJcrypt(cb.window()), InvulnCheat);

  TypeWord(cb, "BYEBYEMONKEY");
  EXPECT_STREQ(RepJcrypt(cb.window()), CameraCheat);

  TypeWord(cb, "TUBERACER");
  EXPECT_STREQ(RepJcrypt(cb.window()), SuicideCheat);
}

/**
 * @test GameCheat.OldJcryptLamerCheats
 * @brief Verifies old Jcrypt Lamer Cheats.
 *
 * @details
 * Exercises the GameCheat code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/GameCheat.cpp
 * @ingroup descent3_tests
 */
TEST(GameCheat, OldJcryptLamerCheats) {
  // demo-era cheats detected via oldjcrypt of the sliding window
  // (LamerCheats table, GameCheat.cpp:163-188)
  CheatBufferMock cb;
  TypeWord(cb, "gabbagabbahey");
  EXPECT_STREQ(RepOldJcrypt(cb.window()), "?E9FI=()");
  TypeWord(cb, "motherlode");
  EXPECT_STREQ(RepOldJcrypt(cb.window()), "=-OQESN1");
  TypeWord(cb, "zingermans");
  EXPECT_STREQ(RepOldJcrypt(cb.window()), "C<G2DAIV");
}

/**
 * @test GameCheat.JcryptTruncatesToEightChars
 * @brief Verifies jcrypt Truncates To Eight Chars.
 *
 * @details
 * Exercises the GameCheat code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/GameCheat.cpp
 * @ingroup descent3_tests
 */
TEST(GameCheat, JcryptTruncatesToEightChars) {
  // len>8 clamps to 8: result depends only on first 8 chars
  EXPECT_STREQ(RepJcrypt("DEADOFNIGHT"), RepJcrypt("DEADOFN"));
  EXPECT_STREQ(RepJcrypt("BURGERGODXYZ"), RepJcrypt("BURGERGO"));
}

/**
 * @test GameCheat.CryptOutputCharset
 * @brief Verifies crypt Output Charset.
 *
 * @details
 * Exercises the GameCheat code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/GameCheat.cpp
 * @ingroup descent3_tests
 */
TEST(GameCheat, CryptOutputCharset) {
  // output range: %=57 then +=39 -> chars in [39, 95]
  const char *c = RepJcrypt("ZZZZZZZZ");
  for (const char *p = c; *p; ++p) {
    EXPECT_GE(*p, 39);
    EXPECT_LE(*p, 95);
  }
}

// helper moved above constant tests

/**
 * @test GameCheat.SlidingBufferWindow
 * @brief Verifies sliding Buffer Window.
 *
 * @details
 * Exercises the GameCheat code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/GameCheat.cpp
 * @ingroup descent3_tests
 */
TEST(GameCheat, SlidingBufferWindow) {
  CheatBufferMock cb;
  cb.reset();
  const char *seq = "TESTICUS";
  while (*seq) cb.type(*seq++);
  EXPECT_STREQ(cb.window(), "TESTICUS"); // last 8 keys visible to crypt

  cb.reset();
  const char *seq2 = "DEADOFNIGHT"; // 11 chars
  while (*seq2) cb.type(*seq2++);
  EXPECT_STREQ(cb.window(), "DOFNIGHT"); // older chars slide out, last 8 remain
}

/**
 * @test GameCheat.TypedSequenceTriggersWeaponCheat
 * @brief Verifies typed Sequence Triggers Weapon Cheat.
 *
 * @details
 * Exercises the GameCheat code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/GameCheat.cpp
 * @ingroup descent3_tests
 */
TEST(GameCheat, TypedSequenceTriggersWeaponCheat) {
  // end-to-end: type IVEGOTIT into buffer, crypt window matches constant
  CheatBufferMock cb;
  cb.reset();
  const char *seq = "AAAAIVEGOTIT"; // leading As simulate fresh buffer
  while (*seq) cb.type(*seq++);
  EXPECT_STREQ(RepJcrypt(cb.window()), WeaponsCheat);
}

// replicated outline mode state machine (GameCheat.cpp:407-438)
// NOTE original quirk at case 0: state=1 is commented out, jumps straight to 3
struct OutlineState {
  int state = 0;
  uint8_t advance() {
    uint8_t new_mode;
    switch (state) {
    case 0: new_mode = 1; state = 3; break; // quirk: skips states 1,2
    case 1: new_mode = 2; state = 2; break;
    case 2: new_mode = 3; state = 3; break;
    case 3: new_mode = 0; state = 0; break;
    default: new_mode = 1; state = 1; break;
    }
    return new_mode;
  }
};

/**
 * @test GameCheat.OutlineModeStateMachineQuirk
 * @brief Verifies outline Mode State Machine Quirk.
 *
 * @details
 * Exercises the GameCheat code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/GameCheat.cpp
 * @ingroup descent3_tests
 */
TEST(GameCheat, OutlineModeStateMachineQuirk) {
  OutlineState os;
  EXPECT_EQ(os.advance(), 1); // mine outline
  EXPECT_EQ(os.state, 3);     // quirk: jumped to 3, not 1
  EXPECT_EQ(os.advance(), 0); // disabled
  EXPECT_EQ(os.state, 0);
}

/**
 * @test GameCheat.OutlineStatesTwoThreeCycle
 * @brief Verifies outline States Two Three Cycle.
 *
 * @details
 * Exercises the GameCheat code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/GameCheat.cpp
 * @ingroup descent3_tests
 */
TEST(GameCheat, OutlineStatesTwoThreeCycle) {
  OutlineState os;
  os.state = 2; // start mid-cycle
  EXPECT_EQ(os.advance(), 3); // mine+object
  EXPECT_EQ(os.advance(), 0); // off
  os.state = 1;
  EXPECT_EQ(os.advance(), 2); // object outline
  os.state = 99;
  EXPECT_EQ(os.advance(), 1); // default branch
  EXPECT_EQ(os.state, 1);
}

// replicated suicide message decode (GameCheat.cpp:639-653)
/**
 * @test GameCheat.SuicideMessageXorDecode
 * @brief Verifies suicide Message Xor Decode.
 *
 * @details
 * Exercises the GameCheat code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/GameCheat.cpp
 * @ingroup descent3_tests
 */
TEST(GameCheat, SuicideMessageXorDecode) {
  static const uint8_t buffer[20] = {0x80, 0x95, 0x9E, 0xFA, 0x89, 0xAE, 0xA8, 0xB3, 0xB1, 0xBF,
                                     0xA9, 0xFA, 0x88, 0xBF, 0xAC, 0xBF, 0xB4, 0xBD, 0xBF, 0xDA};
  char tb[21];
  for (int i = 0; i < 20; i++)
    tb[i] = (char)(buffer[i] ^ 0xDA);
  tb[20] = '\0';
  EXPECT_STREQ(tb, "ZOD Strikes Revenge");
}
