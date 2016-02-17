/*

*/

#include "mysql_capi.h"
#include "db_mysql.h"

#include "dynamic_library.h"

BEGIN_NAMESPACE_DATABASE

USING_BASE(DynamicLibrary);

//////////////////////////////////////////////////////////////////////////

class MySQLCAPI : public IMySQLCAPI {

public:
#define MYSQL_CALL(x)     pfn_mysql_ ## x
  my_ulonglong affected_rows(MYSQL* mysql) { return MYSQL_CALL(affected_rows)(mysql); }

  my_bool autocommit(MYSQL* mysql, my_bool auto_mode) { return MYSQL_CALL(autocommit)(mysql, auto_mode); }

  void close(MYSQL* mysql) { MYSQL_CALL(close)(mysql); }

  my_bool commit(MYSQL* mysql) { return MYSQL_CALL(commit)(mysql); }

  void data_seek(MYSQL_RES* result, my_ulonglong offset) { MYSQL_CALL(data_seek)(result, offset); }

  void debug(const char *debug) { MYSQL_CALL(debug)(debug); }

  unsigned int mysql_errno(MYSQL* mysql) { return (pfn_mysql_errno)(mysql); }

  const char * error(MYSQL* mysql) { return MYSQL_CALL(error)(mysql); }

  MYSQL_FIELD* fetch_field(MYSQL_RES* result) { return MYSQL_CALL(fetch_field)(result); }

  MYSQL_FIELD* fetch_field_direct(MYSQL_RES* result, unsigned int fieldnr) { return MYSQL_CALL(fetch_field_direct)(result, fieldnr); }

  unsigned long * fetch_lengths(MYSQL_RES* result) { return MYSQL_CALL(fetch_lengths)(result); }

  MYSQL_ROW fetch_row(MYSQL_RES* result) { return MYSQL_CALL(fetch_row)(result); }

  unsigned int field_count(MYSQL* mysql) { return MYSQL_CALL(field_count)(mysql); }

  void free_result(MYSQL_RES* result) { MYSQL_CALL(free_result)(result); }

  unsigned long get_client_version() { return MYSQL_CALL(get_client_version)(); }

  const char * get_server_info(MYSQL* mysql) { return MYSQL_CALL(get_server_info)(mysql); }

  unsigned long get_server_version(MYSQL* mysql) { return MYSQL_CALL(get_server_version)(mysql); }

  const char * info(MYSQL* mysql) { return MYSQL_CALL(info)(mysql); }

  MYSQL * init(MYSQL* mysql) { return MYSQL_CALL(init)(mysql); }

  int library_init(int argc, char **argv, char **groups) { return MYSQL_CALL(library_init)(argc, argv, groups); }

  void library_end() { MYSQL_CALL(library_end)(); }

  my_bool more_results(MYSQL* mysql) { return MYSQL_CALL(more_results)(mysql); }

  int next_result(MYSQL* mysql) { return MYSQL_CALL(next_result)(mysql); }

  unsigned int num_fields(MYSQL_RES* result) { return MYSQL_CALL(num_fields)(result); }

  my_ulonglong num_rows(MYSQL_RES* result) { return MYSQL_CALL(num_rows)(result); }

  int options(MYSQL* mysql, enum mysql_option option , const void *arg) { return MYSQL_CALL(options)(mysql, option, arg); }

  int ping(MYSQL* mysql) { return MYSQL_CALL(ping)(mysql); }

  int query(MYSQL* mysql, const char *q) { return MYSQL_CALL(query)(mysql, q); }

  MYSQL * real_connect(MYSQL* mysql,
    const char *  host,
    const char *  user,
    const char *  passwd,
    const char *  db,
    unsigned int  port,
    const char *  unix_socket,
    unsigned long client_flag) { return MYSQL_CALL(real_connect)(mysql, host, user, passwd, db, port, unix_socket, client_flag); }

  unsigned long real_escape_string(MYSQL* mysql, char * to, const char * from, unsigned long length) { return MYSQL_CALL(real_escape_string)(mysql, to, from, length); }

  int real_query(MYSQL* mysql, const char * q, unsigned long length) { return MYSQL_CALL(real_query)(mysql, q, length); }

