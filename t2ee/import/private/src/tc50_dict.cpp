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
IDictIX* TC50Dict::FindNearDict_Simple(IFileIteratorBase* /*IFileIterator*/
                                      ,const char_t* /*strCollector*/, const char_t* /*strDir*/, uint64_t /*sec*/) {

  //
  return GetDefDict_Simple();
}

IDictIX* TC50Dict::FindNearDict_Common(IFileIteratorBase* /*IFileIterator*/
                                    , const char_t* /*strCollector*/, const char_t* /*strDir*/, uint64_t /*sec*/) {

  //
  return GetDefDict_Common();
}

IDictIX* TC50Dict::FindNearDict_Scntr(IFileIteratorBase* /*IFileIterator*/
                                    , const char_t* /*strCollector*/, const char_t* /*strDir*/, uint64_t /*sec*/) {

  //
  return GetDefDict_Scntr();
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

  if (m_pDictNodeDef) { delete m_pDictNodeDef; }
}

rc_t TC50Dict::LoadDefaultDict(const char_t* strPath) {

  if (NULL == m_pDictNodeDef || NULL == strPath) { return RC_S_FAILED; }

  const char_t* strDefaultPath = strPath;
  uint8_t strFileData[kMaxDictDataSize] = {0};

  AutoReleaseFile autoRelFile(OpenFile(strDefaultPath, strAttrOpenRead));
  if (NULL == autoRelFile) { return RC_S_FAILED; }

  file_size_t read_size = kMaxDictDataSize;
  if (RC_S_OK != ReadFile(strFileData, &read_size, autoRelFile, 0x00)) {
    return RC_S_FAILED;
  }

  return m_pDictNodeDef->LoadDictFromFile(strFileData, (uint32_t)read_size);
}

//////////////////////////////////////////////////////////////////////////
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
  if (nSimpleDictLen && strSimpleDictData) {
    if (FALSE == m_autoRelDictIXSimple->Load(strSimpleDictData, nSimpleDictLen)
      || FALSE == m_autoRelDictIXSimple->isValid()
    ) { return RC_S_FAILED; }
  }

  if (current_pos < nSimpleDictLen + 2 * kDictLenBytes) { return RC_S_FAILED; }
  uint32_t nCommonDictLen = (*(uint32_t*)(strDictData + nSimpleDictLen + kDictLenBytes));
  if (current_pos < nSimpleDictLen + nCommonDictLen + 2 * kDictLenBytes) { return RC_S_FAILED; }

  const uint8_t* strCommonDictData = strSimpleDictData + nSimpleDictLen + kDictLenBytes;
  if (nCommonDictLen && strCommonDictData) {
    if (FALSE == m_autoRelDictIXCommon->Load(strCommonDictData, nCommonDictLen)
      || FALSE == m_autoRelDictIXCommon->isValid()
    ) { return RC_S_FAILED; }
  }

  if (current_pos < nSimpleDictLen + nCommonDictLen + 3 * kDictLenBytes) { return RC_S_FAILED; }
  uint32_t nScntrDictLen = (*(uint32_t*)(strDictData + nSimpleDictLen + kDictLenBytes + nCommonDictLen + kDictLenBytes));
  if (current_pos < nSimpleDictLen + nCommonDictLen + nScntrDictLen + 3 * kDictLenBytes) { return RC_S_FAILED; }

  const uint8_t* strScntrDictData = strCommonDictData + nCommonDictLen + kDictLenBytes;
  if (nScntrDictLen && strScntrDictData) {
    if (FALSE == m_autoRelDictIXScntr->Load(strScntrDictData, nScntrDictLen)
      || FALSE == m_autoRelDictIXScntr->isValid()
    ) { return RC_S_FAILED; }
  }

  return RC_S_OK;
}

//////////////////////////////////////////////////////////////////////////
END_NAMESPACE_AGGREGATOR
