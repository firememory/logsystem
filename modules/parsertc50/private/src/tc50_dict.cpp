/*

*/

#include "tc50_dict.h"

#include "file_util.h"

BEGIN_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////

USING_BASE(OpenFile);
USING_BASE(ReadFile);
USING_BASE(strAttrOpenRead);

//////////////////////////////////////////////////////////////////////////
rc_t TC50Dict::AddDict(const char_t* strCollector, const char_t* strDir, file_id_t file_id, uint64_t create_time) {

  if (NULL == strCollector || NULL == strDir || IFileNode::INVALID_FILE_ID == file_id) { return RC_S_NULL_VALUE; }

  DictNode* pDictNode = findDict(strCollector, strDir, create_time);
  if (NULL == pDictNode) {

    pDictNode = new DictNode(strCollector, strDir, file_id, create_time);
    if (NULL == pDictNode) { return RC_E_NOMEM; }
  }
  
  ASSERT(pDictNode);
  return m_setDict.end() == (m_setDict.insert(pDictNode)).first ? RC_S_DUPLICATE : RC_S_OK;
}

IDictIX* TC50Dict::FindNearDict_Simple(IFileIteratorBase* IFileIterator
                                      ,const char_t* strCollector, const char_t* strDir, uint64_t sec) {

  //
  if (NULL == IFileIterator || NULL == strCollector || NULL == strDir) { return NULL; }
  DictNode* pDictNode = getNearDict(IFileIterator, strCollector, strDir, sec);
  if (NULL == pDictNode) { return NULL; }
  return pDictNode && pDictNode->m_autoRelDictIXSimple ? pDictNode->m_autoRelDictIXSimple : NULL;
}

IDictIX* TC50Dict::FindNearDict_Common(IFileIteratorBase* IFileIterator
                                    , const char_t* strCollector, const char_t* strDir, uint64_t sec) {

  //
  if (NULL == IFileIterator || NULL == strCollector || NULL == strDir) { return NULL; }
  DictNode* pDictNode = getNearDict(IFileIterator, strCollector, strDir, sec);
  if (NULL == pDictNode) { return NULL; }
  return pDictNode && pDictNode->m_autoRelDictIXCommon ? pDictNode->m_autoRelDictIXCommon : NULL;
}

IDictIX* TC50Dict::FindNearDict_Scntr(IFileIteratorBase* IFileIterator
                                    , const char_t* strCollector, const char_t* strDir, uint64_t sec) {

  //
  if (NULL == IFileIterator || NULL == strCollector || NULL == strDir) { return NULL; }
  DictNode* pDictNode = getNearDict(IFileIterator, strCollector, strDir, sec);
  if (NULL == pDictNode) { return NULL; }
  return pDictNode && pDictNode->m_autoRelDictIXScntr ? pDictNode->m_autoRelDictIXScntr : NULL;
}

IDictIX* TC50Dict::GetDefDict_Simple() {
  return NULL == m_pDictNodeDef ? NULL : m_pDictNodeDef->m_autoRelDictIXSimple;    
}

IDictIX* TC50Dict::GetDefDict_Common() {
  return NULL == m_pDictNodeDef ? NULL : m_pDictNodeDef->m_autoRelDictIXCommon;
}

IDictIX* TC50Dict::GetDefDict_Scntr() {
  return NULL == m_pDictNodeDef ? NULL : m_pDictNodeDef->m_autoRelDictIXScntr;
}

//////////////////////////////////////////////////////////////////////////

static const uint32_t kMaxDictDataSize      = 256 * 1024;
//////////////////////////////////////////////////////////////////////////

TC50Dict::TC50Dict()
 : m_setDict()
 , m_pDictNodeDef(NULL)
{
  m_pDictNodeDef = new DictNode(NULL_STR, NULL_STR, IFileNode::INVALID_FILE_ID, 0);  
}

TC50Dict::~TC50Dict() {

  clearDictMap();
}

