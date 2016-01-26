/*

*/

#include "code_util.h"

BEGIN_NAMESPACE_BASE

rc_t ParseString(char_t* strDest, uint32_t nDestSize,
                 const char_t* strText, size_t nTextLen,
                 const char_t* strBegin, const char_t* strEnd) {

  const char_t* strValueBegin = STRSTR(strText, nTextLen, strBegin);
  if (NULL == strValueBegin) { return RC_S_NOTFOUND; }
  strValueBegin += STRLEN(strBegin);

  const char_t* strValueEnd = STRSTR(strValueBegin, nTextLen - (strValueBegin - strText), strEnd);
  if (NULL == strValueEnd) { strValueEnd = strText + nTextLen; }

  size_t nValueLen = strValueEnd - strValueBegin;
  if (nValueLen > nDestSize) { return RC_E_NOMEM; }
  STRNCPY(strDest, nDestSize, strValueBegin, nValueLen);

  // XXX
  //strDest[nValueLen] = 0;
  return RC_S_OK;
}

void ToBase16_WT(uint8_t* dest, const uint8_t* src, size_t len) {

  for (int i = (int)(len - 1); i >= 0; --i) {
    uint8_t a     = src[i];
    dest[2*i]     = Base16Encode[(a>>4)&0xf];
    dest[2*i + 1] = Base16Encode[a & 0xf];
  }
}

END_NAMESPACE_BASE

