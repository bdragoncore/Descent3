/*
 * D3 Mock CFile - In-Memory File System for Testing
 *
 * Provides a mock implementation of cfile functions for unit testing.
 * Uses gMock for expectations/verification.
 * Linked as OBJECT library to take precedence over real cfile.
 */

#pragma once

#include <gmock/gmock.h>
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cstring>
#include <cstdio>
#include <cstdarg>

// D3 types
typedef unsigned char ubyte;

// Forward declare (matches real cfile.h)
struct CFILE;

// Minimal CFILE structure for mock (matches cfile.h)
struct MockCFILE {
  char *name = nullptr;
  void *file = nullptr; // Points to our InMemoryFile
  int lib_handle = -1;
  int size = 0;
  int lib_offset = 0;
  int position = 0;
  int flags = 0;
};

// In-memory file storage
class InMemoryFile {
public:
  std::vector<uint8_t> data;
  size_t position = 0;
  bool write_mode = false;
  bool is_text = false;
};

// In-memory file system - fully self-contained, no real cfile calls
class InMemoryFileSystem {
public:
  InMemoryFileSystem();
  ~InMemoryFileSystem();

  MockCFILE *OpenFile(const std::string &name, const char *mode);
  void CloseFile(MockCFILE *cf);

  // Test data injection
  void AddFile(const std::string &name, const std::vector<uint8_t> &data);
  void AddFile(const std::string &name, const std::string &data);
  void Clear();

  // Get internal file for direct manipulation
  InMemoryFile *GetFile(const std::string &name);

  std::map<std::string, std::unique_ptr<InMemoryFile>> files_;
  std::map<CFILE *, std::unique_ptr<MockCFILE>> open_files_;
  int next_handle_ = 1;
};

// Global access for stub
InMemoryFileSystem *&GetInMemoryFileSystem();

// Helper to get MockCFILE* from CFILE*
inline MockCFILE *ToMock(CFILE *cf) { return reinterpret_cast<MockCFILE *>(cf); }

// gMock class - for expectations/verification
class MockCFile {
public:
  MOCK_METHOD(CFILE *, cfopen, (const char *, const char *));
  MOCK_METHOD(void, cfclose, (CFILE *));
  MOCK_METHOD(int, cfilelength, (CFILE *));
  MOCK_METHOD(int, cfseek, (CFILE *, long, int));
  MOCK_METHOD(int, cftell, (CFILE *));
  MOCK_METHOD(int, cfeof, (CFILE *));
  MOCK_METHOD(int32_t, cf_ReadInt, (CFILE *));
  MOCK_METHOD(int16_t, cf_ReadShort, (CFILE *));
  MOCK_METHOD(int8_t, cf_ReadByte, (CFILE *));
  MOCK_METHOD(float, cf_ReadFloat, (CFILE *));
  MOCK_METHOD(double, cf_ReadDouble, (CFILE *));
  MOCK_METHOD(int, cf_ReadBytes, (uint8_t *, int, CFILE *));
  MOCK_METHOD(int, cf_ReadString, (char *, size_t, CFILE *));
  MOCK_METHOD(void, cf_WriteInt, (CFILE *, int32_t));
  MOCK_METHOD(void, cf_WriteShort, (CFILE *, int16_t));
  MOCK_METHOD(void, cf_WriteByte, (CFILE *, int8_t));
  MOCK_METHOD(void, cf_WriteFloat, (CFILE *, float));
  MOCK_METHOD(void, cf_WriteDouble, (CFILE *, double));
  MOCK_METHOD(int, cf_WriteBytes, (const uint8_t *, int, CFILE *));
  MOCK_METHOD(int, cf_WriteString, (CFILE *, const char *));
  // Note: cfprintf uses a custom implementation, not gMock
  MOCK_METHOD(int, cfexist, (const char *));
  MOCK_METHOD(void, cf_Rewind, (CFILE *));
  MOCK_METHOD(int, cfgetc, (CFILE *));
};

MockCFile *&GetMockCFile();

// ============================================================================
// C File API Declarations (standalone mock only)
// When D3_MOCK_CFILE_STANDALONE is defined, this header provides the API so
// tests can link only the mock. When undefined, include cfile.h first so
// the real API is used and this header only provides mock infrastructure.
// ============================================================================

#if defined(D3_MOCK_CFILE_STANDALONE)

#include <filesystem>

// File operations
CFILE *cfopen(const char *filename, const char *mode);
CFILE *cfopen(const std::filesystem::path &filename, const char *mode);
void cfclose(CFILE *cfp);
uint32_t cfilelength(CFILE *cfp);
int cfexist(const char *filename);
int cfexist(const std::filesystem::path &filename);

// Position
int cfseek(CFILE *cfp, long int offset, int where);
long cftell(CFILE *cfp);
int cfeof(CFILE *cfp);
void cf_Rewind(CFILE *cfp);

// Byte I/O
int cfgetc(CFILE *cfp);
int cf_ReadBytes(uint8_t *buf, int count, CFILE *cfp);
int cf_WriteBytes(const uint8_t *buf, int count, CFILE *cfp);

// Number read
int32_t cf_ReadInt(CFILE *cfp, bool little_endian = true);
int16_t cf_ReadShort(CFILE *cfp, bool little_endian = true);
int8_t cf_ReadByte(CFILE *cfp);
float cf_ReadFloat(CFILE *cfp);
double cf_ReadDouble(CFILE *cfp);

// Number write
void cf_WriteInt(CFILE *cfp, int32_t i);
void cf_WriteShort(CFILE *cfp, int16_t s);
void cf_WriteByte(CFILE *cfp, int8_t b);
void cf_WriteFloat(CFILE *cfp, float f);
void cf_WriteDouble(CFILE *cfp, double d);

// String I/O
int cf_ReadString(char *buf, size_t n, CFILE *cfp);
int cf_WriteString(CFILE *cfp, const char *buf);
int cfprintf(CFILE *cfp, const char *format, ...);

// Library management
int cf_OpenLibrary(const char *libname);
void cf_CloseLibrary(int handle);
int cf_SetSearchPath(const char *path, ...);
void cf_ClearAllSearchPaths();
CFILE *cf_OpenFileInLibrary(const char *filename, int libhandle);

// Utility
bool cf_CopyFile(const std::filesystem::path &dest, const std::filesystem::path &src, int copytime = 0);
bool cf_Diff(const std::filesystem::path &a, const std::filesystem::path &b);
void cf_CopyFileTime(char *dest, const char *src);
void cf_ChangeFileAttributes(const char *name, int attr);
unsigned int cf_GetfileCRC(char *src);
unsigned int cf_CalculateFileCRC(CFILE *fp);

#endif // D3_MOCK_CFILE_STANDALONE
