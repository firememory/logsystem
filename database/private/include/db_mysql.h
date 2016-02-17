/*

*/

#ifndef DB_MYSQL_H_
#define DB_MYSQL_H_

#include "database.h"

#if defined(PLATFORM_API_WINDOWS)
# include <winsock2.h>
#else
#endif //

#include <mysql.h>

BEGIN_NAMESPACE_DATABASE

//////////////////////////////////////////////////////////////////////////
class MYSQLRecordSet;
//
class MYSQLRecord: public IRecord {
public:
  void Release();

public:
  rc_t get_data(uint32_t idx, uint8_t** val, uint32_t* len);

  rc_t get_data(uint32_t idx, uchar_t* val);
  rc_t get_data(uint32_t idx, uint16_t* val);
  rc_t get_data(uint32_t idx, uint32_t* val);
  rc_t get_data(uint32_t idx, uint64_t* val);

  rc_t get_data(uint32_t idx, float_t* val);
  rc_t get_data(uint32_t idx, double_t* val);

public:
  MYSQLRecord();
  ~MYSQLRecord();

  void set_mysql_row(MYSQL_ROW row, uint32_t filed_count, const unsigned long* lengths);

private:
  MYSQL_ROW m_row;
  uint32_t m_filed_count;
  const unsigned long* m_lengths;

  DISALLOW_COPY_AND_ASSIGN(MYSQLRecord);
}; // MYSQLRecord

//////////////////////////////////////////////////////////////////////////
// RecordSet
class MYSQLRecordSet : public IRecordSet {
public:
  void Release();

public:
  rc_t fetch();
  rc_t close();

  rc_t first();
  rc_t last();

  rc_t next();
  rc_t prev();

public:
  rc_t get_field_count(uint32_t* count);
  rc_t get_field_name(char_t* name, uint32_t idx);
  rc_t get_field_idx(uint32_t* idx, const char_t* name);

  rc_t get_row_count(uint32_t* count);
  rc_t get_row_count(uint64_t* count);

public:
  IRecord* get_record();

public:
  MYSQLRecordSet();
  ~MYSQLRecordSet();

  void set_mysql_res(MYSQL_RES* res);

private:
  MYSQL_RES*  m_res;
  MYSQL_ROW   m_row;
  uint64_t    m_curr_row_no;

  // OPT
  unsigned long*   m_lengths;
  uint64_t    m_row_count;
  uint32_t    m_filed_count;

  MYSQLRecord m_rec;

  DISALLOW_COPY_AND_ASSIGN(MYSQLRecordSet);
}; // MYSQLRecordSet

class MYSQLConnection : public IConnection {
public:
  void Release();

public:
  /* connect string */
  rc_t connect(const char_t*);
  rc_t connect(const void*) { return RC_E_UNSUPPORTED; }
  rc_t connect(const db_conn_str_t*);

  rc_t disconnect();
  rc_t reconnect();
  rc_t check_connect(bool_t bKeepAlive, bool_t bFix);

public:
  rc_t execute(const char_t*, bool_t);
  rc_t commit();
  rc_t rollback();

public:
  const char_t* get_last_error(uint32_t*);

public:
  IRecordSet* get_record_set();

public:
  static MYSQLConnection* CreateInstance(const char_t* strParam = NULL_STR);
  static const char_t* m_gcStrName;

  static rc_t db_lib_init();
  static void db_lib_deinit();
  static rc_t db_thd_init();
  static void db_thd_deinit();

public:
  MYSQLConnection();
  ~MYSQLConnection();

private:
  rc_t clear_all_result_set();
  void reset_error();
  bool_t didConnected();

private:
  rc_t      m_nStatus;

  MYSQL     m_mysql;
  MYSQL_RES* m_myres;

  MYSQLRecordSet  m_RecordSet;

  const char_t*         m_strDB;
  const db_conn_str_t*  m_dbCon; 

  DISALLOW_COPY_AND_ASSIGN(MYSQLConnection);
}; // MYSQLConnection

END_NAMESPACE_DATABASE

#endif // DB_MYSQL_H_
