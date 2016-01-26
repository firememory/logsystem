/*

*/

#ifndef CODE_UTIL_H_
#define CODE_UTIL_H_

#include "base.h"
#include "base_export.h"

//////////////////////////////////////////////////////////////////////////
static const char_t* kStrAttrValueEnd = _STR(" ");
static char const Base16Encode[] = "0123456789ABCDEF";


BEGIN_NAMESPACE_BASE

EXTERN_C BASE_EXPORT
rc_t ParseString(char_t* strDest, uint32_t nDestSize,
                 const char_t* strText, size_t nTextLen,
                 const char_t* strBegin, const char_t* strEnd);

//////////////////////////////////////////////////////////////////////////

EXTERN_C BASE_EXPORT
void ToBase16_WT(uint8_t* dest, const uint8_t* src, size_t len);

#define ToBase16(d,l)         ToBase16_WT((d), (d), (l))

END_NAMESPACE_BASE

#endif // FILE_UTIL_H_
