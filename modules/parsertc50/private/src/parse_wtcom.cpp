/*

*/

#include "parse_wtcom.h"

BEGIN_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////
// WtDict2
void WtDict2::Release() { delete this;}

uint32_t WtDict2::GetVersion() /*const*/ { return (uint32_t)m_CWtDict2.GetVersion(); }

bool_t WtDict2::isValid() /*const*/ { return m_bLoad && m_CWtDict2.GetStructNum() ? TRUE : FALSE; }

bool_t WtDict2::Load(const uint8_t* data, uint32_t len) {

  if (NULL == data || 0 == len) { return FALSE; }

  m_bLoad = m_CWtDict2.ImportCommon((const LPVOID)data, len);

  if (TRUE == m_bLoad) { makeDict(); }
  return m_bLoad;
}

void* WtDict2::GetDict() const { return (void*)(&m_CWtDict2); }

WtDict2::WtDict2(const uint8_t* data, uint32_t len)
 : m_CWtDict2()
 , m_bLoad(FALSE)

#ifndef USE_WTCOM_PARSER_ANS
 , m_mapStructID()
#endif // USE_WTCOM_PARSER_ANS
{
  Load(data, len);
}

WtDict2::~WtDict2() {
#ifndef USE_WTCOM_PARSER_ANS
  m_mapStructID.clear();
#endif // USE_WTCOM_PARSER_ANS
}


//////////////////////////////////////////////////////////////////////////

void WtDict2::makeDict() {
#ifndef USE_WTCOM_PARSER_ANS
  uint16_t nStructNum = m_CWtDict2.GetStructNum();
  for (uint16_t idx = 0; idx < nStructNum; ++idx) {

    uint16_t nStructID = m_CWtDict2.GetStructID(idx);
    CONST LPWTSTRUCT_INFO lpStructInfo = m_CWtDict2.GetStructInfo(nStructID);
    if (NULL == lpStructInfo || NULL == lpStructInfo->m_pFieldID) { continue; }

    m_mapStructID[nStructID] = StructInfo();

    StructInfo& si = m_mapStructID[nStructID];

    for (uint16_t idx_field = 0; idx_field < lpStructInfo->m_wFieldNum; ++idx_field) {

      si.InsertFuncID(lpStructInfo->m_pFieldID[idx_field], idx_field);
    }
  }
#endif // USE_WTCOM_PARSER_ANS
}

rc_t WtDict2::HaveFuncID(uint16_t wStructID, uint16_t nFuncID, uint16_t* nPos) const {

  ASSERT(nPos);
#ifndef USE_WTCOM_PARSER_ANS
  struct_map_t::const_iterator it = m_mapStructID.find(wStructID);
  if (m_mapStructID.end() == it) { return RC_S_NOTFOUND; }

  return (it->second).HasField(nFuncID, nPos);
#else

  WtDict2& wtDict2 = *(WtDict2*)(this);
  CONST LPWTSTRUCT_INFO lpWtStruct = wtDict2.m_CWtDict2.GetStructInfo(wStructID);
  if (NULL == lpWtStruct || NULL == lpWtStruct->m_pFieldID) { return RC_S_FAILED; }

  for (uint16_t idx = 0; idx < lpWtStruct->m_wFieldNum; ++idx) {

    if (nFuncID == lpWtStruct->m_pFieldID[idx]) {
      (*nPos) = idx;
      return RC_S_OK;
    }
  }
  return RC_S_NOTFOUND;
#endif // USE_WTCOM_PARSER_ANS
}

//////////////////////////////////////////////////////////////////////////
// ParseWtcom2
void ParseWtcom2::Release() { delete this; }

rc_t ParseWtcom2::SetDict(const IDictIX* pDict) {

  if (NULL == pDict || NULL == pDict->GetDict()) { return RC_S_NULL_VALUE; }

  m_pIDictIX = pDict;
  m_CWtCommon.SetDict((CWtDict2*)(pDict->GetDict()));
  return RC_S_OK;
}

rc_t ParseWtcom2::Parse(uint16_t wStructID, const char_t* data, uint32_t len) {

  ASSERT(m_pIDictIX);
  return 0 == m_CWtCommon.CreateStruct(wStructID, (void *)data, len) ? RC_S_OK : RC_S_FAILED;
}

rc_t ParseWtcom2::get_data(uint16_t idx, char_t* val, uint32_t len) {

  ASSERT(m_pIDictIX);
  LPCSTR pcszRet = m_CWtCommon.GetItemValueFromID(idx, val, (WORD)(len));
  if (NULL == pcszRet) { return RC_S_FAILED; }
  return RC_S_OK;
}

rc_t ParseWtcom2::get_data(uint16_t idx, uint32_t* val) {

  ASSERT(m_pIDictIX);
  ASSERT(val);

  uint32_t lRet = m_CWtCommon.GetItemLongValueFromID(idx);
  if (0XFFFFFFFF == lRet) { return RC_S_FAILED; }
  (*val) = lRet;
  return RC_S_OK;
}

rc_t ParseWtcom2::GetReturnNo(int32_t* nRet) {
  ASSERT(m_pIDictIX);
  (*nRet) = m_CWtCommon.GetReturnNo();
  return RC_S_OK;
}

rc_t ParseWtcom2::GetErrmsg(char_t* strMsg, uint32_t nLen) {
  ASSERT(m_pIDictIX);
  return NULL == m_CWtCommon.GetErrmsg(strMsg, (uint16_t)nLen) ? RC_S_FAILED : RC_S_OK;
}

uint32_t ParseWtcom2::GetLineCount() {
  ASSERT(m_pIDictIX);
  return m_CWtCommon.GetLineCount();
}

rc_t ParseWtcom2::GetCookies(char_t* strCookies, uint32_t nLen) {
  ASSERT(m_pIDictIX);
  return NULL == m_CWtCommon.GetCookies(strCookies, (uint16_t)nLen) ? RC_S_FAILED : RC_S_OK;
}

rc_t ParseWtcom2::Next() {
  ASSERT(m_pIDictIX);
  return FALSE == m_CWtCommon.MoveNext() ? RC_S_FAILED : RC_S_OK;
}

rc_t ParseWtcom2::HaveFuncID(uint16_t wStructID, uint16_t nFuncID, uint16_t* nPos) const {

  ASSERT(nPos);
  ASSERT(m_pIDictIX);
  return m_pIDictIX->HaveFuncID(wStructID, nFuncID, nPos);
}

//////////////////////////////////////////////////////////////////////////
ParseWtcom2::ParseWtcom2(const IDictIX* pDict)
  : m_CWtCommon()
  , m_pIDictIX(NULL)
{
  SetDict(pDict);
}

ParseWtcom2::~ParseWtcom2() {}



//////////////////////////////////////////////////////////////////////////
IDictIX* IDictIX::CreateInstance(const char_t* strType, const uint8_t* data, uint32_t len) {

  if (0 == STRCMP(kStrWtDict2, strType)) {
    return new WtDict2(data, len);
  }
  return NULL;
}


IParseIX* IParseIX::CreateInstance(const char_t* strType, const IDictIX* pIDictIX) {

  if (0 == STRCMP(kStrParseWtComm, strType)) {
    return new ParseWtcom2(pIDictIX);
  }
  return NULL;
}

END_NAMESPACE_AGGREGATOR

