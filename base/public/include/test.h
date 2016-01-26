/*
  test mini define.
*/

#include <config.h>


#if defined(DEBUG_ON)
# define DO_UNITTEST
#else
# undef  DO_UNITTEST
#endif // DEBUG_ON
//#define PLATFORM_API_WINDOWS
#define DO_UNITTEST
#ifdef DO_UNITTEST
//////////////////////////////////////////////////////////////////////////
// add console to trace
#if defined(PLATFORM_API_WINDOWS) // WINDOWS API
#include <Windows.h>
#include <fcntl.h>
#include <io.h>
inline void InitConsoleWindow(void) {

  int hCRT;
  FILE *hf;

  if (FALSE == AllocConsole()) { return; }
  hCRT = _open_osfhandle((intptr_t)GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
  if (-1 == hCRT) { return; }

  hf = _fdopen( hCRT, "w");
  if (NULL == hf) { return; }
  *stdout = *hf;
  setvbuf(stdout, NULL, _IONBF, 0);

  printf("InitConsoleWindow OK!\n\n");
}
#endif


#define ERR_CODE

#define EXPECT_EQ(expected, actual) \
  if (expected != actual) return ERR_CODE;

#define EXPECT_NE(expected, actual) \
  if (expected == actual) return ERR_CODE;

#define EXPECT_LE(expected, actual) \
  if (expected <= actual) return ERR_CODE;

#define EXPECT_LT(expected, actual) \
  if (expected < actual) return ERR_CODE;

#define EXPECT_GE(expected, actual) \
  if (expected >= actual) return ERR_CODE;

#define EXPECT_GT(expected, actual) \
  if (expected > actual) return ERR_CODE;


#define ASSERT_EQ(expected, actual) \
  ASSERT(expected == actual);

#define ASSERT_NE(expected, actual) \
  ASSERT(expected != actual);

#define ASSERT_LE(expected, actual) \
  ASSERT(expected <= actual);

#define ASSERT_LT(expected, actual) \
  ASSERT(expected < actual);

#define ASSERT_GE(expected, actual) \
  ASSERT(expected >= actual);

#define ASSERT_GT(expected, actual) \
  ASSERT(expected > actual);


#define TEST_MAIN_MOCK_BEGIN(test_fixture, test_name) \
  void test_fixture ## _ ## test_name ## () {\
    detect_memory_leaks(true);\
    InitConsoleWindow(); \
    memory_leak_unittest(); \


#define TEST_MAIN_MOCK_END                  }

#define TEST_BEGIN(test_fixture, test_name) \
  void test_fixture ## _ ## test_name ## () {

#define TEST_END                            }

#define TEST_DEFINE(test_fixture, test_name) \
  void test_fixture ## _ ## test_name ## ()

#define TEST_EXEC(test_fixture, test_name) \
  test_fixture ## _ ## test_name ## ()


inline void memory_leak_unittest() {

  void* ptr = malloc(12);
  UNUSED_LOCAL_VARIABLE(ptr);
}

#endif // DO_UNITTEST
