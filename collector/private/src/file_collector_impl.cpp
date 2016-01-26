/*

*/

#include "file_collector_impl.h"

BEGIN_NAMESPACE_COLLECTOR

//////////////////////////////////////////////////////////////////////////
rc_t FileCollectorImpl::AddCollectRule(const char_t* strDir, const char_t* strExclude, const char_t* strInclude) {

  if (NULL == strDir || NULL == strInclude) { return RC_S_NULL_VALUE; }

  CollectRule* pCollectRule = new CollectRule(strDir, strExclude, strInclude);

  m_listCollectRule.push_back(pCollectRule);
  return RC_S_OK;
}

void FileCollectorImpl::ClearCollectRule() {

  collect_rule_list_t::iterator it_list, end;
  for (it_list = m_listCollectRule.begin(), end = m_listCollectRule.end(); it_list != end; ++it_list) {

    CollectRule* pCollectRule = (*it_list);
    if (NULL == pCollectRule) { continue; }

    delete pCollectRule;
  }
  m_listCollectRule.clear();
}

IFileNode* FileCollectorImpl::AddFileNode(const file_id_t& id, file_size_t pos, bool_t bFinish, const char_t* strDir, const char_t* strName) {

  if (NULL == strDir || NULL == strName) { return NULL; }

  if (IFileNode::INVALID_FILE_ID == id) { return NULL; }

  hash_value_t hash = char_t_hash()(strDir);
  /* find */
  FileNode* pFileNode = findFileNode(hash, strName);
  if (NULL == pFileNode) {
    pFileNode = new FileNode(strDir, NULL, strName, id, pos, bFinish);
    if (NULL == pFileNode) { return NULL; }

    // add to map
    addFileNode(hash, pFileNode->LocalName(), pFileNode);
  }

  if (IFileNode::INVALID_FILE_ID != id) {

    // update id
    pFileNode->SetID(id);
    addFileNode(id, pFileNode);
  }
  return pFileNode;
}

IFileNode* FileCollectorImpl::FindFileNode(const file_id_t& id) {

  file_node_id_map_t::iterator it_map = m_mapFileNodeID.find(id);
  return m_mapFileNodeID.end() == it_map ? NULL : it_map->second;
}

IFileNode* FileCollectorImpl::FindFileNode(const char_t* strDir, const char_t* strName) {

  hash_value_t hash = char_t_hash()(strDir);
  return findFileNode(hash, strName);
}

void FileCollectorImpl::ClearFileNode() {

  clearFileNode();
}
/*
void FileCollectorImpl::ConfirmSend() {

  file_node_map_t::iterator it_map, end;
  for (it_map = m_mapFileNode.begin(), end = m_mapFileNode.end(); it_map != end; ++it_map) {

    FileNode* pFileNode = it_map->second;
    if (NULL == pFileNode) { continue; }

    //pFileNode->ConfirmSend();
  }  
}
*/
rc_t FileCollectorImpl::Collector(collector_log_type_e collector_log_type, file_collect_callback_t pfn_cb, void* context) {

  if (NULL == pfn_cb) { return RC_S_NULL_VALUE; }

  collect_rule_list_t::iterator it_list, end;
  for (it_list = m_listCollectRule.begin(), end = m_listCollectRule.end(); it_list != end; ++it_list) {

    CollectRule* pCollectRule = (*it_list);
    if (NULL == pCollectRule) { continue; }
    
    searchAllFile(collector_log_type, pCollectRule, pfn_cb, context);
  }

  return RC_S_OK;
}

void FileCollectorImpl::TickProc() {

  file_node_map_t::iterator it_map, end;
  for (it_map = m_mapFileNode.begin(), end = m_mapFileNode.end(); it_map != end; ++it_map) {

    FileNode* pFileNode = it_map->second;
    if (NULL == pFileNode) { continue; }

    pFileNode->TickProc();
  }
}
//////////////////////////////////////////////////////////////////////////

FileCollectorImpl::FileNode* FileCollectorImpl::findFileNode(hash_value_t hash, const char_t* strOne) {

  two_string_key_t twoString;
  twoString.hash = hash;
  twoString.one = strOne;

  file_node_map_t::iterator it_map = m_mapFileNode.find(twoString);
  return m_mapFileNode.end() == it_map ? NULL : it_map->second;
}

rc_t FileCollectorImpl::addFileNode(hash_value_t hash, const char_t* strOne, FileNode* pFileNode) {

  two_string_key_t twoString;
  twoString.hash = hash;
  twoString.one = strOne;

  if (m_mapFileNode.end() != m_mapFileNode.find(twoString)) { return RC_S_DUPLICATE; }

  m_mapFileNode[twoString] = pFileNode;
  return RC_S_OK;
}

rc_t FileCollectorImpl::addFileNode(file_id_t file_id, FileNode* pFileNode) {

  if (m_mapFileNodeID.end() != m_mapFileNodeID.find(file_id)) { return RC_S_DUPLICATE; }

  m_mapFileNodeID[file_id] = pFileNode;
  return RC_S_OK;
}

//////////////////////////////////////////////////////////////////////////
FileCollectorImpl::FileCollectorImpl()

  : m_listCollectRule()
  , m_mapFileNode()
  , m_mapFileNodeID()
  , m_bAfterToDay(TRUE)
{}

FileCollectorImpl::~FileCollectorImpl() {

  ClearCollectRule();
  clearFileNode();
}