rc_t TC50Dict::LoadDefaultDict(const char_t* strPath) {

  if (NULL == m_pDictNodeDef) { return RC_S_FAILED; }

  /*
  for (uint32_t idx = 0; idx < 3; ++idx) {

    char_t strDefaultPath[kMAX_PATH] = {0};
    uint8_t strFileData[kMaxDictDataSize] = {0};

    if (0 >= SNPRINTF(strDefaultPath, sizeof(strDefaultPath), _STR("%s/default%u.sto"), strPath, idx)) {
      continue;
    }

    AutoReleaseFile autoRelFile(OpenFile(strDefaultPath, strAttrOpenRead));
    if (NULL == autoRelFile) { continue; }

    file_size_t read_size = kMaxDictDataSize;
    if (RC_S_OK != ReadFile(strFileData, &read_size, autoRelFile, 0x00)) {
      return RC_S_FAILED;
    }

    m_pDictNodeDef->LoadDictFromFile(strFileData, (uint32_t)read_size);
  }
  */

  char_t strDefaultPath[kMAX_PATH] = {0};
  uint8_t strFileData[kMaxDictDataSize] = {0};

  if (0 >= SNPRINTF(strDefaultPath, sizeof(strDefaultPath), _STR("%s/default.sto"), strPath)) {
    return RC_S_FAILED;
  }

  AutoReleaseFile autoRelFile(OpenFile(strDefaultPath, strAttrOpenRead));
  if (NULL == autoRelFile) { return RC_S_FAILED; }

  file_size_t read_size = kMaxDictDataSize;
  if (RC_S_OK != ReadFile(strFileData, &read_size, autoRelFile, 0x00)) {
    return RC_S_FAILED;
  }

  return m_pDictNodeDef->LoadDictFromFile(strFileData, (uint32_t)read_size);
}

//////////////////////////////////////////////////////////////////////////
void TC50Dict::clearDictMap() {

  dict_set_t::iterator it_list, end;
  for (it_list = m_setDict.begin(), end = m_setDict.end(); it_list != end; ++it_list) {

    DictNode* dictNode = (*it_list);
    if (NULL == dictNode) { continue; }

    delete dictNode;
  }
  m_setDict.clear();

  if (m_pDictNodeDef) { delete m_pDictNodeDef; }
}

TC50Dict::DictNode* TC50Dict::getNearDict(IFileIteratorBase* pIFileIterator
                                 , const char_t* strCollector, const char_t* strDir, uint64_t now) {

  //fds
  DictNode dictNode(strCollector, strDir, IFileNode::INVALID_FILE_ID, now);
  dict_set_t::iterator it_set = m_setDict.upper_bound(&dictNode);
  if (m_setDict.end() == it_set) { return NULL; }

  // make sure dict load file
  DictNode* pDictNode = (*it_set);
  if (NULL == pDictNode) { return NULL; }

  uint8_t strFileData[kMaxDictDataSize] = {0};

  file_size_t pos = 0;
  file_size_t size = 0;
  while (pos) {

    IMemoryNode* pIMemoryNode = pIFileIterator->GetFileData(pDictNode->FileID(), &pos);
    if (NULL == pIMemoryNode) { break; }

    size = pos + pIMemoryNode->len();
    if (size > kMaxDictDataSize) {
      MEMCPY(strFileData + pos, sizeof(strFileData) - (uint32_t)pos, pIMemoryNode->data(), kMaxDictDataSize - (uint32_t)pos);
      size = kMaxDictDataSize;
      break;
    }
    else {
      MEMCPY(strFileData + pos, sizeof(strFileData) - (uint32_t)pos, pIMemoryNode->data(), pIMemoryNode->len());
    }
  }
  
  if (0 == size) { return NULL; }
  pDictNode->LoadDictFromFile(strFileData, (uint32_t)size, pIFileIterator);
  return pDictNode;
}

