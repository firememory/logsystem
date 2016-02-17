/*

*/

#ifndef PORT_C_H_
#define PORT_C_H_
/*
#include "base.h"
#include "base_export.h"

//////////////////////////////////////////////////////////////////////////

BEGIN_NAMESPACE_BASE
*/
//EXTERN_C BASE_EXPORT
char * strstr_s(const char * string, size_t slen, const char * find);

//EXTERN_C BASE_EXPORT
char * strchr_s(const char * string, size_t slen, int ch);

char * strncpy_s_(char * _Dst, size_t dlen, const char * src, size_t slen);

/*
END_NAMESPACE_BASE
*/
#endif // PORT_C_H_