  my_bool rollback(MYSQL* mysql) { return MYSQL_CALL(rollback)(mysql); }

  const char * sqlstate(MYSQL* mysql) { return MYSQL_CALL(sqlstate)(mysql); }

  my_bool ssl_set(MYSQL* mysql,
    const char * key,
    const char * cert,
    const char * ca,
    const char * capath,
    const char * cipher) { return MYSQL_CALL(ssl_set)(mysql, key, cert, ca, capath, cipher); }

  MYSQL_RES* store_result(MYSQL* mysql) { return MYSQL_CALL(store_result)(mysql); }

  MYSQL_RES* use_result(MYSQL* mysql) { return MYSQL_CALL(use_result)(mysql); }

  unsigned int warning_count(MYSQL* mysql) { return MYSQL_CALL(warning_count)(mysql); }

  /* Methods - wrappers of prepared statement stmt_* functions */
  my_ulonglong  stmt_affected_rows(MYSQL_STMT* stmt) { return MYSQL_CALL(stmt_affected_rows)(stmt); }

  my_bool stmt_attr_set(MYSQL_STMT* stmt, enum enum_stmt_attr_type option , const void *arg) { return MYSQL_CALL(stmt_attr_set)(stmt, option, arg); }

  my_bool stmt_bind_param(MYSQL_STMT* stmt, MYSQL_BIND* bnd) { return MYSQL_CALL(stmt_bind_param)(stmt, bnd); }

  my_bool stmt_bind_result(MYSQL_STMT* stmt, MYSQL_BIND* bnd) { return MYSQL_CALL(stmt_bind_result)(stmt, bnd); }

  my_bool stmt_close(MYSQL_STMT* stmt) { return MYSQL_CALL(stmt_close)(stmt); }

  void stmt_data_seek(MYSQL_STMT* stmt, my_ulonglong offset) { MYSQL_CALL(stmt_data_seek)(stmt, offset); }

  unsigned int stmt_errno(MYSQL_STMT* stmt) { return MYSQL_CALL(stmt_errno)(stmt); }

  const char * stmt_error(MYSQL_STMT* stmt) { return MYSQL_CALL(stmt_error)(stmt); }

  int stmt_execute(MYSQL_STMT* stmt) { return MYSQL_CALL(stmt_execute)(stmt); }

  int stmt_fetch(MYSQL_STMT* stmt) { return MYSQL_CALL(stmt_fetch)(stmt); }

  unsigned int stmt_field_count(MYSQL_STMT* stmt) { return MYSQL_CALL(stmt_field_count)(stmt); }

  MYSQL_STMT* stmt_init(MYSQL* mysql) { return MYSQL_CALL(stmt_init)(mysql); }

  my_ulonglong stmt_num_rows(MYSQL_STMT* stmt) { return MYSQL_CALL(stmt_num_rows)(stmt); }

  unsigned long stmt_param_count(MYSQL_STMT* stmt) { return MYSQL_CALL(stmt_param_count)(stmt); }

  int stmt_prepare(MYSQL_STMT* stmt, const char * q, unsigned long length) { return MYSQL_CALL(stmt_prepare)(stmt, q, length); }

  MYSQL_RES* stmt_result_metadata(MYSQL_STMT* stmt) { return MYSQL_CALL(stmt_result_metadata)(stmt); }

  my_bool stmt_send_long_data(MYSQL_STMT* stmt, unsigned int par_number, const char * data, unsigned long len) { return MYSQL_CALL(stmt_send_long_data)(stmt, par_number, data, len); }

  const char *  stmt_sqlstate(MYSQL_STMT* stmt) { return MYSQL_CALL(stmt_sqlstate)(stmt); }

  int stmt_store_result(MYSQL_STMT* stmt) { return MYSQL_CALL(stmt_store_result)(stmt); }

  void thread_end() { MYSQL_CALL(thread_end)(); }

  my_bool thread_init() { return MYSQL_CALL(thread_init)(); }

  int set_character_set(MYSQL* mysql, const char *csname) { return MYSQL_CALL(set_character_set)(mysql, csname); }

public:
#define DEFINE_PFN(x)   ptr2 ## x   pfn_ ## x;

