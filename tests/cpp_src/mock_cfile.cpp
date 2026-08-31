/**
 * @file mock_cfile.cpp
 * @brief D3 Mock CFile Implementation.
 *
 * @details
 * Provides in-memory file system for testing without touching real cfile.
 *
 * This harness validates the behavior of `Descent3/mock_cfile.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/mock_cfile.cpp`
 * @par Harness
 * `mock_cfile.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/mock_cfile.cpp
 */

#include "mock_cfile.hpp"
#if !defined(D3_MOCK_CFILE_STANDALONE)
#include "cfile.h"
#endif
#include "byteswap.h"
#include "pstypes.h"

#include <filesystem>
#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <algorithm>

// Global instances
static InMemoryFileSystem *g_inmem_fs = nullptr;
static MockCFile *g_mock_cfile = nullptr;

InMemoryFileSystem *&GetInMemoryFileSystem() { return g_inmem_fs; }

MockCFile *&GetMockCFile() { return g_mock_cfile; }

// ============================================================================
// InMemoryFileSystem Implementation
// ============================================================================

InMemoryFileSystem::InMemoryFileSystem() = default;

InMemoryFileSystem::~InMemoryFileSystem() { Clear(); }

void InMemoryFileSystem::Clear() {
  // Close all open files
  open_files_.clear();
  files_.clear();
}

void InMemoryFileSystem::AddFile(const std::string &name, const std::vector<uint8_t> &data) {
  auto file = std::make_unique<InMemoryFile>();
  file->data = data;
  files_[name] = std::move(file);
}

void InMemoryFileSystem::AddFile(const std::string &name, const std::string &data) {
  auto file = std::make_unique<InMemoryFile>();
  file->data.assign(data.begin(), data.end());
  files_[name] = std::move(file);
}

InMemoryFile *InMemoryFileSystem::GetFile(const std::string &name) {
  auto it = files_.find(name);
  if (it != files_.end()) {
    return it->second.get();
  }
  return nullptr;
}

MockCFILE *InMemoryFileSystem::OpenFile(const std::string &name, const char *mode) {
  // Find the file (for read mode)
  auto file_it = files_.find(name);
  InMemoryFile *infile = nullptr;

  if (mode[0] == 'r') {
    // Read mode - file must exist
    if (file_it == files_.end()) {
      return nullptr; // File not found
    }
    infile = file_it->second.get();
  } else if (mode[0] == 'w') {
    // Write mode - create new file if doesn't exist
    if (file_it == files_.end()) {
      // Create new file
      auto new_file = std::make_unique<InMemoryFile>();
      new_file->data = {};
      files_[name] = std::move(new_file);
      file_it = files_.find(name);
    }
    infile = file_it->second.get();
    infile->data.clear();
    infile->write_mode = true;
  } else {
    return nullptr; // Unknown mode
  }

  // Create mock CFILE
  auto mcf = std::make_unique<MockCFILE>();
  mcf->name = new char[name.length() + 1];
  strcpy(mcf->name, name.c_str());
  mcf->file = infile;
  mcf->size = static_cast<int>(infile->data.size());
  mcf->lib_handle = -1;
  mcf->lib_offset = 0;
  mcf->position = 0;
  mcf->flags = 0;

  // Handle mode
  if (mode[0] == 'w') {
    infile->write_mode = true;
    mcf->flags |= 0x2; // CFF_WRITING
  }
  if (mode[1] == 't') {
    infile->is_text = true;
    mcf->flags |= 0x1; // CFF_TEXT
  }

  // Generate a unique CFILE* (use address of the unique_ptr)
  CFILE *cfile_ptr = reinterpret_cast<CFILE *>(mcf.get());
  open_files_[cfile_ptr] = std::move(mcf);

  return open_files_[cfile_ptr].get();
}

void InMemoryFileSystem::CloseFile(MockCFILE *cf) {
  if (!cf)
    return;

  // Find and remove from open_files_
  for (auto it = open_files_.begin(); it != open_files_.end(); ++it) {
    if (it->second.get() == cf) {
      // Delete the name
      delete[] cf->name;
      open_files_.erase(it);
      return;
    }
  }
}

// ============================================================================
// Mock CFile Implementation - All functions fully self-contained
// ============================================================================

// Helper: get InMemoryFile from CFILE*
static InMemoryFile *GetInMemoryFile(CFILE *cf) {
  auto *mcf = ToMock(cf);
  if (!mcf || !mcf->file)
    return nullptr;
  return reinterpret_cast<InMemoryFile *>(mcf->file);
}

