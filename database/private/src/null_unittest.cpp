/*

*/
#include "test.h"

#ifdef DO_UNITTEST

#include "db_null.h"

USING_NAMESPACE_DATABASE

TEST_BEGIN(database, null) {
/*
  NullConnection* pConnection = NullConnection::CreateInstance();
  ASSERT_NE(NULL, pConnection);

  ASSERT_EQ(RC_E_UNSUPPORTED,
    pConnection->connect("select 1;") {
    );

  ASSERT_EQ(RC_E_UNSUPPORTED,
    pConnection->execute("select 1;", TRUE) {
    );

  pConnection->Release();
*/
} TEST_END



#endif // DO_UNITTEST