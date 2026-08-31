/*
 * CFile IO Wrappers - Implementation
 *
 * Mock implementations of C stdio functions using linker wrapping.
 * Real functions are accessed via __real_* prefixes.
 */

#include "cfile_io_wrappers.hpp"
#include <cstring>
#include <cstdarg>
#include <cerrno>
#include <stdexcept>
#include <sys/stat.h>

// Global errno override for tests
extern "C" {
int mock_errno = 0;
}

// Static controller instance
CFileIOController *CFileIOController::instance_ = nullptr;

CFileIOController::CFileIOController() {}

CFileIOController::~CFileIOController() { ClearFiles(); }

CFileIOController *CFileIOController::GetInstance() { return instance_; }

void CFileIOController::SetInstance(CFileIOController *controller) { instance_ = controller; }

// File content management
void CFileIOController::AddFile(const std::string &path, const std::vector<uint8_t> &data) {
  auto file = std::make_unique<MockRealFile>();
  file->data = data;
  files_[path] = std::move(file);
}

void CFileIOController::AddFile(const std::string &path, const std::string &data) {
  AddFile(path, std::vector<uint8_t>(data.begin(), data.end()));
}

void CFileIOController::ClearFiles() {
  files_.clear();
  file_descriptors_.clear();
}

MockRealFile *CFileIOController::GetFile(const std::string &path) {
  auto it = files_.find(path);
  if (it != files_.end()) {
    return it->second.get();
  }
  return nullptr;
}

bool CFileIOController::FileExists(const std::string &path) { return GetFile(path) != nullptr; }

// FILE* descriptor management
void CFileIOController::RegisterFILE(FILE *fp, MockRealFile *file) {
  file_descriptors_[fp] = file;
  file->is_open = true;
}

void CFileIOController::UnregisterFILE(FILE *fp) {
  auto it = file_descriptors_.find(fp);
  if (it != file_descriptors_.end()) {
    it->second->is_open = false;
    file_descriptors_.erase(it);
  }
}

MockRealFile *CFileIOController::GetFileFromFILE(FILE *fp) {
  auto it = file_descriptors_.find(fp);
  if (it != file_descriptors_.end()) {
    return it->second;
  }
  return nullptr;
}

// Behavior control
void CFileIOController::SetOpenShouldFail(bool fail, int error) {
  open_should_fail_ = fail;
  if (fail)
    simulated_errno_ = error;
}

void CFileIOController::SetReadShouldFail(bool fail, int error) {
  read_should_fail_ = fail;
  if (fail)
    simulated_errno_ = error;
}

void CFileIOController::SetWriteShouldFail(bool fail, int error) {
  write_should_fail_ = fail;
  if (fail)
    simulated_errno_ = error;
}

void CFileIOController::SetSeekShouldFail(bool fail, int error) {
  seek_should_fail_ = fail;
  if (fail)
    simulated_errno_ = error;
}

void CFileIOController::ResetFailures() {
  open_should_fail_ = false;
  read_should_fail_ = false;
  write_should_fail_ = false;
  seek_should_fail_ = false;
  simulated_errno_ = 0;
  mock_errno = 0;
}

// Call tracking
void CFileIOController::ResetCallCounts() {
  open_call_count_ = 0;
  close_call_count_ = 0;
  read_call_count_ = 0;
  write_call_count_ = 0;
  last_opened_path_.clear();
}

// FD management
void CFileIOController::RegisterFD(int fd, FILE *fp) { fd_to_file_[fd] = fp; }

void CFileIOController::UnregisterFD(int fd) { fd_to_file_.erase(fd); }

FILE *CFileIOController::GetFILEFromFD(int fd) {
  auto it = fd_to_file_.find(fd);
  if (it != fd_to_file_.end()) {
    return it->second;
  }
  return nullptr;
}

// ============================================================================
// Wrapped C Function Implementations
// ============================================================================

