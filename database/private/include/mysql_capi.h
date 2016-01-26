/*

*/

#ifndef MYSQL_CAPI_H_
#define MYSQL_CAPI_H_

#include "database.h"

#if defined(PLATFORM_API_WINDOWS)
# include <winsock2.h>
#else
#endif //

#include <mysql.h>

BEGIN_NAMESPACE_DATABASE

//////////////////////////////////////////////////////////////////////////
class IMySQLCAPI {
public:
  virtual my_ulonglong affected_rows(MYSQL *) = 0;

  virtual my_bool autocommit(MYSQL *, my_bool) = 0;

  virtual void close(MYSQL *mysql) = 0;

  virtual my_bool commit(MYSQL *mysql) = 0;

  virtual void data_seek(MYSQL_RES *, my_ulonglong) = 0;

  virtual void debug(const char *) = 0;

  virtual unsigned int mysql_errno(MYSQL *mysql) = 0;

  virtual const char * error(MYSQL *mysql) = 0;

  virtual MYSQL_FIELD * fetch_field(MYSQL_RES *) = 0;

  virtual MYSQL_FIELD * fetch_field_direct (MYSQL_RES *, unsigned int) = 0;

  virtual unsigned long * fetch_lengths(MYSQL_RES * ) = 0;

  virtual MYSQL_ROW fetch_row(MYSQL_RES * ) = 0;

  virtual unsigned int field_count(MYSQL *) = 0;

  virtual void free_result(MYSQL_RES * ) = 0;

  virtual unsigned long get_client_version() = 0;

  virtual const char * get_server_info(MYSQL *) = 0;

  virtual unsigned long get_server_version(MYSQL *) = 0;

  virtual const char * info(MYSQL *mysql) = 0;

  virtual MYSQL * init(MYSQL *mysql) = 0;

  virtual int library_init(int argc, char **argv, char **groups) = 0;

  virtual void library_end() = 0;

  virtual my_bool more_results(MYSQL *) = 0;

  virtual int next_result(MYSQL *) = 0;

  virtual unsigned int num_fields(MYSQL_RES * ) = 0;

  virtual my_ulonglong num_rows(MYSQL_RES * ) = 0;

  virtual int options(MYSQL *, enum mysql_option option , const void *arg) = 0;

  virtual int ping(MYSQL *) = 0;

  virtual int query(MYSQL *, const char *) = 0;

  virtual MYSQL * real_connect(MYSQL * mysql,
    const char *  host,
    const char *  user,
    const char *  passwd,
    const char *  db,
    unsigned int  port,
    const char *  unix_socket,
    unsigned long client_flag) = 0;

  virtual unsigned long real_escape_string(MYSQL * mysql, char * to, const char * from, unsigned long length) = 0;

  virtual int real_query(MYSQL *, const char *, unsigned long) = 0;

  virtual my_bool rollback(MYSQL *) = 0;

  virtual const char * sqlstate(MYSQL *) = 0;

  virtual my_bool ssl_set(MYSQL * mysql,
    const char * key,
    const char * cert,
    const char * ca,
    const char * capath,
    const char * cipher) = 0;

  virtual MYSQL_RES * store_result(MYSQL *) = 0;

  virtual MYSQL_RES * use_result(MYSQL *) = 0;

  virtual unsigned int warning_count(MYSQL *) = 0;

  /* Methods - wrappers of prepared statement stmt_* functions */
  virtual my_ulonglong  stmt_affected_rows (MYSQL_STMT *) = 0;

  virtual my_bool stmt_attr_set(MYSQL_STMT *stmt, enum enum_stmt_attr_type option , const void *arg) = 0;

  virtual my_bool stmt_bind_param(MYSQL_STMT *, MYSQL_BIND *) = 0;

  virtual my_bool stmt_bind_result(MYSQL_STMT *, MYSQL_BIND *) = 0;

  virtual my_bool stmt_close(MYSQL_STMT *) = 0;

  virtual void stmt_data_seek(MYSQL_STMT *, my_ulonglong) = 0;

