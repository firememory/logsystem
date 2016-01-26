/*

*/

#ifndef SETTING_DB_H_
#define SETTING_DB_H_

//////////////////////////////////////////////////////////////////////////
#include "aggregator_base.h"

BEGIN_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////

class SettingDB : public ISetting {

  // IObject
public:
  const uuid& GetUUID() const { return m_gcUUID; }
//   const char_t* GetName() const { return m_strName; }
//   const rc_t GetStatus() const { return RC_S_OK; }
  const void* ConstCast(const uuid& id) const {
    if (m_gcUUID == id) { return this; }
    else if (kUUID_ISetting == id) { return (ISetting*)(this); }
    return NULL;
  }
  void* Cast(const uuid& id) {
    if (m_gcUUID == id) { return this; }
    else if (kUUID_ISetting == id) { return (ISetting*)(this); }
    return NULL;
  }

  // IResource
public:
  const rc_t GetName(char_t*, uint32_t*) const;
  const rc_t GetStatus() const;

  const rc_t GetResourceSize(uint64_t*, uint64_t*) const;
  const rc_t GetLastErr(uint32_t*, char_t*, uint32_t*) const;
  const rc_t GetInfo(char_t*, uint32_t*) const;
  const rc_t GetLastAliveTime(uint32_t*) const;

  // ISetting
public:  
  void Release() {}

public:
  rc_t Set(char_t const* name, const IGetSetParam*);
  rc_t Get(IGetSetParam*, char_t const* name);

public:
  SettingDB();
  ~SettingDB();

public:
  rc_t Init(IAggregatorBase* pIAggregator);
  static const char_t* GetName() { return m_strName; }

  // database
private:
  inline IDBConnection* getDBConn();
  rc_t execSQL(const char_t* strSQL);
  rc_t execSQL(IGetSetParam*, const char_t* strSQL);

  // 
private:
  // get
  rc_t procGet_CollectorConfig(IGetSetParam*);
  rc_t procGet_CollectorFile(IGetSetParam*);
  rc_t procGet_CollectRule(IGetSetParam*);

  rc_t procGet_FileID(IGetSetParam* pIGetSetParam);
  rc_t procGet_SystemSetting(IGetSetParam* pIGetSetParam);

  rc_t procGet_LogTypeRule(IGetSetParam* pIGetSetParam);

  rc_t procGet_ModuleSetting(IGetSetParam* pIGetSetParam);
  rc_t procSet_ModuleSetting(const IGetSetParam* pIGetSetParam);

  rc_t procSet_CollectorStatus(const IGetSetParam* pIGetSetParam);
  rc_t SettingDB::procSet_ResourceStatus(const IGetSetParam* pIGetSetParam);

  // set
  rc_t procSet_Log(const IGetSetParam*);
  rc_t procSet_UpdateFile(const IGetSetParam* pIGetSetParam);
  
  // rpc
  rc_t procGet_RPCInvoke(IGetSetParam*);
  rc_t procSet_RPCInvoke(const IGetSetParam*);

  rc_t procGet_CheckSystem(IGetSetParam*);
  rc_t procSet_CheckPoint(const IGetSetParam*);

  rc_t procGet_TC50LogRule(IGetSetParam*);

private:
  //
  rc_t logDB(const char_t* lv, const char_t* from
    , const uint8_t* data, uint32_t dlen
    , const uint8_t* content = NULL, uint32_t clen = 0);

private:
  uint32_t              m_nwtGetDBConnection;

  uint32_t              m_nCountGet;
  uint32_t              m_nCountSet;
  uint32_t              m_timeAlive;

private:
  IAggregatorBase*      m_pIAggregator;
  static const char_t*  m_strName;
  static const uuid     m_gcUUID;
  DISALLOW_COPY_AND_ASSIGN(SettingDB);
}; // SettingDB

#define STR_VERSION           "v1.0.0.722"

END_NAMESPACE_AGGREGATOR
#endif // SETTING_DB_H_

