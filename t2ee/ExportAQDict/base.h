/*
  dll so
*/
#ifndef BASE_H_
#define BASE_H_

#define BEGIN_NAMESPACE_BASE
#define END_NAMESPACE_BASE
#define BASE_EXPORT
#define DEBUG_ON
typedef unsigned int uint32_t;
typedef char char_t;
typedef BOOL bool_t;

#define kMAX_PATH   MAX_PATH
#   define BZERO_ARR(d)                 memset((void*)(d), 0, sizeof(d))

typedef enum rc_e {
  
  /* status codes */
  RC_S_OK                               = 0,  // success
    
    RC_S_INIT                             = 10,
    RC_S_BUSY                             ,
    RC_S_RUNNING                          ,
    RC_S_IDLE                             ,  
    RC_S_OPEN                             ,
    RC_S_CLOSED                           ,
    RC_S_UNKNOWN                          ,
    
    RC_S_NULL_VALUE                       = 100,
    RC_S_STATUS                           ,
    RC_S_NOTFOUND                         ,  /* search operation failed */
    RC_S_DUPLICATE                        ,  /* index restriction violated (duplicate) */
    RC_S_CURSOR_END                       ,  /* cursor cannot be moved */
    RC_S_CURSOR_EMPTY                     ,  /* no objects in index */
    RC_S_LAST                             = 256,      /* 0x100 */
    
    /* error codes and code bases */
    RC_E_CORE                             = 300,
    RC_E_INVALID_HANDLE                   ,  /* normally invalid handle causes a fatal error. */
    RC_E_NOMEM                            ,  /* no memory */
    RC_E_ACCESS                           ,  /* attempt to use a read only transaction for a write operation */
    RC_E_ILLEGAL_PARAM                    ,  /* invalid paramter; e.g. illegal search operation type */
    
    RC_E_REFCOUNT                         ,  /* ref count */
    RC_E_INDEX                            ,  /* idx error */
    RC_E_UNSUPPORTED                      ,  /* unsupported call  */
    RC_E_LAST                             = 65536,    /* 0x10000 */
    
    /* unrecoverable (fatal) error code bases */
    RC_ERR_DB                             = 100000,   /* database */
    RC_ERR_DICT                           = 110000,   /* dictionary */
    RC_ERR_DISK                           = 700000,   /* disk */
    RC_ERR_LAST                           = 16777216, /* 0x1000000 */
    
    /* special codes */
    RC_S_FAILED                           = 20000000, /* plus third system code */
    
    RC_LAST
} rc_t;

#define _STR
#define STRCPY(d,l,s)     strcpy(d,s)
#define STRLEN            strlen
#define STRNLEN(s,l)      strlen(s) 
#define STRSTR(s,l,f)     strstr(s,f)
#define STRCAT(d,l,s)     strcat(d,s)
#define MEMMOVE(d,dl,s,l) memmove(d,s,l)

#define PLATFORM_API_WINDOWS


#endif // BASE_H_
