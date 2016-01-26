/*

*/

#include "setting_db.h"


#include "protocol.h"

#include "database.h"

#include "time_util.h"
#include "code_util.h"

BEGIN_NAMESPACE_AGGREGATOR

USING_BASE(DayTime);
USING_BASE(ToBase16_WT);

USING_DATABASE(IConnection);
USING_DATABASE(IRecordSet);
USING_DATABASE(IRecord);
//////////////////////////////////////////////////////////////////////////
// resource
const rc_t SettingDB::GetName(char_t* strName, uint32_t* size) const {
  if (NULL == strName || NULL == size || 0 == (*size)) { return RC_S_NULL_VALUE; }
  STRCPY(strName, (*size), m_strName);
  return RC_S_OK;
}

const rc_t SettingDB::GetStatus() const { return RC_S_OK; }

const rc_t SettingDB::GetResourceSize(uint64_t* curr_size, uint64_t* free_size) const {

  if (NULL == curr_size || 0 == free_size) { return RC_S_NULL_VALUE; }

  (*curr_size) = 0x00;
  (*free_size) = 0x00;
  return RC_S_OK;
}

const rc_t SettingDB::GetLastErr(uint32_t* err_no, char_t* err_msg, uint32_t* size) const {

  if (NULL == err_no || NULL == err_msg || NULL == size || 0 == (*size)) { return RC_S_NULL_VALUE; }

  (*err_no) = 0;
  STRCPY(err_msg, (*size), NULL_STR);
  return RC_S_OK;
}

const rc_t SettingDB::GetInfo(char_t* info, uint32_t* size) const {

  if (NULL == info || NULL == size || 0 == (*size)) { return RC_S_NULL_VALUE; }

  (*size) = SNPRINTF(info, (*size), (*size), _STR("%s: \n"
    "Set Count=%u\n"
    "Get Count=%u\n"
    )

    , STR_VERSION
    , m_nCountSet
    , m_nCountGet
    );
  return RC_S_OK;
}

const rc_t SettingDB::GetLastAliveTime(uint32_t* alive) const {

  if (NULL == alive) { return RC_S_NULL_VALUE; }

  (*alive) = m_timeAlive;
  return RC_S_OK;
}

//////////////////////////////////////////////////////////////////////////
// 
rc_t SettingDB::Set(char_t const* strName, const IGetSetParam* pIGetSetParam) {

  if (NULL == strName) { return RC_S_NULL_VALUE; }
  if (NULL == m_pIAggregator) { return RC_S_FAILED; }

  ++m_nCountSet;
  m_timeAlive = DayTime::now();

  if (0 == STRCMP(kStrCollectLog, strName)) { return procSet_Log(pIGetSetParam); }
  else if (0 == STRCMP(kStrUpdateFileInfo, strName)) { return procSet_UpdateFile(pIGetSetParam); }
  else if (0 == STRCMP(kStrSetModuleSetting, strName)) { return procSet_ModuleSetting(pIGetSetParam); }

  else if (0 == STRCMP(kStrCollectorStatus, strName)) { return procSet_CollectorStatus(pIGetSetParam); }

  else if (0 == STRCMP(kStrResourceStatus, strName)) { return procSet_ResourceStatus(pIGetSetParam); }  
  
  else if (0 == STRCMP(kStrSetRPCInvoke, strName)) { return procSet_RPCInvoke(pIGetSetParam); }

  else if (0 == STRCMP(kStrCheckPoint, strName)) { return procSet_CheckPoint(pIGetSetParam); }

  return RC_E_UNSUPPORTED;
}