extern "C" {

// fopen - Open a file
FILE *__wrap_fopen(const char *filename, const char *mode) {
  CFileIOController *ctrl = CFileIOController::GetInstance();
  if (!ctrl) {
    // No controller - fall back to real implementation
    return __real_fopen(filename, mode);
  }

  ctrl->open_call_count_++;
  ctrl->last_opened_path_ = filename ? filename : "";

  // Check if should simulate failure
  if (ctrl->open_should_fail_) {
    errno = ctrl->simulated_errno_ ? ctrl->simulated_errno_ : ENOENT;
    mock_errno = errno;
    return nullptr;
  }

  // Check write mode
  bool write_mode = (mode[0] == 'w' || mode[0] == 'a');

  MockRealFile *file = ctrl->GetFile(filename);

  if (!file) {
    if (write_mode) {
      // Create new file for writing
      ctrl->AddFile(filename, std::vector<uint8_t>());
      file = ctrl->GetFile(filename);
    } else {
      // File not found for reading
      errno = ENOENT;
      mock_errno = ENOENT;
      return nullptr;
    }
  }

  if (!file) {
    errno = ENOMEM;
    mock_errno = ENOMEM;
    return nullptr;
  }

  // Set up file state
  file->write_mode = write_mode;
  file->mode = mode;
  file->position = 0; // Always reset position on open
  if (write_mode) {
    if (mode[0] == 'w') {
      file->data.clear(); // Truncate for write mode
    }
  }

  // Generate a unique FILE* identifier (can't use real FILE, so use pointer to our structure)
  // We use the address of the MockRealFile as the FILE* handle
  FILE *fp = reinterpret_cast<FILE *>(file);
  ctrl->RegisterFILE(fp, file);

  return fp;
}

// fclose - Close a file
int __wrap_fclose(FILE *stream) {
  CFileIOController *ctrl = CFileIOController::GetInstance();
  if (!ctrl) {
    return __real_fclose(stream);
  }

  ctrl->close_call_count_++;

  MockRealFile *file = ctrl->GetFileFromFILE(stream);
  if (file) {
    // Find and remove any fds associated with this file
    for (auto it = ctrl->fd_to_file_.begin(); it != ctrl->fd_to_file_.end();) {
      if (it->second == stream) {
        it = ctrl->fd_to_file_.erase(it);
      } else {
        ++it;
      }
    }
    ctrl->UnregisterFILE(stream);
    return 0;
  }

  // Unknown FILE* - return error
  errno = EBADF;
  mock_errno = EBADF;
  return EOF;
}

// fread - Read from file
size_t __wrap_fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  CFileIOController *ctrl = CFileIOController::GetInstance();
  if (!ctrl) {
    return __real_fread(ptr, size, nmemb, stream);
  }

  ctrl->read_call_count_++;

  // Check simulated failure
  if (ctrl->read_should_fail_) {
    errno = ctrl->simulated_errno_ ? ctrl->simulated_errno_ : EIO;
    mock_errno = errno;
    return 0;
  }

  MockRealFile *file = ctrl->GetFileFromFILE(stream);
  if (!file || !file->is_open) {
    errno = EBADF;
    mock_errno = EBADF;
    return 0;
  }

  size_t total_bytes = size * nmemb;
  size_t available = file->data.size() - file->position;
  size_t to_read = (total_bytes < available) ? total_bytes : available;

  if (to_read > 0 && ptr) {
    std::memcpy(ptr, file->data.data() + file->position, to_read);
    file->position += to_read;
  }

  return to_read / size; // Return number of items read
}

// fwrite - Write to file
size_t __wrap_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
  CFileIOController *ctrl = CFileIOController::GetInstance();
  if (!ctrl) {
    return __real_fwrite(ptr, size, nmemb, stream);
  }

  ctrl->write_call_count_++;

  // Check simulated failure
  if (ctrl->write_should_fail_) {
    errno = ctrl->simulated_errno_ ? ctrl->simulated_errno_ : EIO;
    mock_errno = errno;
    return 0;
  }

  MockRealFile *file = ctrl->GetFileFromFILE(stream);
  if (!file || !file->is_open) {
    errno = EBADF;
    mock_errno = EBADF;
    return 0;
  }

  if (!file->write_mode) {
    errno = EBADF;
    mock_errno = EBADF;
    return 0;
  }

  size_t total_bytes = size * nmemb;

  // Resize if needed
  if (file->position + total_bytes > file->data.size()) {
    file->data.resize(file->position + total_bytes);
  }

  if (total_bytes > 0 && ptr) {
    std::memcpy(file->data.data() + file->position, ptr, total_bytes);
    file->position += total_bytes;
  }

  return nmemb; // Return number of items written
}

// fseek - Seek in file
int __wrap_fseek(FILE *stream, long offset, int whence) {
  CFileIOController *ctrl = CFileIOController::GetInstance();
  if (!ctrl) {
    return __real_fseek(stream, offset, whence);
  }

  // Check simulated failure
  if (ctrl->seek_should_fail_) {
    errno = ctrl->simulated_errno_ ? ctrl->simulated_errno_ : EIO;
    mock_errno = errno;
    return -1;
  }

  MockRealFile *file = ctrl->GetFileFromFILE(stream);
  if (!file || !file->is_open) {
    errno = EBADF;
    mock_errno = EBADF;
    return -1;
  }

  long new_pos;
  switch (whence) {
  case SEEK_SET:
    new_pos = offset;
    break;
  case SEEK_CUR:
    new_pos = static_cast<long>(file->position) + offset;
    break;
  case SEEK_END:
    new_pos = static_cast<long>(file->data.size()) + offset;
    break;
  default:
    errno = EINVAL;
    mock_errno = EINVAL;
    return -1;
  }

  if (new_pos < 0) {
    errno = EINVAL;
    mock_errno = EINVAL;
    return -1;
  }

  file->position = static_cast<size_t>(new_pos);
  return 0;
}

