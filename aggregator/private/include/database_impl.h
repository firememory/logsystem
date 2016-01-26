/*

*/

#ifndef DATABASE_IMPL_H_
#define DATABASE_IMPL_H_

#include "aggregator.h"
#include "aggregator_export.h"

#include "pool.h"
#include "time_util.h"

#include "database.h"

BEGIN_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////

USING_BASE(PoolThreadSafe);
USING_BASE(TimeControl);

USING_DATABASE(IConnection);
USING_DATABASE(IRecordSet);
USING_DATABASE(Connection_CreateInstance);

// db pool
class DBConnImpl;

typedef PoolThreadSafe<DBConnImpl, const db_conn_str_t*> DBConn_pool_t;

//////////////////////////////////////////////////////////////////////////
class AGGREGATOR_EXPORT DBConnImpl : public IConnection {
  // IConnection
public:
  void Release() { ASSERT(m_pPoolDBConn); m_pPoolDBConn->release(this); }

public:
  /* connect string */
  //rc_t connect(const char_t* strConn) { ASSERT(m_pConnection); return m_pConnection->connect(strConn); }
  rc_t connect(const void* pConn) { ASSERT(m_pConnection); m_tcKeepAlive.UpdateTime(); return m_pConnection->connect(pConn); }
  rc_t connect(const db_conn_str_t* pConn) { ASSERT(m_pConnection); m_tcKeepAlive.UpdateTime(); return m_pConnection->connect(pConn); }

  rc_t disconnect() { ASSERT(m_pConnection); return m_pConnection->disconnect(); }
  rc_t reconnect() { ASSERT(m_pConnection); return m_pConnection->reconnect(); }
  rc_t check_connect(bool_t bKeepAlive, bool_t bFix) { ASSERT(m_pConnection); m_tcKeepAlive.UpdateTime(); return m_pConnection->check_connect(bKeepAlive, bFix); }

public:
  rc_t execute(const char_t* strSQL, bool_t bHavaResult) { ASSERT(m_pConnection); m_tcKeepAlive.UpdateTime(); return m_pConnection->execute(strSQL, bHavaResult);
  }
  rc_t commit() { ASSERT(m_pConnection); return m_pConnection->commit(); }
  rc_t rollback() { ASSERT(m_pConnection); return m_pConnection->rollback(); }

public:
  // return err_msg
  const char_t* get_last_error(uint32_t* nErrNo) { ASSERT(m_pConnection); return m_pConnection->get_last_error(nErrNo); }

public:
  IRecordSet* get_record_set() { ASSERT(m_pConnection); return m_pConnection->get_record_set(); }

public:
  rc_t KeepAlive(uint32_t now) {
    ASSERT(m_pConnection);
    if (m_nLastCheckKeepAlive == now) { return RC_S_STATUS; }
    m_nLastCheckKeepAlive = now;

    if (m_tcKeepAlive.GetIdleTime(now)) { return RC_S_OK; }

    m_pConnection->check_connect(TRUE, TRUE);
    return RC_S_OK;
  }

public:
//   DBConnImpl(void* pool, const char_t* strParam)
//     : m_pConnection(Connection_CreateInstance(strParam))
//     , m_pPoolDBConn(static_cast<DBConn_pool_t*>(pool))
//   {}

  DBConnImpl(void* pool, const db_conn_str_t* pDBConnStr)
    : m_pConnection(Connection_CreateInstance(pDBConnStr->db_driver))
    , m_pPoolDBConn(static_cast<DBConn_pool_t*>(pool))
    , m_tcKeepAlive(pDBConnStr && pDBConnStr->db_keep_alive ? pDBConnStr->db_keep_alive : kKeepDBConnTimeOut)
    , m_nLastCheckKeepAlive(0)
  {
    if (m_pConnection) { m_pConnection->connect(pDBConnStr); }
  }

  ~DBConnImpl() {
    if (m_pConnection) { m_pConnection->Release(); }
  }

  IConnection* ToIConnection() { return this; }

private:
  IConnection*      m_pConnection;
  DBConn_pool_t*    m_pPoolDBConn;

  TimeControl       m_tcKeepAlive;
  uint32_t          m_nLastCheckKeepAlive;

  DISALLOW_COPY_AND_ASSIGN(DBConnImpl);
}; // DBConnImplImpl


END_NAMESPACE_AGGREGATOR

#endif // DATABASE_IMPL_H_