rc_t SettingDB::Get(IGetSetParam* pIGetSetParam, char_t const* strName) {

  if (NULL == pIGetSetParam || NULL == strName) { return RC_S_NULL_VALUE; }

  if (NULL == m_pIAggregator) { return RC_S_FAILED; }

  ++m_nCountGet;
  m_timeAlive = DayTime::now();

  if (0 == STRCMP(kStrGetRPCInvoke, strName)) { return procGet_RPCInvoke(pIGetSetParam); }

  else if (0 == STRCMP(kStrCollectorConfig, strName)) { return procGet_CollectorConfig(pIGetSetParam); }
  else if (0 == STRCMP(kStrCollectorFile, strName)) { return procGet_CollectorFile(pIGetSetParam); }
  else if (0 == STRCMP(kStrCollectRule, strName)) { return procGet_CollectRule(pIGetSetParam); }

  //else if (0 == STRCMP(kStrGetFileID, strName)) { return procGet_FileID(pIGetSetParam); }
  else if (0 == STRCMP(kStrSystemSetting, strName)) { return procGet_SystemSetting(pIGetSetParam); }
  else if (0 == STRCMP(kStrGetLogTypeRule, strName)) { return procGet_LogTypeRule(pIGetSetParam); }
  else if (0 == STRCMP(kStrGetModuleSetting, strName)) { return procGet_ModuleSetting(pIGetSetParam); }

  else if (0 == STRCMP(kStrCheckSystem, strName)) { return procGet_CheckSystem(pIGetSetParam); }

  else if (0 == STRCMP(kStrGetTC50LogRule, strName)) { return procGet_TC50LogRule(pIGetSetParam); }

  return RC_E_UNSUPPORTED;
}
//////////////////////////////////////////////////////////////////////////
const char_t* SettingDB::m_strName      = _STR("SettingDB");
const uuid SettingDB::m_gcUUID          = 0x00003012;
//////////////////////////////////////////////////////////////////////////
SettingDB::SettingDB()
  : m_pIAggregator(NULL)
  , m_nwtGetDBConnection(1) // 1s

  , m_nCountGet(0)
  , m_nCountSet(0)
  , m_timeAlive(0)
{}

SettingDB::~SettingDB() {
}

rc_t SettingDB::Init(IAggregatorBase* pIAggregator) {

  m_pIAggregator = pIAggregator;

  if (m_pIAggregator) {
    static const char_t strTraceStart[] = _STR("SettingDB is working." STR_VERSION);
    logDB(_STR("TRACE"), m_strName, (uint8_t*)strTraceStart, sizeof(strTraceStart) - 1);
  }

  return RC_S_OK;
}

//////////////////////////////////////////////////////////////////////////
// sql
static const uint32_t kMAX_SQL_LEN      = 32 * 1024;
static const char_t* kStrZeroSize       = _STR("0");
static const char_t* kStrSQLNULL        _STR("NULL");

IDBConnection* SettingDB::getDBConn() {

  ASSERT(m_pIAggregator);
  return m_pIAggregator->GetDBConnection(/*"maindb"*/NULL_STR, m_nwtGetDBConnection);
}

rc_t SettingDB::execSQL(const char_t* strSQL) {

  ASSERT(strSQL);
  if (0 >= STRLEN(strSQL)) { return RC_S_NULL_VALUE; }

  // get dblink
  AutoRelease<IConnection*> autoRelIDBConn((IConnection*)getDBConn());
  if (NULL == autoRelIDBConn) { return RC_S_FAILED; }

  return autoRelIDBConn->execute((const char_t*)strSQL, FALSE);
}