// ftell - Get current position
long __wrap_ftell(FILE *stream) {
  CFileIOController *ctrl = CFileIOController::GetInstance();
  if (!ctrl) {
    return __real_ftell(stream);
  }

  MockRealFile *file = ctrl->GetFileFromFILE(stream);
  if (!file || !file->is_open) {
    errno = EBADF;
    mock_errno = EBADF;
    return -1;
  }

  return static_cast<long>(file->position);
}

// feof - Check end of file
int __wrap_feof(FILE *stream) {
  CFileIOController *ctrl = CFileIOController::GetInstance();
  if (!ctrl) {
    return __real_feof(stream);
  }

  MockRealFile *file = ctrl->GetFileFromFILE(stream);
  if (!file || !file->is_open) {
    return 1; // EOF on invalid file
  }

  return (file->position >= file->data.size()) ? 1 : 0;
}

// rewind - Rewind file
void __wrap_rewind(FILE *stream) {
  CFileIOController *ctrl = CFileIOController::GetInstance();
  if (!ctrl) {
    __real_rewind(stream);
    return;
  }

  MockRealFile *file = ctrl->GetFileFromFILE(stream);
  if (file) {
    file->position = 0;
  }
}

// fputc - Write single character
int __wrap_fputc(int c, FILE *stream) {
  CFileIOController *ctrl = CFileIOController::GetInstance();
  if (!ctrl) {
    return __real_fputc(c, stream);
  }

  ctrl->write_call_count_++; // Count fputc as a write operation

  if (ctrl->write_should_fail_) {
    errno = ctrl->simulated_errno_ ? ctrl->simulated_errno_ : EIO;
    mock_errno = errno;
    return EOF;
  }

  MockRealFile *file = ctrl->GetFileFromFILE(stream);
  if (!file || !file->is_open || !file->write_mode) {
    errno = EBADF;
    mock_errno = EBADF;
    return EOF;
  }

  // Resize if needed
  if (file->position >= file->data.size()) {
    file->data.resize(file->position + 1);
  }

  file->data[file->position] = static_cast<uint8_t>(c);
  file->position++;

  return c;
}

// vfprintf - Formatted print to file
int __wrap_vfprintf(FILE *stream, const char *format, va_list ap) {
  CFileIOController *ctrl = CFileIOController::GetInstance();
  if (!ctrl) {
    return __real_vfprintf(stream, format, ap);
  }

  if (ctrl->write_should_fail_) {
    errno = ctrl->simulated_errno_ ? ctrl->simulated_errno_ : EIO;
    mock_errno = errno;
    return -1;
  }

  MockRealFile *file = ctrl->GetFileFromFILE(stream);
  if (!file || !file->is_open || !file->write_mode) {
    errno = EBADF;
    mock_errno = EBADF;
    return -1;
  }

  char buffer[4096];
  int len = vsnprintf(buffer, sizeof(buffer), format, ap);

  if (len > 0) {
    size_t old_size = file->data.size();
    file->data.resize(old_size + len);
    std::memcpy(file->data.data() + old_size, buffer, len);
  }

  return len;
}

// fileno - Get file descriptor from FILE*
int __wrap_fileno(FILE *stream) {
  CFileIOController *ctrl = CFileIOController::GetInstance();
  if (!ctrl) {
    return __real_fileno(stream);
  }

  MockRealFile *file = ctrl->GetFileFromFILE(stream);
  if (file) {
    // Generate a unique fake fd
    int fd = ctrl->next_fd_++;
    ctrl->RegisterFD(fd, stream);
    return fd;
  }

  return -1;
}

// fstat - Get file status
int __wrap_fstat(int fd, struct stat *buf) {
  CFileIOController *ctrl = CFileIOController::GetInstance();
  if (!ctrl || !buf) {
    return __real_fstat(fd, buf);
  }

  // Look up FILE* from fd
  FILE *stream = ctrl->GetFILEFromFD(fd);
  if (!stream) {
    errno = EBADF;
    mock_errno = EBADF;
    return -1;
  }

  MockRealFile *file = ctrl->GetFileFromFILE(stream);
  if (file) {
    // Fill in stat structure with mock file info
    memset(buf, 0, sizeof(struct stat));
    buf->st_size = file->data.size();
    buf->st_mode = S_IFREG | 0644;
    return 0;
  }

  errno = EBADF;
  mock_errno = EBADF;
  return -1;
}

} // extern "C"
