/*
 * Enhanced D3 Mock CFile Implementation
 *
 * Provides a complete mock implementation of cfile functions.
 * Supports HOG libraries, base directories, search paths, and case-insensitive paths.
 */

#include "mock_cfile_full.hpp"
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
  open_files_.clear();
  files_.clear();
  libraries_.clear();
  base_directories_.clear();
  search_paths_.clear();
  next_handle_ = 1;
  next_lib_handle_ = 1;
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
  // Check search paths first if reading and file not found directly
  std::string actual_name = name;

  if (mode[0] == 'r') {
    // Try direct access first
    auto file_it = files_.find(name);

    // If not found, try search paths
    if (file_it == files_.end()) {
      for (const auto &sp : search_paths_) {
        std::string path_name = sp.path + "/" + name;
        file_it = files_.find(path_name);
        if (file_it != files_.end()) {
          actual_name = path_name;
          break;
        }
        // Try case-insensitive match
        for (const auto &[fname, _] : files_) {
          if (CaseInsensitiveCompare(fname, path_name)) {
            actual_name = fname;
            file_it = files_.find(fname);
            break;
          }
        }
        if (file_it != files_.end())
          break;
      }
    }

    // Try case-insensitive match for the original name
    if (file_it == files_.end()) {
      for (const auto &[fname, _] : files_) {
        if (CaseInsensitiveCompare(fname, name)) {
          actual_name = fname;
          file_it = files_.find(fname);
          break;
        }
      }
    }

    // If still not found, try libraries
    if (file_it == files_.end()) {
      for (auto &[handle, lib] : libraries_) {
        auto file_data = lib->GetFileData(name);
        if (!file_data) {
          // Try case-insensitive match in library
          for (const auto &[fname, data] : lib->files) {
            if (CaseInsensitiveCompare(fname, name)) {
              file_data = &data;
              break;
            }
          }
        }
        if (file_data) {
          // Found in library - create the file handle from library data
          return OpenFileFromLibrary(name, handle);
        }
      }
    }

    if (file_it == files_.end()) {
      return nullptr; // File not found
    }
  }

  auto file_it = files_.find(actual_name);
  InMemoryFile *infile = nullptr;

  if (mode[0] == 'r') {
    // Read mode - file must exist
    if (file_it == files_.end()) {
      return nullptr;
    }
    infile = file_it->second.get();
  } else if (mode[0] == 'w') {
    // Write mode - create new file if doesn't exist
    if (file_it == files_.end()) {
      auto new_file = std::make_unique<InMemoryFile>();
      new_file->data = {};
      files_[actual_name] = std::move(new_file);
      file_it = files_.find(actual_name);
    }
    infile = file_it->second.get();
    infile->data.clear();
    infile->write_mode = true;
  } else {
    return nullptr;
  }

  // Create mock CFILE
  auto mcf = std::make_unique<MockCFILE>();
  mcf->name = new char[actual_name.length() + 1];
  strcpy(mcf->name, actual_name.c_str());
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

  CFILE *cfile_ptr = reinterpret_cast<CFILE *>(mcf.get());
  open_files_[cfile_ptr] = std::move(mcf);

  return open_files_[cfile_ptr].get();
}

MockCFILE *InMemoryFileSystem::OpenFileFromLibrary(const std::string &name, int lib_handle) {
  auto lib_it = libraries_.find(lib_handle);
  if (lib_it == libraries_.end()) {
    return nullptr; // Library not found
  }

  auto *lib = lib_it->second.get();
  auto file_data = lib->GetFileData(name);

  // Try case-insensitive match
  if (!file_data) {
    for (const auto &[fname, data] : lib->files) {
      if (CaseInsensitiveCompare(fname, name)) {
        file_data = &data;
        break;
      }
    }
  }

  if (!file_data) {
    return nullptr; // File not in library
  }

  // Create an in-memory file for this library file
  std::string unique_name = lib->name + ":" + name;
  auto infile = std::make_unique<InMemoryFile>();
  infile->data = *file_data;
  files_[unique_name] = std::move(infile);

  // Create mock CFILE
  auto mcf = std::make_unique<MockCFILE>();
  mcf->name = new char[name.length() + 1];
  strcpy(mcf->name, name.c_str());
  mcf->file = files_[unique_name].get();
  mcf->size = static_cast<int>(file_data->size());
  mcf->lib_handle = lib_handle;
  mcf->lib_offset = 0;
  mcf->position = 0;
  mcf->flags = 0;

  CFILE *cfile_ptr = reinterpret_cast<CFILE *>(mcf.get());
  open_files_[cfile_ptr] = std::move(mcf);

  return open_files_[cfile_ptr].get();
}

