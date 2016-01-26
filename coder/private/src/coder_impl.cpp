/*

*/

#include "coder.h"
#include "coder_null.h"
#include "coder_deflate.h"

BEGIN_NAMESPACE_CODER

const char_t* NullCoder::strName     = _STR("NULL");
NullCoder NullCoder::NULL_Coder;

ICoder* ICoder::CreateInstance(const char_t* strType, const char_t* strParam) {

  if (0 == STRCMP(strType, DeflateCoder::strName)) {
    return DeflateCoder::CreateInstance(strParam);
  }
  else {
    return NullCoder::CreateInstance(strParam);
  }
}

ICoder* Coder_CreateInstance(const char_t* strType, const char_t* strParam) {

  return ICoder::CreateInstance(strType, strParam);
}

END_NAMESPACE_CODER