  virtual unsigned int stmt_errno(MYSQL_STMT *) = 0;

  virtual const char * stmt_error(MYSQL_STMT *) = 0;

  virtual int stmt_execute(MYSQL_STMT *) = 0;

  virtual int stmt_fetch(MYSQL_STMT *) = 0;

  virtual unsigned int stmt_field_count(MYSQL_STMT *) = 0;

  virtual MYSQL_STMT * stmt_init(MYSQL *) = 0;

  virtual my_ulonglong stmt_num_rows(MYSQL_STMT *) = 0;

  virtual unsigned long stmt_param_count(MYSQL_STMT *) = 0;

  virtual int stmt_prepare(MYSQL_STMT *, const char *, unsigned long) = 0;

  virtual MYSQL_RES * stmt_result_metadata(MYSQL_STMT *) = 0;

  virtual my_bool stmt_send_long_data(MYSQL_STMT * stmt , unsigned int par_number, const char * data, unsigned long len) = 0;

  virtual const char *  stmt_sqlstate(MYSQL_STMT *) = 0;

  virtual int stmt_store_result(MYSQL_STMT *) = 0;

  virtual void thread_end() = 0;

  virtual my_bool thread_init() = 0;

  virtual int set_character_set(MYSQL *mysql, const char *csname) = 0;

public:
  static IMySQLCAPI* default_instance();
};


//////////////////////////////////////////////////////////////////////////
// MySQL C APi

typedef my_ulonglong (STDCALL *ptr2mysql_affected_rows)(MYSQL *);

typedef my_bool (STDCALL *ptr2mysql_autocommit)(MYSQL *, my_bool);

typedef void (STDCALL *ptr2mysql_close)(MYSQL *mysql);

typedef my_bool (STDCALL *ptr2mysql_commit)(MYSQL *mysql);

typedef void (STDCALL *ptr2mysql_data_seek)(MYSQL_RES *, my_ulonglong);

typedef void (STDCALL *ptr2mysql_debug)(const char *debug);

typedef unsigned int (STDCALL *ptr2mysql_errno)(MYSQL *mysql);

typedef const char * (STDCALL *ptr2mysql_error)(MYSQL *mysql);

typedef MYSQL_FIELD * (STDCALL *ptr2mysql_fetch_field)(MYSQL_RES *);

typedef MYSQL_FIELD * (STDCALL *ptr2mysql_fetch_field_direct)(MYSQL_RES *, unsigned int);

typedef unsigned long * (STDCALL *ptr2mysql_fetch_lengths)(MYSQL_RES *);

typedef MYSQL_ROW (STDCALL *ptr2mysql_fetch_row)(MYSQL_RES *);

typedef unsigned int (STDCALL *ptr2mysql_field_count)(MYSQL *);

typedef void (STDCALL *ptr2mysql_free_result)(MYSQL_RES *);

typedef unsigned long (STDCALL *ptr2mysql_get_client_version)();

typedef const char * (STDCALL *ptr2mysql_get_server_info)(MYSQL *);

typedef unsigned long (STDCALL *ptr2mysql_get_server_version)(MYSQL *);

typedef char * (STDCALL *ptr2mysql_info)(MYSQL *mysql);

typedef MYSQL * (STDCALL *ptr2mysql_init)(MYSQL *mysql);

typedef int (STDCALL *ptr2mysql_library_init)(int, char **, char **);

typedef void (STDCALL *ptr2mysql_library_end)(void);

typedef my_bool (STDCALL *ptr2mysql_more_results)(MYSQL *);

typedef int (STDCALL *ptr2mysql_next_result)(MYSQL *);

typedef unsigned int (STDCALL *ptr2mysql_num_fields)(MYSQL_RES *);

typedef my_ulonglong (STDCALL *ptr2mysql_num_rows)(MYSQL_RES *);

typedef int (STDCALL *ptr2mysql_options)(MYSQL *, enum mysql_option, const void *);

typedef int (STDCALL *ptr2mysql_query)(MYSQL *, const char *);