TC50Dict::DictNode* TC50Dict::findDict(const char_t* strCollector, const char_t* strDir, uint64_t create_time) {

  ASSERT(strCollector && strDir);

  DictNode dictNode(strCollector, strDir, IFileNode::INVALID_FILE_ID, create_time);
  dict_set_t::iterator it_set = m_setDict.find(&dictNode);
  if (m_setDict.end() == it_set) { return NULL; }

  return (*it_set);
}

rc_t TC50Dict::DictNode::LoadDictFromFile(const uint8_t* data, uint32_t len) {

  const uint32_t kDictLenBytes          = 4;

  if (NULL == data || len < 3 * kDictLenBytes) { return RC_S_NULL_VALUE; }

  uint32_t current_pos = len;
  // 格式是：登录字典长度（4个字节）+登录字典内存+标准字典长度（4个字节）+标准字典内存
  // + 安全中心字典长度（4个字节）+安全中心字典内存

  const uint8_t* strDictData = data;
  uint32_t nSimpleDictLen = (*(uint32_t*)(strDictData));  
  if (current_pos < nSimpleDictLen + kDictLenBytes) { return RC_S_FAILED; }

  const uint8_t* strSimpleDictData = strDictData + kDictLenBytes;
  if (nSimpleDictLen) { m_autoRelDictIXSimple->Load(strSimpleDictData, nSimpleDictLen); }

  if (current_pos < nSimpleDictLen + 2 * kDictLenBytes) { return RC_S_FAILED; }
  uint32_t nCommonDictLen = (*(uint32_t*)(strDictData + nSimpleDictLen + kDictLenBytes));
  if (current_pos < nSimpleDictLen + nCommonDictLen + 2 * kDictLenBytes) { return RC_S_FAILED; }

  const uint8_t* strCommonDictData = strSimpleDictData + nSimpleDictLen + kDictLenBytes;
  if (nCommonDictLen) { m_autoRelDictIXCommon->Load(strCommonDictData, nCommonDictLen); }

  if (current_pos < nSimpleDictLen + nCommonDictLen + 3 * kDictLenBytes) { return RC_S_FAILED; }
  uint32_t nScntrDictLen = (*(uint32_t*)(strDictData + nSimpleDictLen + kDictLenBytes + nCommonDictLen + kDictLenBytes));
  if (current_pos < nSimpleDictLen + nCommonDictLen + nScntrDictLen + 3 * kDictLenBytes) { return RC_S_FAILED; }

  const uint8_t* strScntrDictData = strCommonDictData + nCommonDictLen + kDictLenBytes;
  m_autoRelDictIXScntr->Load(strScntrDictData, nScntrDictLen);

  return (TRUE == m_autoRelDictIXSimple->isValid() && TRUE == m_autoRelDictIXCommon->isValid()) ? RC_S_OK : RC_S_FAILED;
}

rc_t TC50Dict::DictNode::LoadDictFromFile(uint8_t* buffer, uint32_t buf_len, IFileIteratorBase* pIFileIterator) {

  ASSERT(pIFileIterator);
  ASSERT(buffer && buf_len);

  if (NULL == m_autoRelDictIXSimple || NULL == m_autoRelDictIXCommon) { return RC_S_FAILED; }

  file_size_t current_pos = 0;
  do {
    AutoRelease<IMemoryNode*> autoRelMemNode(pIFileIterator->GetFileData(m_file_id, &current_pos));
    if (NULL == autoRelMemNode) { break; }

    MEMCPY(buffer + current_pos, buf_len - (uint32_t)current_pos
      , autoRelMemNode->data(), autoRelMemNode->len());
    current_pos = current_pos + autoRelMemNode->len();
  } while(current_pos < buf_len);

  if (0 >= current_pos) { return RC_S_FAILED; }

  return LoadDictFromFile(buffer, (uint32_t)current_pos);
}

//////////////////////////////////////////////////////////////////////////
END_NAMESPACE_AGGREGATOR
