/*

*/

#ifndef DB_NULL_H_
#define DB_NULL_H_

#include "database.h"

BEGIN_NAMESPACE_DATABASE
//////////////////////////////////////////////////////////////////////////

class NullConnection : public IConnection {
public:
  void Release() { }

public:
  /* connect string */
  rc_t connect(const char_t*) { return RC_E_UNSUPPORTED; }
  rc_t connect(const void*) { return RC_E_UNSUPPORTED; }

  rc_t disconnect() { return RC_E_UNSUPPORTED; }
  rc_t reconnect() { return RC_E_UNSUPPORTED; }
  rc_t check_connect(bool_t /*bKeepAlive*/, bool_t /*bFix*/) { return RC_E_UNSUPPORTED; }

public:
  rc_t execute(const char_t*, bool_t) { return RC_E_UNSUPPORTED; }
  rc_t commit() { return RC_E_UNSUPPORTED; }
  rc_t rollback() { return RC_E_UNSUPPORTED; }

public:
  // err_msg < 512
  const char_t* get_last_error(uint32_t*) { return NULL_STR; }

public:
  rc_t get_record_set(IRecordSet**) { return RC_E_UNSUPPORTED; }

public:
  static NullConnection* CreateInstance(const char_t* strParam = NULL_STR) {
    UNUSED_PARAM(strParam);
    return &Null_Connection;
  }
  static const char_t* strName;

private:
  static NullConnection   Null_Connection;
}; // NullConnection

END_NAMESPACE_DATABASE

#endif // DB_NULL_H_