void InMemoryFileSystem::CloseFile(MockCFILE *cf) {
  if (!cf)
    return;

  for (auto it = open_files_.begin(); it != open_files_.end(); ++it) {
    if (it->second.get() == cf) {
      delete[] cf->name;
      open_files_.erase(it);
      return;
    }
  }
}

// Library operations
int InMemoryFileSystem::OpenLibrary(const std::string &name) {
  // Check if library already exists
  for (auto &[handle, lib] : libraries_) {
    if (lib->name == name) {
      return handle; // Return existing handle
    }
  }

  int handle = next_lib_handle_++;
  auto lib = std::make_unique<InMemoryLibrary>();
  lib->name = name;
  lib->handle = handle;
  libraries_[handle] = std::move(lib);
  return handle;
}

void InMemoryFileSystem::CloseLibrary(int handle) { libraries_.erase(handle); }

InMemoryLibrary *InMemoryFileSystem::GetLibrary(int handle) {
  auto it = libraries_.find(handle);
  if (it != libraries_.end()) {
    return it->second.get();
  }
  return nullptr;
}

InMemoryLibrary *InMemoryFileSystem::GetLibrary(const std::string &name) {
  for (auto &[handle, lib] : libraries_) {
    if (lib->name == name) {
      return lib.get();
    }
  }
  return nullptr;
}

void InMemoryFileSystem::AddFileToLibrary(const std::string &lib_name, const std::string &filename,
                                          const std::vector<uint8_t> &data) {
  auto *lib = GetLibrary(lib_name);
  if (!lib) {
    // Create library if it doesn't exist
    int handle = OpenLibrary(lib_name);
    lib = GetLibrary(handle);
  }
  if (lib) {
    lib->files[filename] = data;
  }
}

// Base directory operations
void InMemoryFileSystem::AddBaseDirectory(const std::string &dir) { base_directories_.push_back(dir); }

void InMemoryFileSystem::ClearBaseDirectories() { base_directories_.clear(); }

// Search path operations
bool InMemoryFileSystem::AddSearchPath(const std::string &path, const std::vector<std::string> &ext_list) {
  SearchPath sp;
  sp.path = path;
  sp.extensions = ext_list;
  search_paths_.push_back(sp);
  return true;
}

void InMemoryFileSystem::ClearSearchPaths() { search_paths_.clear(); }

// Case-insensitive helpers
bool InMemoryFileSystem::CaseInsensitiveCompare(const std::string &a, const std::string &b) {
  return ToLower(a) == ToLower(b);
}

std::string InMemoryFileSystem::ToLower(const std::string &s) {
  std::string result = s;
  std::transform(result.begin(), result.end(), result.begin(), ::tolower);
  return result;
}

// Path resolution
std::string InMemoryFileSystem::LocatePath(const std::string &relative_path, bool case_insensitive) {
  // Try direct match first
  if (files_.find(relative_path) != files_.end()) {
    return relative_path;
  }

  // Try base directories
  for (const auto &base : base_directories_) {
    std::string full_path = base + "/" + relative_path;
    if (files_.find(full_path) != files_.end()) {
      return full_path;
    }
  }

  // Try search paths
  for (const auto &sp : search_paths_) {
    std::string path_name = sp.path + "/" + relative_path;
    if (files_.find(path_name) != files_.end()) {
      return path_name;
    }
  }

  // Try libraries
  for (auto &[handle, lib] : libraries_) {
    if (lib->HasFile(relative_path)) {
      return lib->name + "/" + relative_path;
    }
    // Case-insensitive search in library
    if (case_insensitive) {
      for (const auto &[fname, _] : lib->files) {
        if (CaseInsensitiveCompare(fname, relative_path)) {
          return lib->name + "/" + fname;
        }
      }
    }
  }

  // Case-insensitive search in regular files
  if (case_insensitive) {
    std::string lower_path = ToLower(relative_path);
    for (const auto &[fname, _] : files_) {
      // Check just the filename portion
      size_t last_slash = fname.find_last_of("/");
      std::string filename_only = (last_slash != std::string::npos) ? fname.substr(last_slash + 1) : fname;
      if (ToLower(filename_only) == lower_path) {
        return fname;
      }
    }
  }

  return ""; // Not found
}