rc_t SettingDB::execSQL(IGetSetParam* pIGetSetParam, const char_t* strSQL) {

  ASSERT(pIGetSetParam);
  ASSERT(strSQL);
  if (0 >= STRLEN(strSQL)) {
    return RC_S_NULL_VALUE; }

  // get dblink
  AutoRelease<IConnection*> autoRelIDBConn((IConnection*)getDBConn());
  if (NULL == autoRelIDBConn) {
    return RC_S_FAILED; }

  if (RC_S_OK != autoRelIDBConn->execute(strSQL, TRUE)) {
    return RC_S_FAILED; }

  AutoRelease<IRecordSet*> autoRelRecSet(autoRelIDBConn->get_record_set());
  if (NULL == autoRelRecSet) { return RC_S_FAILED; }

  uint32_t nRowIdx = 0;
  uint32_t nFieldCount;
  if (RC_S_OK != autoRelRecSet->get_field_count(&nFieldCount)) {
    return RC_S_FAILED; }

  rc_t rc = RC_S_OK;
  while (RC_S_OK == rc && RC_S_OK == autoRelRecSet->fetch()) {

    AutoRelease<IRecord*> autoRelRec(autoRelRecSet->get_record());
    if (NULL == autoRelRec) { goto next_rec; }

    //
    for (uint32_t idxField = 0; idxField < nFieldCount; ++idxField) {

      uint8_t* data;
      uint32_t len;
      if (RC_S_OK != autoRelRec->get_data(idxField, &data, &len)) { break; }
      pIGetSetParam->SetResult(nRowIdx, idxField, (const char_t*)data, len);
    }

next_rec:
    ++nRowIdx;
    rc = autoRelRecSet->next();
  }

  return RC_S_OK;
}

//////////////////////////////////////////////////////////////////////////
// collector
static const char_t* strCollectorConfigFMT = _STR("call sp_collector_get_config('%s', @err_no, @err_msg);select @err_no, @err_msg;");
rc_t SettingDB::procGet_CollectorConfig(IGetSetParam* pIGetSetParam) {

  ASSERT(pIGetSetParam);

  const char_t* strCollector  = pIGetSetParam->GetCondition(0, NULL);
  if (NULL == strCollector) { return RC_S_NULL_VALUE; }

  char_t strSQL[kMAX_SQL_LEN + 1] = {0};
  SNPRINTF(strSQL, kMAX_SQL_LEN, kMAX_SQL_LEN, strCollectorConfigFMT, strCollector);

  return execSQL(pIGetSetParam, strSQL);
}

static const char_t* strCollectorFileFMT = _STR("call sp_collector_get_file('%s', '%s', @err_no, @err_msg);select @err_no, @err_msg;");

rc_t SettingDB::procGet_CollectorFile(IGetSetParam* pIGetSetParam) {

  ASSERT(pIGetSetParam);

  char_t strSQL[kMAX_SQL_LEN + 1] = {0};

  const char_t* strCollector  = pIGetSetParam->GetCondition(0, NULL);
  const char_t* strName       = pIGetSetParam->GetCondition(1, NULL);
  if (NULL == strCollector) { strCollector = NULL_STR; }
  if (NULL == strName) { strName = NULL_STR; }

  SNPRINTF(strSQL, kMAX_SQL_LEN, kMAX_SQL_LEN, strCollectorFileFMT, strCollector, strName);
  return execSQL(pIGetSetParam, strSQL);
}

static const char_t* strCollectorRuleFMT = _STR("call sp_collector_get_rule('%s', @err_no, @err_msg);select @err_no, @err_msg;");
rc_t SettingDB::procGet_CollectRule(IGetSetParam* pIGetSetParam) {

  ASSERT(pIGetSetParam);

  const char_t* strCollector  = pIGetSetParam->GetCondition(0, NULL);
  if (NULL == strCollector) { return RC_S_NULL_VALUE; }

  char_t strSQL[kMAX_SQL_LEN + 1] = {0};
  SNPRINTF(strSQL, kMAX_SQL_LEN, kMAX_SQL_LEN, strCollectorRuleFMT, strCollector);

  return execSQL(pIGetSetParam, strSQL);
}

//////////////////////////////////////////////////////////////////////////
// fileid

static const char_t* strGetFileIDFMT = _STR("call sp_collector_get_fileid('%s', '%s', '%s', '%s', @err_no, @err_msg);select @err_no, @err_msg;");

