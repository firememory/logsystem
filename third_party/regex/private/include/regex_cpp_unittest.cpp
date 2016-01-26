#include <stdio.h>
#include <string.h>

#include <regex.hpp>

/* 取子串的函数 */
static char* substr(const char*str, unsigned start, unsigned end) {
  unsigned n = end - start;
  static char stbuf[256];
  strncpy(stbuf, str + start, n);
  stbuf[n] = 0;
  return stbuf;
}

/* 主程序 */
static int regex_unittest(int argc, char** argv) {

  char * pattern;
  size_t x, z, lno = 0;
  char ebuf[128], lbuf[256];
  if (argc<2) { return 1; }

  /* 编译正则表达式*/
  pattern = argv[1];
  pattern = "^20[0-9]{6}\\.log$";
  //pattern = "^[a-z]{2}\\.log$";
  //pattern = "ab{0,3}c";

  RegEx regex(3, pattern, REG_EXTENDED);
  //z = regcomp(&reg, pattern, cflags);
  if (TRUE != regex.didCompile()){
    //regerror(z, &reg, ebuf, sizeof(ebuf));
    fprintf(stderr, "%s: pattern '%s' \n",ebuf, pattern);
    return 1;
  }
  /* 逐行处理输入的数据 */
  while(fgets(lbuf, sizeof(lbuf), stdin) && 0 != strcmp("exit\n", lbuf))
  {
    ++lno;
    if ((z = strlen(lbuf)) > 0 && lbuf[z-1] == '\n')
    lbuf[z - 1] = 0;
    /* 对每一行应用正则表达式进行匹配 */
    if (TRUE != regex.isMatch(lbuf, 0)) continue;
//     z = regexec(&reg, lbuf, nmatch, pm, 0);
//     if (z == REG_NOMATCH) continue;
//     else if (z != 0) {
//       regerror(z, &reg, ebuf, sizeof(ebuf));
//       fprintf(stderr, "%s: regcom('%s')\n", ebuf, lbuf);
//       return 2;
//     }
    /* 输出处理结果 */
    for (x = 0; x < regex.MatchCount(); ++ x) {

      uint32_t start, end;
      if (TRUE != regex.MatchPos(&start, &end, x)) break;
      if (!x) printf("%04d: %s\n", lno, lbuf);
      printf(" $%d='%s'\n", x, substr(lbuf, start, end));
    }
  }
  /* 释放正则表达式 */
  //regfree(&reg);
  return 0;
}


void regex_unittest_cpp(int argc, char** argv) {

  regex_unittest(argc, argv);
}

//////////////////////////////////////////////////////////////////////////
void regex_unittest_cpp(int argc, char** argv);

extern "C" void regex_unittest_c(int argc, char** argv);


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

#define ENABLE_DETECT_MEMORY_LEAK
// regex[a-z]*
void main(int argc, char** argv) {

  detect_memory_leaks(true);

  regex_unittest_cpp(argc, argv);
  regex_unittest_c(argc, argv);
}