  DEFINE_PFN(mysql_affected_rows);
  DEFINE_PFN(mysql_autocommit);
  DEFINE_PFN(mysql_close);
  DEFINE_PFN(mysql_commit);
  DEFINE_PFN(mysql_data_seek);
  DEFINE_PFN(mysql_debug);
  DEFINE_PFN(mysql_errno);
  DEFINE_PFN(mysql_error);
  DEFINE_PFN(mysql_fetch_field);
  DEFINE_PFN(mysql_fetch_field_direct);
  DEFINE_PFN(mysql_fetch_lengths);
  DEFINE_PFN(mysql_fetch_row);
  DEFINE_PFN(mysql_field_count);
  DEFINE_PFN(mysql_free_result);
  DEFINE_PFN(mysql_get_client_version);
  DEFINE_PFN(mysql_get_server_info);
  DEFINE_PFN(mysql_get_server_version);
  DEFINE_PFN(mysql_info);
  DEFINE_PFN(mysql_init);
  DEFINE_PFN(mysql_library_init);
  DEFINE_PFN(mysql_library_end);
  DEFINE_PFN(mysql_more_results);
  DEFINE_PFN(mysql_next_result);
  DEFINE_PFN(mysql_num_fields);
  DEFINE_PFN(mysql_num_rows);
  DEFINE_PFN(mysql_options);
  DEFINE_PFN(mysql_query);
  DEFINE_PFN(mysql_ping);
  DEFINE_PFN(mysql_real_connect);
  DEFINE_PFN(mysql_real_escape_string);
  DEFINE_PFN(mysql_real_query);
  DEFINE_PFN(mysql_rollback);
  DEFINE_PFN(mysql_sqlstate);
  DEFINE_PFN(mysql_ssl_set);
  DEFINE_PFN(mysql_store_result);
  DEFINE_PFN(mysql_use_result);
  DEFINE_PFN(mysql_warning_count);

  DEFINE_PFN(mysql_stmt_affected_rows);
  DEFINE_PFN(mysql_stmt_attr_set);
  DEFINE_PFN(mysql_stmt_bind_param);
  DEFINE_PFN(mysql_stmt_bind_result);
  DEFINE_PFN(mysql_stmt_close);
  DEFINE_PFN(mysql_stmt_data_seek);
  DEFINE_PFN(mysql_stmt_errno);
  DEFINE_PFN(mysql_stmt_error);
  DEFINE_PFN(mysql_stmt_execute);
  DEFINE_PFN(mysql_stmt_fetch);
  DEFINE_PFN(mysql_stmt_field_count);
  DEFINE_PFN(mysql_stmt_init);
  DEFINE_PFN(mysql_stmt_num_rows);
  DEFINE_PFN(mysql_stmt_param_count);
  DEFINE_PFN(mysql_stmt_prepare);
  DEFINE_PFN(mysql_stmt_result_metadata);
  DEFINE_PFN(mysql_stmt_send_long_data);
  DEFINE_PFN(mysql_stmt_sqlstate);
  DEFINE_PFN(mysql_stmt_store_result);
  DEFINE_PFN(mysql_thread_init);
  DEFINE_PFN(mysql_thread_end);
  DEFINE_PFN(mysql_set_character_set);
};


//////////////////////////////////////////////////////////////////////////
namespace {

  MySQLCAPI   gMySQLCAPI;

#if defined(PLATFORM_API_WINDOWS) // WINDOWS API
# define MYSQL_LIB_NAME       _STR("libmysql.dll")
#else
# define MYSQL_LIB_NAME       _STR("libmysql.so")
#endif

  struct DataBastLibrary {