CFILE *cfopen(const char *filename, const char *mode) {
  if (g_inmem_fs) {
    return reinterpret_cast<CFILE *>(g_inmem_fs->OpenFile(filename, mode));
  }
  return nullptr;
}

CFILE *cfopen(const std::filesystem::path &filename, const char *mode) {
  if (g_inmem_fs) {
    return reinterpret_cast<CFILE *>(g_inmem_fs->OpenFile(filename.string(), mode));
  }
  return nullptr;
}

void cfclose(CFILE *cf) {
  if (!cf)
    return;
  auto *mcf = ToMock(cf);
  if (g_inmem_fs) {
    g_inmem_fs->CloseFile(mcf);
  }
}

uint32_t cfilelength(CFILE *cfp) {
  if (!cfp)
    return 0;
  auto *mcf = ToMock(cfp);
  return mcf ? static_cast<uint32_t>(mcf->size) : 0;
}

int cfexist(const char *filename) {
  if (g_inmem_fs) {
    auto *f = g_inmem_fs->GetFile(filename);
    if (f)
      return 2; // CFES_ON_DISK
  }
  return 0; // CFES_NOT_FOUND
}

int cfexist(const std::filesystem::path &filename) { return cfexist(filename.string().c_str()); }

int cfseek(CFILE *cfp, long int offset, int where) {
  auto *mcf = ToMock(cfp);
  if (!mcf)
    return -1;

  auto *infile = GetInMemoryFile(cfp);
  if (!infile)
    return -1;

  size_t new_pos;
  switch (where) {
  case SEEK_SET:
    new_pos = offset;
    break;
  case SEEK_CUR:
    new_pos = mcf->position + offset;
    break;
  case SEEK_END:
    new_pos = mcf->size + offset;
    break;
  default:
    return -1;
  }

  if (new_pos > infile->data.size())
    return -1;
  mcf->position = new_pos;
  infile->position = new_pos;
  return 0;
}

long cftell(CFILE *cfp) {
  auto *mcf = ToMock(cfp);
  return mcf ? static_cast<long>(mcf->position) : 0;
}

int cfeof(CFILE *cfp) {
  auto *mcf = ToMock(cfp);
  if (!mcf)
    return 1;
  return (mcf->position >= mcf->size) ? 1 : 0;
}

void cf_Rewind(CFILE *cfp) {
  auto *mcf = ToMock(cfp);
  if (!mcf)
    return;
  mcf->position = 0;
  auto *infile = GetInMemoryFile(cfp);
  if (infile)
    infile->position = 0;
}

int cfgetc(CFILE *cfp) {
  auto *mcf = ToMock(cfp);
  if (!mcf)
    return EOF;
  if (mcf->position >= mcf->size)
    return EOF;

  auto *infile = GetInMemoryFile(cfp);
  if (!infile || infile->data.empty())
    return EOF;

  int c = infile->data[mcf->position];
  mcf->position++;
  infile->position++;

  // Handle text mode newline conversion
  if (infile->is_text) {
    if (c == 10) { // LF
      // Check for CR/LF pair
      if (mcf->position < mcf->size && infile->data[mcf->position] == 13) {
        mcf->position++; // Skip CR
        infile->position++;
      }
      c = '\n';           // Return as newline
    } else if (c == 13) { // CR
      // Skip CR and check for LF
      if (mcf->position < mcf->size && infile->data[mcf->position] == 10) {
        mcf->position++; // Skip LF
        infile->position++;
      }
      c = '\n'; // Return as newline
    }
  }

  return c;
}

int cf_ReadBytes(uint8_t *buf, int count, CFILE *cfp) {
  auto *mcf = ToMock(cfp);
  if (!mcf || !buf || count <= 0)
    return 0;

  auto *infile = GetInMemoryFile(cfp);
  if (!infile)
    return 0;

  int avail = mcf->size - mcf->position;
  int to_read = std::min(count, avail);

  if (to_read > 0 && !infile->data.empty()) {
    std::memcpy(buf, infile->data.data() + mcf->position, to_read);
  }

  mcf->position += to_read;
  infile->position = mcf->position;

  return to_read;
}

int32_t cf_ReadInt(CFILE *cfp, bool little_endian) {
  int32_t val = 0;
  cf_ReadBytes(reinterpret_cast<uint8_t *>(&val), sizeof(val), cfp);
  if (little_endian) {
    return INTEL_INT(val);
  } else {
    // Big-endian: swap bytes
    return ((val & 0xFF) << 24) | ((val & 0xFF00) << 8) | ((val & 0xFF0000) >> 8) | ((val >> 24) & 0xFF);
  }
}

