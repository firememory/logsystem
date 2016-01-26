/*

*/

#ifndef _REGEX_HPP_
#define	_REGEX_HPP_	


#include "regex.h"

#if !defined(HAS_CONFIG)
typedef unsigned int                    uint32_t;
typedef uint32_t                        bool_t;

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

#endif // HAS_CONFIG

#include <stdlib.h>

class RegEx {

public:
  RegEx(uint32_t match_count)
    : m_bCompile(FALSE)
    , m_pmatch(NULL)
    , m_nmc(match_count)
  {
    if (m_nmc) {
      m_pmatch = (regmatch_t*)malloc(sizeof(regmatch_t) * m_nmc);
    }
  }

  RegEx(uint32_t match_count, __REG_CONST char * pattern, int cflags = REG_EXTENDED)
    : m_bCompile(FALSE)
    , m_pmatch(NULL)
    , m_nmc(match_count)
  {

    if (pattern) {

      if(REG_EXTENDED == cflags && 0 == regcomp(&m_regex, pattern, REG_EXTENDED)) { m_bCompile = TRUE; }
      else {
        // try REG_EXTENDED
        if (0 == regcomp(&m_regex, pattern, cflags)) { m_bCompile = TRUE; }
      }
    }

    if (m_nmc) {
      m_pmatch = (regmatch_t*)malloc(sizeof(regmatch_t) * m_nmc);
    }
  }

  ~RegEx(){

    if (TRUE == m_bCompile) { regfree(&m_regex); }
    if (m_pmatch) { free(m_pmatch); }
  }

public:
  inline bool_t didCompile() const { return m_bCompile; }
  inline bool_t Compile(__REG_CONST char * pattern, int cflags = REG_EXTENDED) {

    if (TRUE == m_bCompile) { regfree(&m_regex); m_bCompile = FALSE; }

    if (pattern) {

      if(REG_EXTENDED == cflags && 0 == regcomp(&m_regex, pattern, REG_EXTENDED)) { m_bCompile = TRUE; }
      else {
        // try REG_EXTENDED
        if (0 == regcomp(&m_regex, pattern, cflags)) { m_bCompile = TRUE; }
      }
    }

    return m_bCompile;
  }

  inline uint32_t MatchCount() const { return m_nmc; }
  inline void SetMatchCount(uint32_t match_count) {

    if (match_count > m_nmc) {
      //if (m_pmatch) { free(m_pmatch); }
      m_nmc = match_count;
      m_pmatch = (regmatch_t*)realloc(m_pmatch, sizeof(regmatch_t) * m_nmc);
    }
  }

public:
  inline bool_t isMatchOnly(const char* str, uint32_t cflags = 0) {

    if (FALSE == m_bCompile) { return FALSE; }

    regmatch_t pmatch[1];
    return REG_OKAY != regexec(&m_regex, str, 1, pmatch, cflags | REG_ICASE) ? FALSE : TRUE;
  }

  inline bool_t isMatchOnly(const char* str, uint32_t matchLen, uint32_t cflags = 0) {

    if (FALSE == m_bCompile) { return FALSE; }

    regmatch_t pmatch[1];
    if (REG_OKAY != regexec(&m_regex, str, 1, pmatch, cflags | REG_ICASE)) { return FALSE; }
    return matchLen == (uint32_t)(pmatch->rm_eo - pmatch->rm_so) ? TRUE : FALSE;
  }

  inline bool_t isMatch(const char* str, uint32_t cflags = 0) {

    if (FALSE == m_bCompile || 0 == m_nmc || NULL == m_pmatch) { return FALSE; }

    return REG_OKAY != regexec(&m_regex, str, m_nmc, m_pmatch, cflags | REG_ICASE) ? FALSE : TRUE;
  }

  inline bool_t MatchPos(uint32_t* start, uint32_t* end, uint32_t pos) const {

    if (NULL == start || NULL == end) { return FALSE; }
    if (FALSE == m_bCompile) { return FALSE; }
    if (m_nmc < pos || NULL == m_pmatch || -1 == m_pmatch[pos].rm_so) { return FALSE; }

    (*start) = m_pmatch[pos].rm_so;
    (*end) = m_pmatch[pos].rm_eo;
    return TRUE;
  }

private:

  bool_t      m_bCompile;

  regex_t     m_regex;
  regmatch_t* m_pmatch;
  uint32_t    m_nmc;
}; // RegEx

#endif // _REGEX_HPP_
