/*

*/

//#include "db_null.h"
#include "db_mysql.h"

#include "mysql_capi.h"

BEGIN_NAMESPACE_DATABASE
//////////////////////////////////////////////////////////////////////////
//NullConnection  NullConnection::Null_Connection;
//const char_t*   NullConnection::strName = _STR("NULL");

IConnection* IConnection::CreateInstance(const char_t* strType) {

  if (0 == STRCMP(strType, MYSQLConnection::m_gcStrName)) {
    return MYSQLConnection::CreateInstance();
  }
  else {
    //return NullConnection::CreateInstance();
    return NULL;
  }
}

IConnection* Connection_CreateInstance(const char_t* strType) {

  return IConnection::CreateInstance(strType);
}

rc_t db_lib_init() { return MYSQLConnection::db_lib_init(); }

void db_lib_deinit() { MYSQLConnection::db_lib_deinit(); }

rc_t db_thd_init() { return MYSQLConnection::db_thd_init(); }

void db_thd_deinit() { MYSQLConnection::db_thd_deinit(); }

END_NAMESPACE_DATABASE