rc_t SettingDB::procGet_FileID(IGetSetParam* pIGetSetParam) {

  //
  const char_t* strCollector  = pIGetSetParam->GetCondition(0, NULL);
  const char_t* strDir        = pIGetSetParam->GetCondition(1, NULL);
  const char_t* strName       = pIGetSetParam->GetCondition(2, NULL);
  const char_t* strCreateTime = pIGetSetParam->GetCondition(3, NULL);
  if (NULL == strCollector || NULL == strDir || NULL == strName || NULL == strCreateTime) { return RC_S_NULL_VALUE; }
  /*
  char_t strDirHex[kMAX_PATH * 2 + 1] = {0};
  size_t nDirLen = (uint32_t)STRLEN(strDir);
  if (nDirLen > kMAX_PATH) { return RC_E_NOMEM; }
  ToBase16_WT((uint8_t*)strDirHex, (const uint8_t*)strDir, nDirLen);
  */
  char_t strSQL[kMAX_SQL_LEN + 1] = {0};
  SNPRINTF(strSQL, kMAX_SQL_LEN, kMAX_SQL_LEN, strGetFileIDFMT, strCollector, strDir, strName, strCreateTime);

  return execSQL(pIGetSetParam, strSQL);
}

static const char_t* strSetFileInfoFMT = _STR("call sp_collector_set_fileinfo(%u, '%s', '%s', '%s', %s, '%s', %s, @err_no, @err_msg);select @err_no, @err_msg;");

rc_t SettingDB::procSet_UpdateFile(const IGetSetParam* pIGetSetParam) {

  file_id_t file_id = IFileNode::INVALID_FILE_ID;
  const char_t* strCollector  = pIGetSetParam->GetCondition(0, &file_id);
  const char_t* strDir        = pIGetSetParam->GetCondition(1, NULL);
  const char_t* strName       = pIGetSetParam->GetCondition(2, NULL);
  const char_t* strCreateTime = pIGetSetParam->GetCondition(3, NULL);
  const char_t* strPath       = pIGetSetParam->GetCondition(4, NULL);
  const char_t* strFileSize   = pIGetSetParam->GetCondition(5, NULL);

  if (IFileNode::INVALID_FILE_ID == file_id) { return RC_S_NULL_VALUE; }
  if (NULL == strFileSize) {
    if (NULL == strCollector || NULL == strDir || NULL == strName || NULL == strCreateTime || NULL == strPath) {
      return RC_S_NULL_VALUE; }

    strFileSize = kStrZeroSize;
  }
  else {

    strCollector = NULL_STR;
    strDir = NULL_STR;
    strName = NULL_STR;
    strCreateTime = kStrSQLNULL;
    strPath = NULL_STR;
  }

  // size
  char_t strSQL[kMAX_SQL_LEN + 1] = {0};
  SNPRINTF(strSQL, kMAX_SQL_LEN, kMAX_SQL_LEN, strSetFileInfoFMT, file_id
    , strCollector, strDir, strName, strCreateTime, strPath, strFileSize);
  return execSQL(strSQL);
}

//////////////////////////////////////////////////////////////////////////
// system config

static const char_t* strGetSysSettingFMT = _STR("call sp_sys_get_setting(@err_no, @err_msg);select @err_no, @err_msg;");

rc_t SettingDB::procGet_SystemSetting(IGetSetParam* pIGetSetParam) {
  return execSQL(pIGetSetParam, strGetSysSettingFMT);
}

//////////////////////////////////////////////////////////////////////////
// log type rule
static const char_t* strGetLogTypeRuleFMT = _STR("call sp_parser_get_logtype_rule('%s', @err_no, @err_msg);select @err_no, @err_msg;");

rc_t SettingDB::procGet_LogTypeRule(IGetSetParam* pIGetSetParam) {

  const char_t* strLogType   = pIGetSetParam->GetCondition(0, NULL);
  if (NULL == strLogType) { return RC_S_NULL_VALUE; }

  // size
  char_t strSQL[kMAX_SQL_LEN + 1] = {0};
  SNPRINTF(strSQL, kMAX_SQL_LEN, kMAX_SQL_LEN, strGetLogTypeRuleFMT, strLogType);

  return execSQL(pIGetSetParam, strSQL);
}