int16_t cf_ReadShort(CFILE *cfp, bool little_endian) {
  int16_t val = 0;
  cf_ReadBytes(reinterpret_cast<uint8_t *>(&val), sizeof(val), cfp);
  if (little_endian) {
    return INTEL_SHORT(val);
  } else {
    // Big-endian: swap bytes
    return ((val & 0xFF) << 8) | ((val >> 8) & 0xFF);
  }
}

int8_t cf_ReadByte(CFILE *cfp) {
  int c = cfgetc(cfp);
  return static_cast<int8_t>(c);
}

float cf_ReadFloat(CFILE *cfp) {
  float val = 0.0f;
  cf_ReadBytes(reinterpret_cast<uint8_t *>(&val), sizeof(val), cfp);
  return INTEL_FLOAT(val);
}

double cf_ReadDouble(CFILE *cfp) {
  double val = 0.0;
  cf_ReadBytes(reinterpret_cast<uint8_t *>(&val), sizeof(val), cfp);
  // Handle big-endian if needed
#ifdef OUTRAGE_BIG_ENDIAN
  double t;
  int *sp = reinterpret_cast<int *>(&val);
  int *dp = reinterpret_cast<int *>(&t);
  dp[0] = SWAPINT(sp[1]);
  dp[1] = SWAPINT(sp[0]);
  val = t;
#endif
  return val;
}

int cf_ReadString(char *buf, size_t n, CFILE *cfp) {
  if (!buf || n == 0)
    return -1;

  auto *mcf = ToMock(cfp);
  if (!mcf)
    return -1;

  auto *infile = GetInMemoryFile(cfp);
  if (!infile)
    return -1;

  size_t count = 0;
  char *bp = buf;

  for (;;) {
    if (count >= n - 1)
      break;

    int c = cfgetc(cfp);
    if (c == EOF) {
      if (!cfeof(cfp)) {
        // Error - not actually at EOF
        break;
      }
      break;
    }

    // End of string conditions
    bool is_text = (mcf->flags & 0x1) != 0; // CFF_TEXT
    if ((!is_text && c == 0) || (is_text && c == '\n')) {
      break;
    }

    *bp++ = static_cast<char>(c);
    count++;
  }

  *bp = '\0';
  return static_cast<int>(count);
}

int cf_WriteBytes(const uint8_t *buf, int count, CFILE *cfp) {
  auto *mcf = ToMock(cfp);
  if (!mcf || !buf || count <= 0)
    return 0;

  // Check if opened for writing
  if (!(mcf->flags & 0x2))
    return 0; // Not CFF_WRITING

  auto *infile = GetInMemoryFile(cfp);
  if (!infile)
    return 0;

  infile->data.resize(mcf->position + count);
  std::memcpy(infile->data.data() + mcf->position, buf, count);
  mcf->position += count;
  infile->position = mcf->position;
  mcf->size = static_cast<int>(infile->data.size());

  return count;
}

void cf_WriteInt(CFILE *cfp, int32_t i) {
  int32_t t = INTEL_INT(i);
  cf_WriteBytes(reinterpret_cast<const uint8_t *>(&t), sizeof(t), cfp);
}

void cf_WriteShort(CFILE *cfp, int16_t s) {
  int16_t t = INTEL_SHORT(s);
  cf_WriteBytes(reinterpret_cast<const uint8_t *>(&t), sizeof(t), cfp);
}

void cf_WriteByte(CFILE *cfp, int8_t b) {
  auto *mcf = ToMock(cfp);
  if (!mcf)
    return;

  auto *infile = GetInMemoryFile(cfp);
  if (!infile)
    return;

  // Check if opened for writing
  if (!(mcf->flags & 0x2))
    return;

  if (infile->data.size() <= static_cast<size_t>(mcf->position)) {
    infile->data.resize(mcf->position + 1);
  }
  infile->data[mcf->position] = static_cast<uint8_t>(b);
  mcf->position++;
  infile->position = mcf->position;
  mcf->size = static_cast<int>(infile->data.size());

  // Handle text mode newline
  if (infile->is_text && b == '\n') {
    mcf->position++;
    infile->position++;
  }
}

void cf_WriteFloat(CFILE *cfp, float f) {
  float t = INTEL_FLOAT(f);
  cf_WriteBytes(reinterpret_cast<const uint8_t *>(&t), sizeof(t), cfp);
}

void cf_WriteDouble(CFILE *cfp, double d) {
#ifdef OUTRAGE_BIG_ENDIAN
  double t;
  int *sp = reinterpret_cast<int *>(&d);
  int *dp = reinterpret_cast<int *>(&t);
  dp[0] = SWAPINT(sp[1]);
  dp[1] = SWAPINT(sp[0]);
  d = t;
#endif
  cf_WriteBytes(reinterpret_cast<const uint8_t *>(&d), sizeof(d), cfp);
}

