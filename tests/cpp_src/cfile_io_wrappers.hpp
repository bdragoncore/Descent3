/*
 * CFile IO Wrappers - Linker Wrapped C Stdio Functions
 *
 * Provides mock implementations of C stdio functions for testing real cfile
 * library with controlled filesystem behavior.
 *
 * Uses GCC/Clang --wrap linker feature to intercept C function calls.
 */

#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <memory>

// In-memory file representation for mock filesystem
class MockRealFile {
public:
  std::vector<uint8_t> data;
  size_t position = 0;
  bool is_open = false;
  bool write_mode = false;
  std::string mode;
  int error_code = 0; // Simulated errno
};

// Controller for test scenarios
class CFileIOController {
public:
  CFileIOController();
  ~CFileIOController();

  // File content management
  void AddFile(const std::string &path, const std::vector<uint8_t> &data);
  void AddFile(const std::string &path, const std::string &data);
  void ClearFiles();
  MockRealFile *GetFile(const std::string &path);
  bool FileExists(const std::string &path);

  // File descriptor management (FILE* -> MockRealFile mapping)
  void RegisterFILE(FILE *fp, MockRealFile *file);
  void UnregisterFILE(FILE *fp);
  MockRealFile *GetFileFromFILE(FILE *fp);
  void RegisterFD(int fd, FILE *fp);
  void UnregisterFD(int fd);
  FILE *GetFILEFromFD(int fd);

  // Behavior control
  void SetOpenShouldFail(bool fail, int error = 0);
  void SetReadShouldFail(bool fail, int error = 0);
  void SetWriteShouldFail(bool fail, int error = 0);
  void SetSeekShouldFail(bool fail, int error = 0);
  void ResetFailures();

  // Call tracking
  int GetOpenCallCount() const { return open_call_count_; }
  int GetCloseCallCount() const { return close_call_count_; }
  int GetReadCallCount() const { return read_call_count_; }
  int GetWriteCallCount() const { return write_call_count_; }
  std::string GetLastOpenedPath() const { return last_opened_path_; }
  void ResetCallCounts();

  // Error simulation
  void SetSimulatedError(int error) { simulated_errno_ = error; }
  int GetSimulatedError() const { return simulated_errno_; }
  void ClearSimulatedError() { simulated_errno_ = 0; }

  // Global instance accessor
  static CFileIOController *GetInstance();
  static void SetInstance(CFileIOController *controller);

  // Internal state - accessible by wrapper functions
  std::map<std::string, std::unique_ptr<MockRealFile>> files_;
  std::map<FILE *, MockRealFile *> file_descriptors_;
  std::map<int, FILE *> fd_to_file_;
  int next_fd_ = 100; // Start fake fd numbers at 100 to avoid conflicts

  int open_call_count_ = 0;
  int close_call_count_ = 0;
  int read_call_count_ = 0;
  int write_call_count_ = 0;
  std::string last_opened_path_;

  bool open_should_fail_ = false;
  bool read_should_fail_ = false;
  bool write_should_fail_ = false;
  bool seek_should_fail_ = false;
  int simulated_errno_ = 0;
  int next_fd_id_ = 1;

  static CFileIOController *instance_;
};

// Helper to get/set errno from C code
extern "C" {
extern int mock_errno;
}

// Wrapped function declarations (these are called instead of real C functions)
extern "C" {
FILE *__real_fopen(const char *filename, const char *mode);
int __real_fclose(FILE *stream);
size_t __real_fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t __real_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
int __real_fseek(FILE *stream, long offset, int whence);
long __real_ftell(FILE *stream);
int __real_feof(FILE *stream);
void __real_rewind(FILE *stream);
int __real_fputc(int c, FILE *stream);
int __real_vfprintf(FILE *stream, const char *format, va_list ap);
int __real_fileno(FILE *stream);
int __real_fstat(int fd, struct stat *buf);
}
