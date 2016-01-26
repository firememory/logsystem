/*

*/

#ifndef DATABASE_H_
#define DATABASE_H_


//////////////////////////////////////////////////////////////////////////
#include <config.h>
#include "error.h"

#include "database_export.h"

//////////////////////////////////////////////////////////////////////////

#if HAS_NAMESPACE
# define NAMESPACE_DATABASE_NAME        database
# define BEGIN_NAMESPACE_DATABASE       BEGIN_NAMESPACE namespace NAMESPACE_DATABASE_NAME {
# define END_NAMESPACE_DATABASE         } END_NAMESPACE
# define USING_NAMESPACE_DATABASE       using namespace NAMESPACE_NAME::NAMESPACE_DATABASE_NAME
# define USING_DATABASE(x)              using NAMESPACE_DATABASE_NAME::x
#else
# define BEGIN_NAMESPACE_DATABASE
# define END_NAMESPACE_DATABASE
# define USING_NAMESPACE_DATABASE
# define USING_DATABASE(x)
#endif


#include "db_utility.h"

//////////////////////////////////////////////////////////////////////////
BEGIN_NAMESPACE_DATABASE

interface IRecord {
public:
  virtual void Release() = 0;

public:
  virtual rc_t get_data(uint32_t idx, uint8_t** val, uint32_t* len) = 0;

  virtual rc_t get_data(uint32_t idx, uchar_t* val) = 0;
  virtual rc_t get_data(uint32_t idx, uint16_t* val) = 0;
  virtual rc_t get_data(uint32_t idx, uint32_t* val) = 0;
  virtual rc_t get_data(uint32_t idx, uint64_t* val) = 0;

  virtual rc_t get_data(uint32_t idx, float_t* val) = 0;
  virtual rc_t get_data(uint32_t idx, double_t* val) = 0;
}; // IRecord

//////////////////////////////////////////////////////////////////////////
// RecordSet
interface IRecordSet {
public:
  virtual void Release() = 0;

public:
  virtual rc_t fetch() = 0;
  virtual rc_t close() = 0;

  virtual rc_t first() = 0;
  virtual rc_t last() = 0;

  virtual rc_t next() = 0;
  virtual rc_t prev() = 0;

public:
  virtual rc_t get_field_count(uint32_t* count) = 0;
  virtual rc_t get_field_name(char_t* name, uint32_t idx) = 0;
  virtual rc_t get_field_idx(uint32_t* idx, const char_t* name) = 0;

  virtual rc_t get_row_count(uint32_t* count) = 0;
  virtual rc_t get_row_count(uint64_t* count) = 0;

public:
  virtual IRecord* get_record() = 0;
}; // IRecordSet

//////////////////////////////////////////////////////////////////////////
// Connection
interface IConnection {
public:
  virtual void Release() = 0;

public:
  /* connect string */
  //virtual rc_t connect(const char_t*) = 0;
  virtual rc_t connect(const void*) = 0;
  virtual rc_t connect(const db_conn_str_t*) = 0;

  virtual rc_t disconnect() = 0;
  virtual rc_t reconnect() = 0;
  virtual rc_t check_connect(bool_t bKeepAlive, bool_t bFix) = 0;

public:
  virtual rc_t execute(const char_t* strSQL, bool_t bHavaResult) = 0;
  virtual rc_t commit() = 0;
  virtual rc_t rollback() = 0;

  // stmt
public:

public:
  // return err_msg
  virtual const char_t* get_last_error(uint32_t*) = 0;

public:
  virtual IRecordSet* get_record_set() = 0;

public:
  static IConnection* CreateInstance(const char_t*);
}; // IConnection

//////////////////////////////////////////////////////////////////////////
EXTERN_C DATABASE_EXPORT
IConnection* Connection_CreateInstance(const char_t*);

EXTERN_C DATABASE_EXPORT rc_t db_lib_init();
EXTERN_C DATABASE_EXPORT void db_lib_deinit();

EXTERN_C DATABASE_EXPORT rc_t db_thd_init();
EXTERN_C DATABASE_EXPORT void db_thd_deinit();

class AutoInitDBThd {
public:
  AutoInitDBThd() { db_thd_init(); }
  ~AutoInitDBThd() { db_thd_deinit(); }
}; // AutoInitDBThd

END_NAMESPACE_DATABASE

#endif // DATABASE_H_