//////////////////////////////////////////////////////////////////////////
// rpc
static const char_t* strGetRPCInvokeFMT = _STR("call sp_sys_get_rpc(@err_no, @err_msg);select @err_no, @err_msg;");

rc_t SettingDB::procGet_RPCInvoke(IGetSetParam* pIGetSetParam) {
  return execSQL(pIGetSetParam, strGetRPCInvokeFMT);
}

static const char_t* strSetRPCInvokeFMT = _STR("call sp_sys_set_rpc(%s, %s, '%s', x'%s', @err_no, @err_msg);select @err_no, @err_msg;");

rc_t SettingDB::procSet_RPCInvoke(const IGetSetParam* pIGetSetParam) {

  const char_t* strID     = pIGetSetParam->GetCondition(0, NULL);
  const char_t* strErrNo  = pIGetSetParam->GetCondition(1, NULL);
  const char_t* strErrMsg = pIGetSetParam->GetCondition(2, NULL);

  uint32_t nResultLen = 0;
  const char_t* strResult = pIGetSetParam->GetCondition(3, &nResultLen);

  if (NULL == strID) { return RC_S_NULL_VALUE; }

  const uint32_t kMAX_RESULT_LEN  = 8 * 1024;
  char_t strHexResult[kMAX_RESULT_LEN + 1] = {0};
  nResultLen = nResultLen > kMAX_RESULT_LEN / 2 ? kMAX_RESULT_LEN / 2 : kMAX_RESULT_LEN;
  if (strResult && nResultLen) {
    ToBase16_WT((uint8_t *)strHexResult, (const uint8_t*)strResult, nResultLen);
  }

  char_t strSQL[kMAX_SQL_LEN + 1] = {0};
  SNPRINTF(strSQL, kMAX_SQL_LEN, kMAX_SQL_LEN, strSetRPCInvokeFMT, strID, strErrNo, strErrMsg, strHexResult);
  return execSQL(strSQL);
}

//////////////////////////////////////////////////////////////////////////
// check system
static const char_t* strGetCheckSystemFMT = _STR("call sp_sys_checksystem(@err_no, @err_msg);select @err_no, @err_msg;");

rc_t SettingDB::procGet_CheckSystem(IGetSetParam* pIGetSetParam) {
  return execSQL(pIGetSetParam, strGetCheckSystemFMT);
}
// checkpoint

static const char_t* strSetCheckPointFMT = _STR("call sp_sys_set_checkpoint(@err_no, @err_msg);select @err_no, @err_msg;");
rc_t SettingDB::procSet_CheckPoint(const IGetSetParam* pIGetSetParam) {

  UNUSED_PARAM(pIGetSetParam);
  return execSQL(strSetCheckPointFMT);
}

//////////////////////////////////////////////////////////////////////////
// parse file
static const char_t* strGetModuleSettingFMT = _STR("call sp_module_get_setting('%s', '%s', @err_no, @err_msg);select @err_no, @err_msg;");

rc_t SettingDB::procGet_ModuleSetting(IGetSetParam* pIGetSetParam) {

  const char_t* strModule   = pIGetSetParam->GetCondition(0, NULL);
  const char_t* strName     = pIGetSetParam->GetCondition(1, NULL);
  if (NULL == strModule) { return RC_S_NULL_VALUE; }

  if (NULL == strName) { strName = NULL_STR; }

  // size
  char_t strSQL[kMAX_SQL_LEN + 1] = {0};
  SNPRINTF(strSQL, kMAX_SQL_LEN, kMAX_SQL_LEN, strGetModuleSettingFMT, strModule, strName);

  return execSQL(pIGetSetParam, strSQL);
}

