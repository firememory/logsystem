
typedef char            char_t;
typedef unsigned char   uint8_t;
typedef unsigned int    uint32_t;
typedef uint32_t        bool_t;

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

#   define MEMCHR                       memchr
#   define STRCHR                       strchr_s
#   define STRSTR                       strstr_s
#   define BZERO(d,l)                   memset((void*)(d), 0, (l))
#   define STRCHR                       strchr_s
#   define STRLEN                       strlen
#   define STRCPY                       strcpy_s
#   define STRNCPY                      strncpy_s_

#include <string.h>
#include <list>

char * strncpy_s_(char * _Dst, size_t dlen, const char * src, size_t slen) {
  if (dlen <= slen) {
    strncpy(_Dst, src, dlen - 1);
    _Dst[dlen - 1] = 0;
  }
  else {
    strncpy(_Dst, src, slen);
    _Dst[slen] = 0;
  }
  return _Dst;
}

char * strstr_s(const char *s, size_t slen, const char *find) {
  char c, sc;  size_t len;
  if ((c = *find++) != '\0') {
    len = strlen(find);
    do {
      do {
        if (slen-- < 1 || (sc = *s++) == '\0')
          return (NULL);
      } while (sc != c);
      if (len > slen)
        return (NULL);
    } while (strncmp(s, find, len) != 0);
    s--;
  }
  return ((char *)s);
}

char * strchr_s(const char * string, size_t slen, int ch) {

  size_t idx = 0;
  while (*string && *string != (char)ch && idx < slen) {
    string++; ++idx;
  }

  if (*string == (char)ch)
    return((char *)string);
  return(NULL);
}


typedef enum collector_log_type_e_ {
  FULL_LOG
  , MEM_NINI_LOG
} collector_log_type_e;


void setMemMiniLogIncludeLog(const char_t*);

typedef struct includt_log_t {
  char_t strText[64];
} includt_log;
typedef std::list<includt_log_t> include_log_list_t;
include_log_list_t          m_listMemMiniLogIncludeLog;

void setMemMiniLogIncludeLog(const char_t* strIncludeLog) {

  const char_t* strFind = NULL;

  do {

    m_listMemMiniLogIncludeLog.push_back(includt_log());
    includt_log& il = m_listMemMiniLogIncludeLog.back();

    strFind = STRCHR(strIncludeLog, STRLEN(strIncludeLog), ',');
    if (NULL == strFind)  {
      STRCPY(il.strText, sizeof(il.strText), strIncludeLog);
      break;
    }

    STRNCPY(il.strText, sizeof(il.strText), strIncludeLog, strFind - strIncludeLog);
    strIncludeLog = strFind + 1;

  } while (*strIncludeLog);
}