std::vector<std::string> InMemoryFileSystem::LocateMultiplePaths(const std::string &relative_path) {
  std::vector<std::string> results;

  // Check all locations
  std::string direct = LocatePath(relative_path, false);
  if (!direct.empty()) {
    results.push_back(direct);
  }

  return results;
}

// Check file existence
int InMemoryFileSystem::FileExists(const std::string &name) {
  // Check direct files (on "disk")
  if (files_.find(name) != files_.end()) {
    return 2; // CFES_ON_DISK (2 = on disk in original enum, but we use it for in-memory)
  }

  // Check case-insensitive in direct files
  for (const auto &[fname, _] : files_) {
    if (CaseInsensitiveCompare(fname, name)) {
      return 2;
    }
  }

  // Check in libraries
  for (auto &[handle, lib] : libraries_) {
    if (lib->HasFile(name)) {
      return 1; // CFES_IN_LIBRARY
    }
    // Case-insensitive search
    for (const auto &[fname, _] : lib->files) {
      if (CaseInsensitiveCompare(fname, name)) {
        return 1;
      }
    }
  }

  return 0; // CFES_NOT_FOUND
}

// ============================================================================
// Mock CFile Implementation - All functions fully self-contained
// ============================================================================

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
  return cfopen(filename.string().c_str(), mode);
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
    return g_inmem_fs->FileExists(filename);
  }
  return 0;
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
      if (mcf->position < mcf->size && infile->data[mcf->position] == 13) {
        mcf->position++;
        infile->position++;
      }
      c = '\n';
    } else if (c == 13) { // CR
      if (mcf->position < mcf->size && infile->data[mcf->position] == 10) {
        mcf->position++;
        infile->position++;
      }
      c = '\n';
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
    return ((val & 0xFF) << 24) | ((val & 0xFF00) << 8) | ((val & 0xFF0000) >> 8) | ((val >> 24) & 0xFF);
  }
}

int16_t cf_ReadShort(CFILE *cfp, bool little_endian) {
  int16_t val = 0;
  cf_ReadBytes(reinterpret_cast<uint8_t *>(&val), sizeof(val), cfp);
  if (little_endian) {
    return INTEL_SHORT(val);
  } else {
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
      if (!cfeof(cfp))
        break;
      break;
    }

    bool is_text = (mcf->flags & 0x1) != 0;
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

  if (!(mcf->flags & 0x2))
    return 0;

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

  if (!(mcf->flags & 0x2))
    return;

  if (infile->data.size() <= static_cast<size_t>(mcf->position)) {
    infile->data.resize(mcf->position + 1);
  }
  infile->data[mcf->position] = static_cast<uint8_t>(b);
  mcf->position++;
  infile->position = mcf->position;
  mcf->size = static_cast<int>(infile->data.size());

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
  auto *mcf = ToMock(cfp);
  if (mcf && (mcf->flags & 0x1)) {
    cf_WriteByte(cfp, '\n');
    return len + 1;
  } else {
    cf_WriteByte(cfp, 0);
    return len + 1;
  }
}

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

// ============================================================================
// Library Management
// ============================================================================

int cf_OpenLibrary(const char *libname) {
  if (g_inmem_fs) {
    return g_inmem_fs->OpenLibrary(libname);
  }
  return 0;
}

void cf_CloseLibrary(int handle) {
  if (g_inmem_fs) {
    g_inmem_fs->CloseLibrary(handle);
  }
}

CFILE *cf_OpenFileInLibrary(const char *filename, int libhandle) {
  if (g_inmem_fs) {
    return reinterpret_cast<CFILE *>(g_inmem_fs->OpenFileFromLibrary(filename, libhandle));
  }
  return nullptr;
}

