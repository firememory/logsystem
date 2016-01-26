/*

*/
#include "test.h"

#ifdef DO_UNITTEST

#include "db_mysql.h"

USING_NAMESPACE_DATABASE


TEST_BEGIN(database, mysql) {

  MYSQLConnection* pConnection = MYSQLConnection::CreateInstance();
  ASSERT_NE(NULL, pConnection);

  const char_t* strConn = _STR("<Server Driver=\"MYSQLCI\" Source=\"192.168.2.205,3306\" DBName=\"tdx_hk\" DBUsr=\"root\" DBPwd=\"\" DBFlag=\"196608\" BaseConn=\"20\" MaxConn=\"1000\" >");
  ASSERT_EQ(RC_S_OK, pConnection->connect(strConn));
  ASSERT_EQ(RC_S_OK, pConnection->execute("select 1;", FALSE));

  pConnection->Release();

} TEST_END



#endif // DO_UNITTEST