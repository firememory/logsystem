/*

*/

#ifndef FILE_UTIL_H_
#define FILE_UTIL_H_

#include "base.h"
#include "base_export.h"

#include "time_util.h"

BEGIN_NAMESPACE_BASE
USING_BASE(Time);
//////////////////////////////////////////////////////////////////////////
typedef FILE*                           file_handle_t;
typedef file_handle_t                   PlatformFile;

//////////////////////////////////////////////////////////////////////////

// Used to hold information about a given file.
// If you add more fields to this structure (platform-specific fields are OK),
// make sure to update all functions that use it in file_util_{win|posix}.cc
// too, and the ParamTraits<base::PlatformFileInfo> implementation in
// chrome/common/common_param_traits.cc.
struct /*BASE_EXPORT*/ PlatformFileInfo {
//   PlatformFileInfo();
//   ~PlatformFileInfo();

  // The size of the file in bytes.  Undefined when is_directory is true.
  file_size_t/*int64_t*/ size;

  // True if the file corresponds to a directory.
  bool_t is_directory;

  // True if the file corresponds to a symbolic link.
  bool_t is_symbolic_link;

  // The last modified time of a file.
  /*base::*/Time last_modified;

  // The last accessed time of a file.
  /*base::*/Time last_accessed;

  // The creation time of a file.
  /*base::*/Time creation_time;
};

bool_t GetPlatformFileInfo(PlatformFile file, PlatformFileInfo* info);

//////////////////////////////////////////////////////////////////////////

static const char_t* strAttrOpenRead              = _STR("rb");
static const char_t* strAttrOpenWrite             = _STR("wb");
static const char_t* strAttrOpenReadWrite         = _STR("rb+");

inline bool_t isDirChar(char_t ch) { return (_CHAR('\\') == ch || _CHAR('/') == ch) ? TRUE : FALSE; }

//typedef char_t*                         FilePath;

// Wrapper for fopen-like calls. Returns non-NULL FILE* on success.
BASE_EXPORT file_handle_t OpenFile(const char_t* filename, const char_t* mode);

// Closes file opened by OpenFile. Returns true on success.
BASE_EXPORT bool_t CloseFile(file_handle_t file);

// Lock file
BASE_EXPORT bool_t LockFile(file_handle_t file);

BASE_EXPORT bool_t UnLockFile(file_handle_t file);

// pos == kuint32max, SEEK_CUR

// Reads the given number of bytes from the file into the buffer.  Returns
// the number of read bytes, or -1 on error.
BASE_EXPORT rc_t ReadFile(uint8_t* data, file_size_t* size, file_handle_t file, file_size_t pos);

// Writes the given buffer into the file, overwriting any data that was
// previously there.  Returns the number of bytes written, or -1 on error.
BASE_EXPORT rc_t WriteFile(file_handle_t file, file_size_t pos, const uint8_t* data, file_size_t* size);

// no seek
BASE_EXPORT rc_t ReadFile(uint8_t* data, file_size_t* size, file_handle_t file);
BASE_EXPORT rc_t WriteFile(file_handle_t file, const uint8_t* data, file_size_t* size);

BASE_EXPORT rc_t FlushFile(file_handle_t file);

// Moves the given path, whether it's a file or a directory.
// If a simple rename is not possible, such as in the case where the paths are
// on different volumes, this will attempt to copy and delete. Returns
// true for success.
BASE_EXPORT bool_t Move(const char_t* from_path, const char_t* to_path);

BASE_EXPORT bool_t Copy(const char_t* from_path, const char_t* to_path);

BASE_EXPORT bool_t MakeDirectory(const char_t* full_path);

BASE_EXPORT bool_t Delete(const char_t* path, bool_t recursive);

//////////////////////////////////////////////////////////////////////////
const char_t* GetFileName(const char_t* strPath);
//const char_t* ChangeDirToUnixStype(char_t* strPath);

const char_t* ChangeDirStype(char_t* strPath);

END_NAMESPACE_BASE

//////////////////////////////////////////////////////////////////////////

#include "object.h"
BEGIN_NAMESPACE

USING_BASE(file_handle_t);
USING_BASE(CloseFile);

typedef AutoRelease<file_handle_t>         AutoReleaseFile;

template<>
void AutoReleaseFile::Release() { if (m_handle) { CloseFile(m_handle); m_handle = NULL; } }

END_NAMESPACE

#endif // FILE_UTIL_H_