static const char_t* strSetModuleSettingFMT = _STR("call sp_module_set_setting('%s', '%s', '%s', '%s', @err_no, @err_msg);select @err_no, @err_msg;");

rc_t SettingDB::procSet_ModuleSetting(const IGetSetParam* pIGetSetParam) {

  const char_t* strModule   = pIGetSetParam->GetCondition(0, NULL);
  const char_t* strName     = pIGetSetParam->GetCondition(1, NULL);
  const char_t* strVal      = pIGetSetParam->GetCondition(2, NULL);
  const char_t* strData     = pIGetSetParam->GetCondition(3, NULL);
  if (NULL == strModule || NULL == strName) { return RC_S_NULL_VALUE; }

  if (NULL == strVal) { strVal = NULL_STR; }
  if (NULL == strData) { strData = NULL_STR; }

  // size
  char_t strSQL[kMAX_SQL_LEN + 1] = {0};
  SNPRINTF(strSQL, kMAX_SQL_LEN, kMAX_SQL_LEN, strSetModuleSettingFMT, strModule, strName, strVal, strData);

  return execSQL(strSQL);
}

//////////////////////////////////////////////////////////////////////////
// status
static const char_t* strSetCollectorStatusFMT = _STR("call sp_collecotr_set_status('%s', %s, %s, %s, %u, %u, @err_no, @err_msg);select @err_no, @err_msg;");

rc_t SettingDB::procSet_CollectorStatus(const IGetSetParam* pIGetSetParam) {

  const char_t* strCollector      = pIGetSetParam->GetCondition(0, NULL);
  const char_t* strRecvFileReport = pIGetSetParam->GetCondition(1, NULL);

  uint32_t nLogonTime = 0;
  uint32_t nLastKeepAlive = 0;
  const char_t* strRecvFile       = pIGetSetParam->GetCondition(2, &nLogonTime);
  const char_t* strRecvFileReal   = pIGetSetParam->GetCondition(3, &nLastKeepAlive);
  
  if (NULL == strCollector) { return RC_S_NULL_VALUE; }

  // size
  char_t strSQL[kMAX_SQL_LEN + 1] = {0};
  SNPRINTF(strSQL, kMAX_SQL_LEN, kMAX_SQL_LEN, strSetCollectorStatusFMT, strCollector
    , strRecvFileReport, strRecvFile, strRecvFileReal, nLogonTime, nLastKeepAlive);

  return execSQL(strSQL);
}

static const char_t* strSetResourceStatusFMT = _STR("call sp_sys_set_resource_status('%s', %s, %s, %u, %u, '%s', '%s', @err_no, @err_msg);select @err_no, @err_msg;");

rc_t SettingDB::procSet_ResourceStatus(const IGetSetParam* pIGetSetParam) {

  const char_t* strName         = pIGetSetParam->GetCondition(0, NULL);

  uint32_t nLastAlive = 0;
  uint32_t nLastErrNO = 0;

  const char_t* strCurrentSize  = pIGetSetParam->GetCondition(1, &nLastAlive);
  const char_t* strFreeSize     = pIGetSetParam->GetCondition(2, &nLastErrNO);

  const char_t* strLastErrMsg   = pIGetSetParam->GetCondition(3, NULL);
  const char_t* strInfo         = pIGetSetParam->GetCondition(4, NULL);

  if (NULL == strName) { return RC_S_NULL_VALUE; }
  if (NULL == strCurrentSize) { strCurrentSize = NULL_STR; }
  if (NULL == strFreeSize) { strFreeSize = NULL_STR; }
  if (NULL == strLastErrMsg) { strLastErrMsg = NULL_STR; }
  if (NULL == strInfo) { strInfo = NULL_STR; }

  // size
  char_t strSQL[kMAX_SQL_LEN + 1] = {0};
  SNPRINTF(strSQL, kMAX_SQL_LEN, kMAX_SQL_LEN, strSetResourceStatusFMT, strName
    , strCurrentSize, strFreeSize
    , nLastAlive
    , nLastErrNO, strLastErrMsg
    , strInfo);

  return execSQL(strSQL);
}

