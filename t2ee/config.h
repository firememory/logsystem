/*

*/

#ifndef CONFIG_H_
#define CONFIG_H_

#define HAS_CONFIG                      1

//////////////////////////////////////////////////////////////////////////
// platform
#if !defined(HAS_PLATFORM)
# define HAS_PLATFORM
# if defined(_MSC_VER)
#   define PLATFORM_API_WINDOWS         1
#   define PLATFORM_API_POSIX           0
# else  // _MSC_VER
#   error "PLATFORM is unknown"
# endif // !_MSC_VER
#endif // HAS_PLATFORM

// 
#if defined(_MSC_VER)
#define COMPILER_MSVC
#endif // MSC_VER

// export & import
#if !defined(HAS_DECLSPEC)
# define HAS_DECLSPEC                   1
# define SYMBOL_EXPORT                  __declspec(dllexport)
# define SYMBOL_IMPORT                  __declspec(dllimport)
#else
# define HAS_DECLSPEC                   0  
#endif // HAS_DECLSPEC

// namespace
#if !defined(HAS_NAMESPACE)
# define HAS_NAMESPACE                  1
# define NAMESPACE_NAME                 tls
# define BEGIN_NAMESPACE                namespace NAMESPACE_NAME {
# define END_NAMESPACE                  } // namespace
# define USING_NAMESPACE                using namespace NAMESPACE_NAME
# define USING(x)                       using NAMESPACE_NAME::x
#else
# define HAS_NAMESPACE                  0
# define BEGIN_NAMESPACE
# define END_NAMESPACE
# define USING_NAMESPACE
# define USING(x)
#endif // BEGIN_NAMESPACE


#if defined (NDEBUG)
# undef DEBUG_ON
#else
# define DEBUG_ON
#endif // NDEBUG

#if !defined(ASSERT)
#include <assert.h>
# define ASSERT                         assert
#endif // ASSERT

#if !defined(TRACE) && !defined(HAS_TRACE)
# define HAS_TRACE                      1
# if !defined(DEBUG_ON)
#   define TRACE
#   define detect_memory_leaks(x)
# else
#   define TRACE                        PRINTF("\n");PRINTF
# endif // DEBUG_ON
#else
# define HAS_TRACE                      0
#endif // 

//////////////////////////////////////////////////////////////////////////
// crt memory leak
#if defined(ENABLE_DETECT_MEMORY_LEAK)
# ifdef DEBUG_ON
#   define __ENABLE_DETECT_MEMORY_LEAK
#   define _CRTDBG_MAP_ALLOC
#   include <stdlib.h>
#   include <crtdbg.h>
#   ifdef __cplusplus
#     ifndef DBG_NEW
#       define DBG_NEW new ( _CLIENT_BLOCK, __FILE__ , __LINE__ )
#       define new DBG_NEW
#     endif
#   endif  /* __cplusplus */
# endif /* DEBUG_ON */
#endif

#if defined(__ENABLE_DETECT_MEMORY_LEAK)
# define detect_memory_leaks(x)         _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF)
#else
# define detect_memory_leaks(x)
#endif // ENABLE_DETECT_MEMORY_LEAKS

//////////////////////////////////////////////////////////////////////////
// 
#ifndef interface
# define interface                      struct
#endif //interface

#include "basictypes.h"

//////////////////////////////////////////////////////////////////////////
// extern type
typedef uint32_t                        uuid;
static const uuid NULL_UUID             = 0;

#if !defined(HAS_FILE_SIZE_T)
# define HAS_FILE_SIZE_T                1
typedef int64_t                         file_size_t;
#else
# define HAS_FILE_SIZE_T                0
#endif // HAS_FILE_SIZE_T


#endif // CONFIG_H_

