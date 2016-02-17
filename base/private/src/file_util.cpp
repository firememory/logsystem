/*

*/

#include "file_util.h"

BEGIN_NAMESPACE_BASE

//////////////////////////////////////////////////////////////////////////
//#include <stdio.h>

file_handle_t OpenFile(const char_t* filename, const char_t* mode) {
  return fopen(filename, mode);
}

bool_t CloseFile(file_handle_t file) {
  return 0 == fclose((FILE*)file) ? TRUE : FALSE;
}

rc_t ReadFile(uint8_t* data, file_size_t* size, file_handle_t file, file_size_t pos) {

  ASSERT(file);
  ASSERT(data);
  ASSERT(size);

  // seek
  if (kuint64max != pos &&  0 != _fseeki64((FILE*)file, pos, SEEK_SET)) { return RC_S_FAILED; }

  // read
  (*size) = fread(data, 1, (size_t)(*size), (FILE*)file);
  return (*size) ? RC_S_OK : RC_S_FAILED;
}

rc_t WriteFile(file_handle_t file, file_size_t pos, const uint8_t* data, file_size_t* size) {

  ASSERT(file);
  ASSERT(data);
  ASSERT(size);

  // seek
  if (kuint64max != pos &&  0 != _fseeki64((FILE*)file, pos, SEEK_SET)) { return RC_S_FAILED; }

  // read
  (*size) = fwrite(data, 1, (size_t)(*size), (FILE*)file);
  return (*size) ? RC_S_OK : RC_S_FAILED;
}

//////////////////////////////////////////////////////////////////////////
// no seek
rc_t ReadFile(uint8_t* data, file_size_t* size, file_handle_t file) {

  ASSERT(file);
  ASSERT(data);
  ASSERT(size);

  // read
  (*size) = fread(data, 1, (size_t)(*size), (FILE*)file);
  return (*size) ? RC_S_OK : RC_S_FAILED;
}


rc_t WriteFile(file_handle_t file, const uint8_t* data, file_size_t* size) {

  ASSERT(file);
  ASSERT(data);
  ASSERT(size);

  // read
  (*size) = fwrite(data, 1, (size_t)(*size), (FILE*)file);
  return (*size) ? RC_S_OK : RC_S_FAILED;
}

rc_t FlushFile(file_handle_t file) {

  ASSERT(file);

  return 0 == fflush(file) ? RC_S_OK : RC_S_FAILED;
}

//////////////////////////////////////////////////////////////////////////
#if defined(PLATFORM_API_WINDOWS)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT  0x0500
#endif
#include <Windows.h>
//#include <WinUser.h>

// CharNext
#if !defined(CHARNEXT)
# define CHARNEXT(x)                     (x) + sizeof(char_t)
#endif // CHARNEXT

Time FromFileTime(const FILETIME& ft) {

  LARGE_INTEGER size;
  size.HighPart = ft.dwHighDateTime;
  size.LowPart = ft.dwLowDateTime;
  return size.QuadPart / 10;
}

