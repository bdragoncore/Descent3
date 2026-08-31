/*
 * Enhanced D3 Mock CFile - Full Implementation
 *
 * Provides a complete mock implementation of cfile functions for unit testing.
 * Supports:
 * - In-memory file system
 * - HOG library simulation
 * - Base directories
 * - Search paths
 * - Case-insensitive path resolution
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
#include <filesystem>
#include <algorithm>

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

// Simulated HOG library
class InMemoryLibrary {
public:
  std::string name;
  int handle = -1;
  std::map<std::string, std::vector<uint8_t>> files; // filename -> data

  bool HasFile(const std::string &filename) const { return files.find(filename) != files.end(); }

  const std::vector<uint8_t> *GetFileData(const std::string &filename) const {
    auto it = files.find(filename);
    if (it != files.end()) {
      return &it->second;
    }
    return nullptr;
  }
};

// Search path entry
struct SearchPath {
  std::string path;
  std::vector<std::string> extensions; // empty = all files
};

// Enhanced in-memory file system
class InMemoryFileSystem {
public:
  InMemoryFileSystem();
  ~InMemoryFileSystem();

  // File operations
  MockCFILE *OpenFile(const std::string &name, const char *mode);
  MockCFILE *OpenFileFromLibrary(const std::string &name, int lib_handle);
  void CloseFile(MockCFILE *cf);

  // Test data injection
  void AddFile(const std::string &name, const std::vector<uint8_t> &data);
  void AddFile(const std::string &name, const std::string &data);
  void Clear();

  // Get internal file for direct manipulation
  InMemoryFile *GetFile(const std::string &name);

  // Library operations
  int OpenLibrary(const std::string &name);
  void CloseLibrary(int handle);
  InMemoryLibrary *GetLibrary(int handle);
  InMemoryLibrary *GetLibrary(const std::string &name);
  void AddFileToLibrary(const std::string &lib_name, const std::string &filename, const std::vector<uint8_t> &data);

  // Base directory operations
  void AddBaseDirectory(const std::string &dir);
  void ClearBaseDirectories();
  std::vector<std::string> GetBaseDirectories() const { return base_directories_; }

  // Search path operations
  bool AddSearchPath(const std::string &path, const std::vector<std::string> &ext_list = {});
  void ClearSearchPaths();
  std::vector<SearchPath> GetSearchPaths() const { return search_paths_; }

  // Path resolution
  std::string LocatePath(const std::string &relative_path, bool case_insensitive = true);
  std::vector<std::string> LocateMultiplePaths(const std::string &relative_path);

  // Check if file exists and where
  int FileExists(const std::string &name); // Returns: 0=not found, 1=on disk, 2=in library

  std::map<std::string, std::unique_ptr<InMemoryFile>> files_;
  std::map<CFILE *, std::unique_ptr<MockCFILE>> open_files_;
  std::map<int, std::unique_ptr<InMemoryLibrary>> libraries_;
  std::vector<std::string> base_directories_;
  std::vector<SearchPath> search_paths_;
  int next_handle_ = 1;
  int next_lib_handle_ = 1;

  // Case-insensitive helpers (public for external use)
  static bool CaseInsensitiveCompare(const std::string &a, const std::string &b);
  static std::string ToLower(const std::string &s);

private:
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
  MOCK_METHOD(int, cfexist, (const char *));
  MOCK_METHOD(void, cf_Rewind, (CFILE *));
  MOCK_METHOD(int, cfgetc, (CFILE *));
  MOCK_METHOD(int, cf_OpenLibrary, (const char *));
  MOCK_METHOD(void, cf_CloseLibrary, (int));
  MOCK_METHOD(CFILE *, cf_OpenFileInLibrary, (const char *, int));
};

MockCFile *&GetMockCFile();

// ============================================================================
// C File API Declarations (standalone mock only)
// ============================================================================

#if defined(D3_MOCK_CFILE_STANDALONE)

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

// Base directory management
void cf_AddBaseDirectory(const std::filesystem::path &base_directory);
void cf_ClearBaseDirectories();

// Path resolution
std::filesystem::path cf_LocatePath(const std::filesystem::path &relative_path);
std::vector<std::filesystem::path> cf_LocateMultiplePaths(const std::filesystem::path &relative_path);

// Utility
bool cf_CopyFile(const std::filesystem::path &dest, const std::filesystem::path &src, int copytime = 0);
bool cf_Diff(const std::filesystem::path &a, const std::filesystem::path &b);
void cf_CopyFileTime(char *dest, const char *src);
void cf_ChangeFileAttributes(const char *name, int attr);
unsigned int cf_GetfileCRC(char *src);
unsigned int cf_CalculateFileCRC(CFILE *fp);

#endif // D3_MOCK_CFILE_STANDALONE