int cf_WriteString(CFILE *cfp, const char *buf) {
  if (!buf)
    return 0;
  int len = static_cast<int>(strlen(buf));
  if (len > 0) {
    cf_WriteBytes(reinterpret_cast<const uint8_t *>(buf), len, cfp);
  }
  // Terminate with newline (text) or null (binary)
  auto *mcf = ToMock(cfp);
  if (mcf && (mcf->flags & 0x1)) { // CFF_TEXT
    cf_WriteByte(cfp, '\n');
    return len + 1;
  } else {
    cf_WriteByte(cfp, 0);
    return len + 1;
  }
}

// cfprintf - custom implementation outside gMock
int cfprintf(CFILE *cfp, const char *format, ...) {
  char buffer[4096];
  va_list args;
  va_start(args, format);
  int len = vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  if (len > 0) {
    cf_WriteBytes(reinterpret_cast<const uint8_t *>(buffer), len, cfp);
  }
  return len;
}

// Stub implementations for library functions (no-op for tests)
int cf_OpenLibrary(const char *libname) {
  return 0; // 0 means error
}

void cf_CloseLibrary(int handle) {
  // No-op
}

int cf_SetSearchPath(const char *path, ...) {
  return 1; // Success
}

void cf_ClearAllSearchPaths() {
  // No-op
}

CFILE *cf_OpenFileInLibrary(const char *filename, int libhandle) { return nullptr; }

// Utility stubs (simplified)
bool cf_CopyFile(const std::filesystem::path &dest, const std::filesystem::path &src, int copytime) {
  (void)copytime;
  if (g_inmem_fs) {
    auto *src_file = g_inmem_fs->GetFile(src.string());
    if (src_file) {
      g_inmem_fs->AddFile(dest.string(), src_file->data);
      return true;
    }
  }
  return false;
}

bool cf_Diff(const std::filesystem::path &a, const std::filesystem::path &b) {
  if (g_inmem_fs) {
    auto *fa = g_inmem_fs->GetFile(a.string());
    auto *fb = g_inmem_fs->GetFile(b.string());
    if (fa && fb) {
      return fa->data != fb->data;
    }
  }
  return true;
}

void cf_CopyFileTime(char *dest, const char *src) {
  // No-op
}

void cf_ChangeFileAttributes(const char *name, int attr) {
  // No-op
}

unsigned int cf_GetfileCRC(char *src) { return 0; }

unsigned int cf_CalculateFileCRC(CFILE *fp) { return 0; }

// ============================================================================
// Stubs for other dependencies (ddebug, ddio, args, mem, etc)
// ============================================================================

#define MAX_ARGS 100
#define MAX_CHARS_PER_ARG 256
extern char GameArgs[MAX_ARGS][MAX_CHARS_PER_ARG];

char GameArgs[MAX_ARGS][MAX_CHARS_PER_ARG];

int FindArg(const char *which) { return 0; }

void ddio_SplitPath(const char *srcPath, char *path, char *filename, char *ext) {
  if (path)
    *path = '\0';
  if (filename)
    *filename = '\0';
  if (ext)
    *ext = '\0';

  if (!srcPath || !*srcPath)
    return;

  const char *lastSlash = strrchr(srcPath, '/');
  const char *lastDot = strrchr(srcPath, '.');

  if (filename) {
    if (lastSlash) {
      strncpy(filename, lastSlash + 1, MAX_CHARS_PER_ARG - 1);
      filename[MAX_CHARS_PER_ARG - 1] = '\0';
    } else {
      strncpy(filename, srcPath, MAX_CHARS_PER_ARG - 1);
      filename[MAX_CHARS_PER_ARG - 1] = '\0';
    }
    if (lastDot && lastDot > (lastSlash ? lastSlash : srcPath)) {
      strncpy(ext, lastDot + 1, MAX_CHARS_PER_ARG - 1);
      ext[MAX_CHARS_PER_ARG - 1] = '\0';
    }
  }

  if (path) {
    if (lastSlash) {
      size_t len = lastSlash - srcPath;
      strncpy(path, srcPath, std::min(len, (size_t)MAX_CHARS_PER_ARG - 1));
      path[std::min(len, (size_t)MAX_CHARS_PER_ARG - 1)] = '\0';
    } else {
      *path = '\0';
    }
  }
}

// Stub function - used as function pointer in pserror.h
extern "C" {
void mprintf(const char *format, ...) {}
void mprintf_at(void *wnd, int row, int col, const char *format, ...) {}
}