    DataBastLibrary()
      : m_dync_lib(MYSQL_LIB_NAME)
    {

      if (FALSE == m_dync_lib.isValid()) { return; }
      
#define GET_PROC_ADDR(x)    gMySQLCAPI.pfn_ ## x = m_dync_lib.LocateSymbol<ptr2 ## x>(#x);
  
      GET_PROC_ADDR(mysql_affected_rows);
      GET_PROC_ADDR(mysql_autocommit);
      GET_PROC_ADDR(mysql_close);
      GET_PROC_ADDR(mysql_commit);
      GET_PROC_ADDR(mysql_data_seek);
      GET_PROC_ADDR(mysql_debug);
      GET_PROC_ADDR(mysql_errno);
      GET_PROC_ADDR(mysql_error);
      GET_PROC_ADDR(mysql_fetch_field);
      GET_PROC_ADDR(mysql_fetch_field_direct);
      GET_PROC_ADDR(mysql_fetch_lengths);
      GET_PROC_ADDR(mysql_fetch_row);
      GET_PROC_ADDR(mysql_field_count);
      GET_PROC_ADDR(mysql_free_result);
      GET_PROC_ADDR(mysql_get_client_version);
      GET_PROC_ADDR(mysql_get_server_info);
      GET_PROC_ADDR(mysql_get_server_version);
      GET_PROC_ADDR(mysql_info);
      GET_PROC_ADDR(mysql_init);
      GET_PROC_ADDR(mysql_library_init);
      GET_PROC_ADDR(mysql_library_end);
      GET_PROC_ADDR(mysql_more_results);
      GET_PROC_ADDR(mysql_next_result);
      GET_PROC_ADDR(mysql_num_fields);
      GET_PROC_ADDR(mysql_num_rows);
      GET_PROC_ADDR(mysql_options);
      GET_PROC_ADDR(mysql_query);
      GET_PROC_ADDR(mysql_ping);
      GET_PROC_ADDR(mysql_real_connect);
      GET_PROC_ADDR(mysql_real_escape_string);
      GET_PROC_ADDR(mysql_real_query);
      GET_PROC_ADDR(mysql_rollback);
      GET_PROC_ADDR(mysql_sqlstate);
      GET_PROC_ADDR(mysql_ssl_set);
      GET_PROC_ADDR(mysql_store_result);
      GET_PROC_ADDR(mysql_use_result);
      GET_PROC_ADDR(mysql_warning_count);

      GET_PROC_ADDR(mysql_stmt_affected_rows);
      GET_PROC_ADDR(mysql_stmt_attr_set);
      GET_PROC_ADDR(mysql_stmt_bind_param);
      GET_PROC_ADDR(mysql_stmt_bind_result);
      GET_PROC_ADDR(mysql_stmt_close);
      GET_PROC_ADDR(mysql_stmt_data_seek);
      GET_PROC_ADDR(mysql_stmt_errno);
      GET_PROC_ADDR(mysql_stmt_error);
      GET_PROC_ADDR(mysql_stmt_execute);
      GET_PROC_ADDR(mysql_stmt_fetch);
      GET_PROC_ADDR(mysql_stmt_field_count);
      GET_PROC_ADDR(mysql_stmt_init);
      GET_PROC_ADDR(mysql_stmt_num_rows);
      GET_PROC_ADDR(mysql_stmt_param_count);
      GET_PROC_ADDR(mysql_stmt_prepare);
      GET_PROC_ADDR(mysql_stmt_result_metadata);
      GET_PROC_ADDR(mysql_stmt_send_long_data);
      GET_PROC_ADDR(mysql_stmt_sqlstate);
      GET_PROC_ADDR(mysql_stmt_store_result);
      GET_PROC_ADDR(mysql_thread_init);
      GET_PROC_ADDR(mysql_thread_end);
      GET_PROC_ADDR(mysql_set_character_set);

      //
      gMySQLCAPI.pfn_mysql_library_init = m_dync_lib.LocateSymbol<ptr2mysql_library_init>("mysql_server_init");
      gMySQLCAPI.pfn_mysql_library_end = m_dync_lib.LocateSymbol<ptr2mysql_library_end>("mysql_server_end");

      MYSQLConnection::db_lib_init();
    }

    ~DataBastLibrary() {
      if (FALSE == m_dync_lib.isValid()) { return; }
      MYSQLConnection::db_lib_deinit();
    }

    bool_t IsValid() { return m_dync_lib.isValid(); }

    // 
    DynamicLibrary m_dync_lib;

  } database_libraray;
}

//////////////////////////////////////////////////////////////////////////
IMySQLCAPI* IMySQLCAPI::default_instance() { return TRUE == database_libraray.IsValid() ? &gMySQLCAPI : NULL; }

END_NAMESPACE_DATABASE