void FileCollectorImpl::clearFileNode() {

  file_node_map_t::iterator it_map, end;
  for (it_map = m_mapFileNode.begin(), end = m_mapFileNode.end(); it_map != end; ++it_map) {

    FileNode* pFileNode = it_map->second;
    if (NULL == pFileNode) { continue; }

    delete pFileNode;
  }
  m_mapFileNode.clear();
  m_mapFileNodeID.clear();
}


//////////////////////////////////////////////////////////////////////////

#if defined(PLATFORM_API_WINDOWS)

#include <Windows.h>
//#define _WIN32_WINNT  0x0500

// CharNext
#if !defined(CHARNEXT)
# define CHARNEXT(x)                     (x) + sizeof(char_t)
#endif // CHARNEXT


#include <time.h>
#define LOCALTIME(tm, time) localtime_s(&(tm), &(time))

// base/file_util
EXTERN_C void FindData2FileInfo(PlatformFileInfo* info, WIN32_FIND_DATA& file_info);

bool_t FileCollectorImpl::searchAllFile(collector_log_type_e collector_log_type
                                        ,CollectRule* pCollectRule, file_collect_callback_t pfn_cb, void* context) {

  ASSERT(pCollectRule);
  ASSERT(pfn_cb);

  // now
  struct tm today;
  time_t now = time(NULL);
  LOCALTIME(today, now);

  WIN32_FIND_DATA find_file_data;

  //__try 
  {

    HANDLE find_handle = FindFirstFile(pCollectRule->find_dir, &find_file_data);

    if (find_handle != INVALID_HANDLE_VALUE) {
      do {
        // Don't count current or parent directories.
        if ((STRCMP(find_file_data.cFileName, _STR("..")) == 0) ||
          (STRCMP(find_file_data.cFileName, _STR(".")) == 0))
          continue;

        if (find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { continue; }

        /*
        long result = CompareFileTime(&find_file_data.ftCreationTime,  // NOLINT
          &comparison_filetime);
        // File was created after or on comparison time
        if ((result == 1) || (result == 0))
          ++file_count;
        */

        // is we need file
        //uint32_t nMatchLen = STRLEN(find_file_data.cFileName);
        if (TRUE == pCollectRule->regExExclude.didCompile()
          && TRUE == pCollectRule->regExExclude.isMatchOnly(find_file_data.cFileName)
        ) { continue; }

        if (FALSE == pCollectRule->regExInclude.isMatchOnly(find_file_data.cFileName)) { continue; }

        // test file
//         char_t strPath[MAX_PATH + 1] = {0};
//         SNPRINTF(strPath, MAX_PATH, MAX_PATH, "%s/%s", pCollectRule->dir, find_file_data.cFileName);
//         AutoReleaseFile autoRelFile(OpenFile(strPath, strAttrOpenRead));
//         if (NULL == autoRelFile) { continue; }

        // find map
        FileNode* pFileNode;
        if (MEM_NINI_LOG == collector_log_type) {
          pFileNode = findFileNode(pCollectRule->mml_hash, find_file_data.cFileName);
        }
        else {
          pFileNode = findFileNode(pCollectRule->hash, find_file_data.cFileName);
        }
        if (NULL == pFileNode) {

          if (TRUE == m_bAfterToDay) {
          // time control
          SYSTEMTIME nCreateTime;
          SYSTEMTIME stUTC;
          if (FALSE == FileTimeToSystemTime(&(find_file_data.ftCreationTime), &stUTC)
            || FALSE == SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &nCreateTime)
            ) { continue; }

          if (nCreateTime.wYear != today.tm_year + 1900
            || nCreateTime.wMonth != today.tm_mon + 1
            || nCreateTime.wDay != today.tm_mday
            ) { continue; }
          }


//           char_t strPath[MAX_PATH + 1] = {0};
//           SNPRINTF(strPath, MAX_PATH, MAX_PATH, "%s/%s", pCollectRule->dir, find_file_data.cFileName);

//           AutoReleaseFile AutoRelFile(::OpenFile(strPath, strAttrOpenRead));
//           if (NULL == AutoRelFile) { continue; }
// 
          pFileNode = new FileNode(pCollectRule->dir
            , MEM_NINI_LOG == collector_log_type ? pCollectRule->GetMMLDir() : NULL
            , find_file_data.cFileName);
          if (NULL == pFileNode) { continue; }

          // update file info
          FindData2FileInfo(&pFileNode->FileInfo(), find_file_data);

          // add to map
          addFileNode(pCollectRule->hash, pFileNode->LocalName(), pFileNode);

          // new file
          if (RC_S_OK != (pfn_cb)(/*this, */pFileNode, context)) { break; }
        }
        else {

          ASSERT(pFileNode);

          // update file info
          PlatformFileInfo& fileInfo = pFileNode->FileInfo();
          FindData2FileInfo(&fileInfo, find_file_data);

          if (pFileNode->GetSendSize() < fileInfo.size) {

            // new data
            if (RC_S_OK != (pfn_cb)(/*this, */pFileNode, context)) { break; }
          }
        }

      } while (FindNextFile(find_handle,  &find_file_data));

      FindClose(find_handle);
    }
  }
//    __except(EXCEPTION_EXECUTE_HANDLER) {
// 
//     return TRUE;
//   }

  return TRUE;
}

#else
# error "FILE API is unknown"
#endif // PLATFORM_API_WINDOWS


END_NAMESPACE_COLLECTOR