// ============================================================================
// Search Paths
// ============================================================================

int cf_SetSearchPath(const char *path, ...) {
  if (!g_inmem_fs || !path)
    return 0;

  std::vector<std::string> ext_list;
  va_list args;
  va_start(args, path);

  // First vararg is the extension list (if any)
  const char *ext = va_arg(args, const char *);
  while (ext != nullptr) {
    ext_list.push_back(ext);
    ext = va_arg(args, const char *);
  }

  va_end(args);

  return g_inmem_fs->AddSearchPath(path, ext_list) ? 1 : 0;
}

void cf_ClearAllSearchPaths() {
  if (g_inmem_fs) {
    g_inmem_fs->ClearSearchPaths();
  }
}

// ============================================================================
// Base Directories
// ============================================================================

void cf_AddBaseDirectory(const std::filesystem::path &base_directory) {
  if (g_inmem_fs) {
    g_inmem_fs->AddBaseDirectory(base_directory.string());
  }
}

void cf_ClearBaseDirectories() {
  if (g_inmem_fs) {
    g_inmem_fs->ClearBaseDirectories();
  }
}

// ============================================================================
// Path Resolution
// ============================================================================

std::filesystem::path cf_LocatePath(const std::filesystem::path &relative_path) {
  if (g_inmem_fs) {
    std::string result = g_inmem_fs->LocatePath(relative_path.string(), true);
    return result.empty() ? std::filesystem::path() : std::filesystem::path(result);
  }
  return std::filesystem::path();
}

std::vector<std::filesystem::path> cf_LocateMultiplePaths(const std::filesystem::path &relative_path) {
  std::vector<std::filesystem::path> results;
  if (g_inmem_fs) {
    auto paths = g_inmem_fs->LocateMultiplePaths(relative_path.string());
    for (const auto &p : paths) {
      results.push_back(p);
    }
  }
  return results;
}

// ============================================================================
// Utility Functions
// ============================================================================

bool cf_CopyFile(const std::filesystem::path &dest, const std::filesystem::path &src, int copytime) {
  (void)copytime;
  if (g_inmem_fs) {
    auto *src_file = g_inmem_fs->GetFile(src.string());
    if (src_file) {
      g_inmem_fs->AddFile(dest.string(), src_file->data);
      return true;
    }

    // Check in libraries
    for (auto &[handle, lib] : g_inmem_fs->libraries_) {
      auto file_data = lib->GetFileData(src.string());
      if (!file_data) {
        // Try case-insensitive
        for (const auto &[fname, data] : lib->files) {
          if (InMemoryFileSystem::CaseInsensitiveCompare(fname, src.string())) {
            file_data = &data;
            break;
          }
        }
      }
      if (file_data) {
        g_inmem_fs->AddFile(dest.string(), *file_data);
        return true;
      }
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
  // No-op for mock
  (void)dest;
  (void)src;
}

void cf_ChangeFileAttributes(const char *name, int attr) {
  // No-op for mock
  (void)name;
  (void)attr;
}

unsigned int cf_GetfileCRC(char *src) {
  // Simplified CRC calculation for mock
  if (g_inmem_fs) {
    auto *file = g_inmem_fs->GetFile(src);
    if (file && !file->data.empty()) {
      unsigned int crc = 0;
      for (auto byte : file->data) {
        crc = crc * 31 + byte;
      }
      return crc;
    }
  }
  return 0;
}

unsigned int cf_CalculateFileCRC(CFILE *fp) {
  if (!fp)
    return 0;

  auto *mcf = ToMock(fp);
  if (!mcf)
    return 0;

  auto *infile = GetInMemoryFile(fp);
  if (!infile || infile->data.empty())
    return 0;

  // Calculate CRC from current position to end
  unsigned int crc = 0;
  for (size_t i = mcf->position; i < infile->data.size(); i++) {
    crc = crc * 31 + infile->data[i];
  }

  return crc;
}

// ============================================================================
// Stubs for other dependencies
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

extern "C" {
void mprintf(const char *format, ...) {}
void mprintf_at(void *wnd, int row, int col, const char *format, ...) {}
}