EXTERN_C
void FindData2FileInfo(PlatformFileInfo* info, WIN32_FIND_DATA& file_info) {

  LARGE_INTEGER size;
  size.HighPart = file_info.nFileSizeHigh;
  size.LowPart = file_info.nFileSizeLow;
  info->size = size.QuadPart;
  info->is_directory =
    (file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
  info->is_symbolic_link = FALSE; // Windows doesn't have symbolic links.
  info->last_modified = /*base::Time::*/FromFileTime(file_info.ftLastWriteTime);
  info->last_accessed = /*base::Time::*/FromFileTime(file_info.ftLastAccessTime);
  info->creation_time = /*base::Time::*/FromFileTime(file_info.ftCreationTime);
}

#include <io.h>
bool_t GetPlatformFileInfo(PlatformFile file, PlatformFileInfo* info) {

  if (NULL == file || NULL == info) { return FALSE; }

  HANDLE hFile = (HANDLE)_get_osfhandle(_fileno(file));

  BY_HANDLE_FILE_INFORMATION file_info;
  if (GetFileInformationByHandle(hFile, &file_info) == 0)
    return FALSE;

  LARGE_INTEGER size;
  size.HighPart = file_info.nFileSizeHigh;
  size.LowPart = file_info.nFileSizeLow;
  info->size = size.QuadPart;
  info->is_directory =
    (file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
  info->is_symbolic_link = FALSE; // Windows doesn't have symbolic links.
  info->last_modified = /*base::Time::*/FromFileTime(file_info.ftLastWriteTime);
  info->last_accessed = /*base::Time::*/FromFileTime(file_info.ftLastAccessTime);
  info->creation_time = /*base::Time::*/FromFileTime(file_info.ftCreationTime);
  return TRUE;
}

BOOL WINAPI MakeSureDirectoryPathExists(LPCTSTR pszDirPath)
{
  //LPTSTR p, pszDirCopy;
  
  // Make a copy of the string for editing.

  __try
  {
    char_t pszDirCopy[MAX_PATH];
    LPTSTR p;
    DWORD dwAttributes;

//     pszDirCopy = (LPTSTR)malloc(sizeof(TCHAR) * (lstrlen(pszDirPath) + 1));
// 
//     if(pszDirCopy == NULL)
//       return FALSE;

    lstrcpy(pszDirCopy, pszDirPath);

    p = pszDirCopy;

    // If the second character in the path is "\", then this is a UNC
    // path, and we should skip forward until we reach the 2nd \ in the path.

    if((*p == TEXT('\\')) && (*(p+1) == TEXT('\\')))
    {
      p++;            // Skip over the first \ in the name.
      p++;            // Skip over the second \ in the name.

      // Skip until we hit the first "\" ().

      while(*p && *p != TEXT('\\'))
      {
        p = CHARNEXT(p);
      }

      // Advance over it.

      if(*p)
      {
        p++;
      }

      // Skip until we hit the second "\" ().

      while(*p && *p != TEXT('\\'))
      {
        p = CHARNEXT(p);
      }

      // Advance over it also.

      if(*p)
      {
        p++;
      }

    }
    else if(*(p+1) == TEXT(':')) // Not a UNC. See if it's <drive>:
    {
      p++;
      p++;

      // If it exists, skip over the root specifier

      if(*p && (*p == TEXT('\\') || *p == TEXT('/')))
      {
        p++;
      }
    }

    while(*p)
    {
      if(*p == TEXT('\\') || *p == TEXT('/'))
      {
        *p = TEXT('\0');
        dwAttributes = GetFileAttributes(pszDirCopy);

        // Nothing exists with this name. Try to make the directory name and error if unable to.
        if(dwAttributes == 0xffffffff)
        {
          if(!CreateDirectory(pszDirCopy, NULL))
          {
            if(GetLastError() != ERROR_ALREADY_EXISTS)
            {
              //free(pszDirCopy);
              return FALSE;
            }
          }
        }
        else
        {
          if((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
          {
            // Something exists with this name, but it's not a directory... Error
            //free(pszDirCopy);
            return FALSE;
          }
        }

        *p = TEXT('\\');
      }

      p = CHARNEXT(p);
    }
  }
  __except(EXCEPTION_EXECUTE_HANDLER)
  {
    // SetLastError(GetExceptionCode());
    //free(pszDirCopy);
    return FALSE;
  }

  //free(pszDirCopy);
  return TRUE;
}


bool_t MakeDirectory(const char_t* full_path) {

  return MakeSureDirectoryPathExists(full_path);
  //return 0 == _mkdir(full_path) ? TRUE : FALSE;
}

bool_t Move(const char_t* from_path, const char_t* to_path) {

  // NOTE: I suspect we could support longer paths, but that would involve
  // analyzing all our usage of files.
  if (STRLEN(from_path) >= MAX_PATH ||
    STRLEN(to_path) >= MAX_PATH) {
      return FALSE;
  }
  //if (MoveFileEx(from_path, to_path, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING) != 0)
  if (MoveFileEx(from_path, to_path, 0) != 0)
    return TRUE;

  return FALSE;
}

bool_t Copy(const char_t* from_path, const char_t* to_path) {

  // NOTE: I suspect we could support longer paths, but that would involve
  // analyzing all our usage of files.
  if (STRLEN(from_path) >= MAX_PATH ||
    STRLEN(to_path) >= MAX_PATH) {
      return FALSE;
  }
  if (CopyFileEx(from_path, to_path, NULL, NULL, NULL, COPY_FILE_FAIL_IF_EXISTS) != 0)
    return TRUE;

  return FALSE;
}

bool_t Delete(const char_t* path, bool_t recursive) {
  UNUSED_PARAM(recursive);
  return DeleteFile(path) ? TRUE : FALSE;
}

bool_t LockFile(file_handle_t file) {

  HANDLE hFile = (HANDLE)_get_osfhandle(_fileno(file));
  if (NULL == hFile) { return FALSE; }

  OVERLAPPED ovlp;
  BZERO(&ovlp, sizeof(OVERLAPPED));
  return LockFileEx(hFile, LOCKFILE_EXCLUSIVE_LOCK, 0, 0, 0, &ovlp);
}

bool_t UnLockFile(file_handle_t file) {

  HANDLE hFile = (HANDLE)_get_osfhandle(_fileno(file));
  if (NULL == hFile) { return FALSE; }

  OVERLAPPED ovlp;
  BZERO(&ovlp, sizeof(OVERLAPPED));
  return UnlockFileEx(hFile, 0, 0, 0, &ovlp);
}

#else
# error "FILE API is unknown"
#endif

//////////////////////////////////////////////////////////////////////////

const char_t* GetFileName(const char_t* strPath) {

  const char_t* strFind = strPath;
  const char_t* strFindLast = strFind;
  while (NULL != (strFind = STRCHR(strFind, STRLEN(strFind), '\\'))) { ++strFind; strFindLast = strFind; }

  if (strFindLast) { 
    strFind = strFindLast;
    while (NULL != (strFind = STRCHR(strFind, STRLEN(strFind), CHAR_DIR_SEP))) { ++strFind; strFindLast = strFind; }
  }

  return strFindLast;
}

const char_t* ChangeDirToUnixStype(char_t* strPath) {

  /*
  char_t* strFind = strPath;
  while (NULL != (strFind = STRCHR(strFind, '\\'))) { (*strFind) = '/'; ++strFind; }
  return strPath;
  */

  char * string = strPath;
  int ch = '\\';

  while (*string) {

    if (*string == (char)ch) { *string = CHAR_DIR_SEP; }
    string++;
  }
  return strPath;
}

const char_t* ChangeDirStype(char_t* strPath) {

  char * string = strPath;
  int ch = '\\';

  while (*string) {

    if (*string == (char)ch) { *string = CHAR_DIR_SEP; }
    string++;
  }

  // del last
  while (CHAR_DIR_SEP == (*string - 1))  {
    --string;
  }

  *(string + 1) = 0x00;
  return strPath;
}

END_NAMESPACE_BASE
