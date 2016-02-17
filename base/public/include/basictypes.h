#ifndef BASE_BASICTYPES_H_
#define BASE_BASICTYPES_H_

#include <limits.h>         // So we can set the bounds of our types
#include <stddef.h>         // For size_t
#include <string.h>         // for memcpy

//#include "base/port.h"    // Types that only need exist on certain systems
#include "port.h"
#include "port_c.h"
#if defined(__cplusplus)

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

template<typename T>
inline void ignore_result(const T&) {
}

#endif // _cplusplus
//////////////////////////////////////////////////////////////////////////
// base data type
typedef char                            int8_t;
typedef short                           int16_t;
typedef int                             int32_t;

typedef unsigned char                   uint8_t;
typedef unsigned short                  uint16_t;
typedef unsigned int                    uint32_t;

#if defined(COMPILER_MSVC)
typedef __int64                         int64_t;
typedef unsigned __int64                uint64_t;
#else
typedef long long                       int64_t;
typedef unsigned long long              uint64_t;
#endif // COMPILER_MSVC

typedef float                           float_t;
typedef double                          double_t;

typedef uint32_t                        bool_t;


const uint8_t  kuint8max  = (( uint8_t) 0xFF);
const uint16_t kuint16max = ((uint16_t) 0xFFFF);
const uint32_t kuint32max = ((uint32_t) 0xFFFFFFFF);
const uint64_t kuint64max = ((uint64_t) GG_LONGLONG(0xFFFFFFFFFFFFFFFF));
const  int8_t  kint8min   = ((  int8_t) (-128)/*0x80*/);
const  int8_t  kint8max   = ((  int8_t) 0x7F);
const  int16_t kint16min  = (( int16_t) (-32768)/*0x8000*/);
const  int16_t kint16max  = (( int16_t) 0x7FFF);
const  int32_t kint32min  = (( int32_t) 0x80000000);
const  int32_t kint32max  = (( int32_t) 0x7FFFFFFF);
const  int64_t kint64min  = (( int64_t) GG_LONGLONG(0x8000000000000000));
const  int64_t kint64max  = (( int64_t) GG_LONGLONG(0x7FFFFFFFFFFFFFFF));


//////////////////////////////////////////////////////////////////////////
//
#ifndef NULL
# ifdef __cplusplus
#   define NULL                         0
# else
#   define NULL                         ((void *)0)
# endif
#endif

#ifndef FALSE
# define FALSE                          0
#endif

#ifndef TRUE
# define TRUE                           1
#endif

#ifndef IN
# define IN
#endif

#ifndef OUT
# define OUT
#endif


//#ifndef __cplusplus
#ifndef max
# define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif // max

#ifndef min
# define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif // min
// #endif

//////////////////////////////////////////////////////////////////////////
// unused
#if !defined(UNUSED_PARAM)
#define UNUSED_PARAM(p)	                ((void)(p))	/* to avoid warnings */
#endif

#if !defined(UNUSED_LOCAL_VARIABLE)
#define UNUSED_LOCAL_VARIABLE(lv)	      UNUSED_PARAM(lv)
#endif

//////////////////////////////////////////////////////////////////////////
// extern
#if !defined(HAS_EXTERN_C) && !defined(EXTERN_C)
# define HAS_EXTERN_C                   1
# if defined(__cplusplus)
#   define EXTERN_C                     extern "C"
# else
#   define EXTERN_C
# endif // __cplusplus
#else
# define HAS_EXTERN_C                   0
#endif // HAS_EXTERN_C

//////////////////////////////////////////////////////////////////////////
// memory
#   define MEMCPY                       memcpy_s
#   define MEMSET                       memset
#   define MEMCMP                       memcmp
#   define MEMCHR                       memchr
#   define BZERO(d,l)                   memset((void*)(d), 0, (l))
#   define BZERO_ARR(d)                 memset((void*)(d), 0, sizeof(d))
#   define MEMMOVE                      memmove_s

//////////////////////////////////////////////////////////////////////////
// string
#if !defined(HAS_STRING)
# define HAS_STRING                     1
# if !defined(UNICODE)
typedef char                            char_t;
typedef unsigned char                   uchar_t;
#   define _STR(x)                      (x)
#   define _CHAR(x)                     (x)
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#   define VSNPRINTF                    vsnprintf_s
#   define SNPRINTF                     _snprintf_s
#   define STRCPY                       strcpy_s
#   define STRNCPY                      strncpy_s_
#   define STRLEN                       strlen
#   define STRNLEN                      strnlen
#   define STRCHR                       strchr_s
#   define STRSTR                       strstr_s
//#   define STRSTR(s,l,f)                strstr(s, f)
#   define STRCAT                       strcat_s
#   define STRNCAT                      strncat_s
#   define STRCMP                       strcmp
#   define STRNCMP                      strncmp
#   define STRICMP                      stricmp
#   define STRNICMP                     strnicmp
#   define PRINTF                       printf_s
//#   define SSCANF                       sscanf_s
#   define SSCANF                       _snscanf_s
// 
#   define ATOI                         atoi
#   define ATOI64                       _atoi64
#   define ITOA                         _itoa_s
#   define I64TOA                       _ui64toa_s
# else
# define HAS_STRING                     0
#   error "UNICODE string is not implemented!"
# endif // UNICODE
#endif // HAS_STRING

// const
static const char_t*  NULL_STR          = _STR("");
static const char_t*  STR_DIR_SEP       = _STR("/");
static const char_t   CHAR_DIR_SEP      = _CHAR('/');
static const char_t   END_CHAR          = 0;
const uint32_t kMAX_PATH                = 260;

#endif  // BASE_BASICTYPES_H_