typedef int (STDCALL *ptr2mysql_ping)(MYSQL *);

typedef MYSQL * (STDCALL *ptr2mysql_real_connect)(MYSQL *, const char *, const char *, const char * , const char *, unsigned int, const char *, unsigned long);

typedef unsigned long (STDCALL *ptr2mysql_real_escape_string)(MYSQL * mysql, char *, const char *, unsigned long);

typedef int (STDCALL *ptr2mysql_real_query)(MYSQL *, const char *, unsigned long);

typedef my_bool (STDCALL *ptr2mysql_rollback)(MYSQL *mysql);

typedef const char * (STDCALL *ptr2mysql_sqlstate)(MYSQL *mysql);

typedef my_bool (STDCALL *ptr2mysql_ssl_set)(MYSQL *, const char *, const char *, const char *, const char *, const char *);

typedef MYSQL_RES * (STDCALL *ptr2mysql_store_result)(MYSQL *);

typedef MYSQL_RES * (STDCALL *ptr2mysql_use_result)(MYSQL *);

typedef unsigned int (STDCALL *ptr2mysql_warning_count)(MYSQL *);

/* Prepared Statement stmt_* functions */
typedef my_ulonglong (STDCALL *ptr2mysql_stmt_affected_rows)(MYSQL_STMT *);

typedef my_bool (STDCALL *ptr2mysql_stmt_attr_set)(MYSQL_STMT *, enum enum_stmt_attr_type, const void *);

typedef my_bool (STDCALL *ptr2mysql_stmt_bind_param)(MYSQL_STMT *, MYSQL_BIND *);

typedef my_bool (STDCALL *ptr2mysql_stmt_bind_result)(MYSQL_STMT *, MYSQL_BIND *);

typedef my_bool (STDCALL *ptr2mysql_stmt_close)(MYSQL_STMT *);

typedef void (STDCALL *ptr2mysql_stmt_data_seek)(MYSQL_STMT *, my_ulonglong);

typedef unsigned int (STDCALL *ptr2mysql_stmt_errno)(MYSQL_STMT *);

typedef const char * (STDCALL *ptr2mysql_stmt_error)(MYSQL_STMT *);

typedef int (STDCALL *ptr2mysql_stmt_execute)(MYSQL_STMT *);

typedef int (STDCALL *ptr2mysql_stmt_fetch)(MYSQL_STMT *);

typedef unsigned int (STDCALL *ptr2mysql_stmt_field_count)(MYSQL_STMT *);

typedef MYSQL_STMT * (STDCALL *ptr2mysql_stmt_init)(MYSQL *);

typedef my_ulonglong (STDCALL *ptr2mysql_stmt_num_rows)(MYSQL_STMT *);

typedef unsigned long (STDCALL *ptr2mysql_stmt_param_count)(MYSQL_STMT *);

typedef int (STDCALL *ptr2mysql_stmt_prepare)(MYSQL_STMT *, const char *, unsigned long);

typedef MYSQL_RES * (STDCALL *ptr2mysql_stmt_result_metadata)(MYSQL_STMT *);

typedef my_bool (STDCALL *ptr2mysql_stmt_send_long_data)(MYSQL_STMT *, unsigned int, const char *, unsigned long);

typedef const char * (STDCALL *ptr2mysql_stmt_sqlstate)(MYSQL_STMT *);

typedef int (STDCALL *ptr2mysql_stmt_store_result)(MYSQL_STMT *);

typedef my_bool (STDCALL *ptr2mysql_thread_init)();

typedef void (STDCALL *ptr2mysql_thread_end)();

typedef int (STDCALL *ptr2mysql_set_character_set)(MYSQL *mysql, const char *csname);

//////////////////////////////////////////////////////////////////////////
// dynamic mysql proxy.

struct st_mysql_capi_t {

  ptr2mysql_library_init    pfn_mysql_library_init;
  ptr2mysql_library_end     pfn_mysql_library_end;
};


END_NAMESPACE_DATABASE

#endif // MYSQL_CAPI_H_