//////////////////////////////////////////////////////////////////////////
// TC50
static const char_t* strGetTc50LogRuleFMT = _STR("call sp_tc50_get_log_rule(@err_no, @err_msg);select @err_no, @err_msg;");

rc_t SettingDB::procGet_TC50LogRule(IGetSetParam* pIGetSetParam) {
  return execSQL(pIGetSetParam, strGetTc50LogRuleFMT);
}

//////////////////////////////////////////////////////////////////////////
// log
rc_t SettingDB::procSet_Log(const IGetSetParam* pIGetSetParam) {

  ASSERT(pIGetSetParam);

  // check
  const char_t* strLV         = pIGetSetParam->GetCondition(0, NULL);
  const char_t* strFrom       = pIGetSetParam->GetCondition(1, NULL);

  uint32_t dlen = 0;
  const char_t* strData       = pIGetSetParam->GetCondition(2, &dlen);
  uint32_t clen = 0;
  const char_t* strContent    = pIGetSetParam->GetCondition(3, &clen);

  if (NULL == strLV || NULL == strFrom || NULL == strData || 0 == dlen) { return RC_S_NULL_VALUE; }

  return logDB(strLV, strFrom, (const uint8_t*)strData, dlen, (const uint8_t*)strContent, clen);
}

#include <malloc.h>
/*
static const char_t* strLogFMT = _STR("call sp_sys_log('%s', '%s', x'%s', x'%s', @err_no, @err_msg);select @err_no, @err_msg;");
*/
static const char_t strLogHead[]        = _STR("call sp_sys_log('%s', '%s', x'");
static const char_t strLogMid[]         = _STR("', x'");
static const char_t strLogTail[]        = _STR("', @err_no, @err_msg);select @err_no, @err_msg;");

static const uint32_t nStrLogHeadLen    = sizeof(strLogHead) - 1;
static const uint32_t nStrLogMidLen     = sizeof(strLogMid) - 1;
static const uint32_t nStrLogTailLen    = sizeof(strLogTail) - 1;

rc_t SettingDB::logDB(const char_t* lv, const char_t* from
                      , const uint8_t* data, uint32_t dlen
                      , const uint8_t* content, uint32_t clen) {

  ASSERT(lv);
  ASSERT(from);
  ASSERT(data);
  ASSERT(dlen);
  //
  //char_t strSQL[kMAX_SQL_LEN + 1] = {0};
  // please drop it
  // hex encode
  uint32_t nSQLSize = 1024 + dlen * 2 + clen * 2;
  char_t* strSQL = (char_t*)alloca(nSQLSize);
  if (NULL == strSQL) { return RC_E_NOMEM; }

  uint32_t nStrLen = SNPRINTF(strSQL, nSQLSize, nSQLSize, strLogHead, lv, from);

  MEMCPY(strSQL + nStrLen,    nSQLSize - nStrLen, data, dlen);
  ToBase16((uint8_t*)strSQL + nStrLen, dlen);
  nStrLen += dlen * 2;

  MEMCPY(strSQL + nStrLen,    nSQLSize - nStrLen, strLogMid, nStrLogMidLen);
  nStrLen += nStrLogMidLen;

  MEMCPY(strSQL + nStrLen,    nSQLSize - nStrLen, content, clen);
  ToBase16((uint8_t*)strSQL + nStrLen, clen);
  nStrLen += clen * 2;

  MEMCPY(strSQL + nStrLen,    nSQLSize - nStrLen, strLogTail, nStrLogTailLen);
  nStrLen += nStrLogTailLen;
  strSQL[nStrLen] = 0;

  return execSQL(strSQL);
}

END_NAMESPACE_AGGREGATOR