uint32_t preProcLogFile(collector_log_type_e collector_log_type, uint32_t* nMemMiniLogData
                                      , uint8_t* pLogData, uint32_t nLen) {

  // filter
  if (MEM_NINI_LOG == collector_log_type) {

    uint32_t nLogLen = nLen;
    const char_t* strLog = (const char_t*)pLogData;
#define IS_HAVE_INCLUDE_LOG(x)    (x & kHaveLastIncludeLog)
#define SET_HAVE_INCLUDE_LOG(x)   (x | kHaveLastIncludeLog)
#define UNSET_HAVE_INCLUDE_LOG(x) (x | kHaveLastIncludeLog)

    // last include log
    static const uint32_t kHaveLastIncludeLog = 1 << 1;
    if (IS_HAVE_INCLUDE_LOG(*nMemMiniLogData)) {

      // get last log
      const char_t* strFindEnd = (const char_t*)MEMCHR(strLog, '\n', nLogLen);
      if (NULL == strFindEnd) {
        // All log is used.
        return (uint32_t)(strFindEnd - (const char_t*)pLogData); }

      const char_t* strFindEndZero = (const char_t*)MEMCHR(strLog, 0x00, nLogLen);
      if (strFindEndZero && strFindEndZero < strFindEnd) {
        strLog = (const char_t*)strFindEndZero + 1;
      }
      else {
        strLog = strFindEnd + 1;
      }

      // next is data
      strFindEnd = (const char_t*)MEMCHR(strLog, '\n', nLogLen);
      if (strFindEnd && '|' == *strFindEnd) { strLog = strFindEnd + 1; }

      nLogLen = nLen - (strLog - (const char_t*)pLogData);
      UNSET_HAVE_INCLUDE_LOG(*nMemMiniLogData);
    }

    bool_t bIsIncludeLog = FALSE;
    while (strLog < (const char_t*)pLogData + nLen ) {

      // find \r\n
      const char_t* strFindEnd = (const char_t*)MEMCHR(strLog, '\n', nLogLen);
      if (strFindEnd) {

        // fix strlog
        do 
        {
          const char_t* strFindEndZero = (const char_t*)MEMCHR(strLog, 0x00, nLogLen);
          if (NULL == strFindEndZero || strFindEndZero > strFindEnd) { break; }

          nLogLen -= (uint32_t)(strFindEndZero - strLog);
          strLog = strFindEndZero + 1;
        } while(strLog < ((const char_t *)pLogData + nLen));
      }
      else {
        strFindEnd = (const char_t *)pLogData + nLen;
        return (uint32_t)(strFindEnd - (const char_t*)pLogData);
      }

      //ASSERT(strFindEnd);
      // is data
      if (TRUE == bIsIncludeLog) {
        const char_t* nLastChar = strFindEnd;
        while (' ' == *(nLastChar)
          || '\r' == *(nLastChar)
          || '\n' == *(nLastChar)
          )
        {
          --nLastChar;
        }

        if ('|' == *(nLastChar)) {
          strLog = strFindEnd + 1;
          bIsIncludeLog = FALSE;
          continue;
        }
      }

      // find include log req
      {
        bIsIncludeLog = FALSE;
        include_log_list_t::const_iterator it_list, end;
        for (it_list = m_listMemMiniLogIncludeLog.begin(), end = m_listMemMiniLogIncludeLog.end()
          ; it_list != end; ++it_list
          )
        {
          const includt_log& il = *(it_list);
          if (STRSTR(strLog, strFindEnd - strLog, il.strText)) {
            // save this.
            bIsIncludeLog = TRUE;
            break;
          }
        }

        // clear data
        if (FALSE == bIsIncludeLog) { BZERO(strLog, strFindEnd - strLog + 1); }
      }

      strLog = strFindEnd + 1;
      nLogLen = nLen - (strLog - (const char_t *)pLogData);
      printf("%u, ", strLog - (const char_t*)pLogData);
    }
  }

  return nLen;
}



int main() {

  setMemMiniLogIncludeLog("0-94,0-96,0-98,0-100");

  FILE* outFile = fopen("z:/o/20120823.log.s", "wb");
  FILE* inFile = fopen("z:/o/20120823.log", "rb");
  uint8_t strFileData[256*1024 + 1024];

  uint32_t nCurrSize = 0;
  uint32_t nReadSize = 0;
  uint32_t nMemMiniLogData = 0;
  do {

    nReadSize = 256*1024 - nCurrSize;
    nReadSize = (uint32_t)fread(strFileData + nCurrSize, 1, nReadSize, inFile);
    nReadSize = nReadSize + nCurrSize;

    nCurrSize = preProcLogFile(MEM_NINI_LOG, &nMemMiniLogData, strFileData, nReadSize);
    fwrite(strFileData, nCurrSize, 1, outFile);
    memmove_s(strFileData, sizeof(strFileData), strFileData + nCurrSize, nReadSize - nCurrSize);
    nCurrSize = nReadSize - nCurrSize;

  } while (nReadSize == 256*1024);

  fclose(inFile);
  fclose(outFile);
  return 0;
}