/*

*/
#include "aggregator_impl.h"

#include "protocol.h"

#include "file_util.h"
#include "math_util.h"
#include "memory_pool.h"
#include "xml.h"

#include "getset_param.h"

BEGIN_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////
USING_BASE(OpenFile);
USING_BASE(ReadFile);
USING_BASE(strAttrOpenRead);

USING_BASE(PlatformFileInfo);
USING_BASE(GetPlatformFileInfo);
USING_BASE(Time);
USING_BASE(gcd);
USING(AutoReleaseMemoryBase);
//////////////////////////////////////////////////////////////////////////

rc_t AggregatorImpl::RegisterFunc(IObject* pObject, const char_t* strModuleType) {

  if (NULL == pObject || NULL == strModuleType) { return RC_S_NULL_VALUE; }

  if (0 == STRCMP(strModuleType, strAttrTypeSetting)) {
    m_lstSettingObject.push_back(pObject);
  }
  else if (0 == STRCMP(strModuleType, strAttrTypeParser)) {
    m_lstParseObject.push_back(pObject);
  }
  else if (0 == STRCMP(strModuleType, strAttrTypeLogType)) {
    m_lstLogTypeObject.push_back(pObject);
  }
  else { return RC_S_NOTFOUND; }

  return RC_S_OK;
}

rc_t AggregatorImpl::UnRegisterFunc(IObject* pObject, const char_t* strModuleType) {

  if (NULL == pObject || NULL == strModuleType) { return RC_S_NULL_VALUE; }

  return RC_E_UNSUPPORTED;
}

static const uint32_t nReTryCount       = 5;

IFileIterator* AggregatorImpl::GetFileIterator(const char_t* strType, uint32_t nWaitTime) {

  if (NULL == m_autoRelIFileSystem) { return NULL; }

  FileIteratorImpl* pFileIteratorImpl = m_poolFileIterator.get();
  /*
  if (NULL == pFileIteratorImpl) { return NULL; }

  pFileIteratorImpl->SetType(strType);
  return pFileIteratorImpl->ToIFileIterator();
  */

  if (NULL == pFileIteratorImpl && nWaitTime) {
    Time start = micro_time::to_millisecond(micro_time::time());
    Time now;
    nWaitTime *= 1000;
    do 
    {
      Thread::sleep(nWaitTime / nReTryCount);

      pFileIteratorImpl = m_poolFileIterator.get();

      now = micro_time::to_millisecond(micro_time::time());
    } while(NULL == pFileIteratorImpl && now - start < nWaitTime);
  }

  if (NULL == pFileIteratorImpl) { return NULL; }

  m_timeAlive = DayTime::now();

  pFileIteratorImpl->SetType(strType);
  return pFileIteratorImpl->ToIFileIterator();
}

IDBConnection* AggregatorImpl::GetDBConnection(const char_t* strType, uint32_t nWaitTime) {

  if (NULL == strType) { return NULL; }

  dbconn_pool_map_t::iterator it_map;
  if (NULL_STR == strType || 0 == STRCMP(NULL_STR, strType)) {
    it_map = m_mapDBConnPool.begin();
  }
  else {
    it_map = m_mapDBConnPool.find(strType);
  }

  //dbconn_pool_map_t::iterator it_map = m_mapDBConnPool.find(strType);
  
  if (m_mapDBConnPool.end() == it_map || NULL == it_map->second) { return NULL; }

  DBConn_pool_t* pDBConnPool = it_map->second;
  ASSERT(pDBConnPool);

  DBConnImpl* pDBConnImpl = pDBConnPool->get();
  /*
  if (NULL == pDBConnImpl) { return NULL; }
  return pDBConnImpl->ToIConnection();
  */

  if (NULL == pDBConnImpl && nWaitTime) {
    Time start = micro_time::to_millisecond(micro_time::time());
    Time now;
    nWaitTime *= 1000;
    do 
    {
      Thread::sleep(nWaitTime / nReTryCount);

      pDBConnImpl = pDBConnPool->get();

      now = micro_time::to_millisecond(micro_time::time());
    } while(NULL == pDBConnImpl && now - start < nWaitTime);
  }

  // return pDBConnImpl ? pDBConnImpl->ToIConnection() : NULL;

  if (NULL == pDBConnImpl) {
    ++m_nCountFailedGetDBConn;
    return NULL;
  }
  
  m_timeAlive = DayTime::now();
  return pDBConnImpl->ToIConnection();
}

ILogWriter* AggregatorImpl::GetLogWriter(uint32_t nWaitTime) {

  UNUSED_PARAM(nWaitTime);
  return NULL;
}

const char_t* AggregatorImpl::GetSystemFullPath() {

  return m_strAggregatorDir;
}

//////////////////////////////////////////////////////////////////////////
typedef GetSetParamImpl<file_id_t*>     FileGSObject;
template<>
void FileGSObject::SetResult(uint32_t row, uint32_t col, const char_t* val, uint32_t len) {

  if (row || col || NULL == val || 0 == len) { return; }
  ASSERT(m_r);
  (*m_r) = (file_id_t)(ATOI(val));
}

rc_t AggregatorImpl::AddFile(const file_id_t& file_id, const char_t* collector
                             , const char_t* strDir, const char_t* strName, uint64_t create_time) {

  if (NULL == m_autoRelIFileSystem) { return RC_S_NULL_VALUE; }

  const IFileNode* pIFileNode;
  {
    AutoLock autoLock(m_lockFileSystem);
    pIFileNode = m_autoRelIFileSystem->AddFile(file_id, collector, strDir, strName, create_time);
    if (NULL == pIFileNode) { return RC_S_OK; }
  }

  // wait up file iterator
  setLogTypeFile(file_id);
  notifyFileIterator(file_id);

  // update file setting
  char_t strCreateTime[32] = {0};
  I64TOA(create_time, strCreateTime, sizeof(strCreateTime), 10);

  FileGSObject fileGSObject(NULL, collector, file_id
    , strDir, 0, strName, 0, strCreateTime, 0, pIFileNode->LocalPath(), 0, NULL, 0);
  rc_t rc = SetSetting(kStrUpdateFileInfo, &fileGSObject);
  if (RC_S_OK != rc) { return rc; }

  m_timeAlive = DayTime::now();
  return RC_S_OK;
}

rc_t AggregatorImpl::AddFile(const file_id_t& file_id, const char_t* collector, const char_t* strLocalPath
                             , const char_t* strDir, const char_t* strName, uint64_t create_time, file_size_t size
                             , bool_t bFinish, const char_t* checksum) {

  // 
  if (NULL == m_autoRelIFileSystem) { return RC_S_NULL_VALUE; }

  {
    AutoLock autoLock(m_lockFileSystem);
    const IFileNode* pIFileNode = m_autoRelIFileSystem->AddFile(file_id, collector, strLocalPath
      , strDir, strName, create_time, size
      , bFinish, checksum);
    if (NULL == pIFileNode) { return RC_S_FAILED; }
  }
  
  // wait up file iterator
  setLogTypeFile(file_id);
  notifyFileIterator(file_id);

//   // update file setting
//   char_t strFileID[32] = {0};
//   ITOA(file_id, strFileID, 10);
//   FileGSObject fileGSObject(NULL, strFileID, 0, pIFileNode->LocalPath(), 0, "PROCESS", 0);
//   rc_t rc = SetSetting(kStrUpdateFileInfo, &fileGSObject);
//   if (RC_S_OK != rc) { return rc; }
  return RC_S_OK;
}


rc_t AggregatorImpl::SetData(const file_id_t& file_id, IMemoryNode* memNode, file_size_t pos, file_size_t len) {

  if (NULL == m_autoRelIFileSystem) { return RC_S_NULL_VALUE; }

  file_size_t file_size = 0;
  rc_t rc;
  {
    AutoLock autoLock(m_lockFileSystem);
    rc = m_autoRelIFileSystem->SetData(&file_size, file_id, memNode, pos, len);
    if (RC_S_OK != rc) {
      m_autoRelIHostContext->Log(ILogWriter::LV_ERR, _STR("SetData Failed, %u"), rc);
      return rc; }
  }

  setLogTypeFile(file_id);
  // wait up file iterator
  notifyFileIterator(file_id);

  // update setting

  // lock
  {
    //AutoLock autoLock(m_lockMapFileUpdate);

    file_update_map_t::iterator it_map = m_mapFileUpdate.find(file_id);
    if (it_map == m_mapFileUpdate.end()) { m_mapFileUpdate[file_id] = 1;}
    else { ++it_map->second; }
  }

  m_timeAlive = DayTime::now();
  return rc;
}

void AggregatorImpl::clearMapFileUpdate() {

  updateMapFileUpdate();
  m_mapFileUpdate.clear();
}

void AggregatorImpl::updateMapFileUpdate() {

  // XXX
  //AutoLock autoLock(m_lockMapFileUpdate);

  file_update_map_t::iterator it_map, end;
  for (it_map = m_mapFileUpdate.begin(), end = m_mapFileUpdate.end(); it_map != end; ++it_map) {

    if (0 == it_map->second) { continue; }

    file_id_t file_id = it_map->first;
    it_map->second = 0;

    AutoRelease<IFileNode*> autoRelFileNode(m_autoRelIFileSystem->GetFileNode(file_id));
    if (NULL == autoRelFileNode) { continue; }

    char_t strFileSize[32] = {0};
    I64TOA(autoRelFileNode->Size(), strFileSize, sizeof(strFileSize), 10);    
    FileGSObject fileGSObject(NULL, NULL, file_id, NULL, 0, NULL, 0, NULL, 0, NULL, 0, strFileSize, 0);
    SetSetting(kStrUpdateFileInfo, &fileGSObject);
  }
}

void AggregatorImpl::reportMapFileUpdate(uint32_t now) {

  if (m_tcFileUpdate.GetIdleTime(now)) { return; }

  updateMapFileUpdate();
}

//////////////////////////////////////////////////////////////////////////
// resource
const rc_t AggregatorImpl::GetName(char_t* strName, uint32_t* size) const {
  if (NULL == strName || NULL == size || 0 == (*size)) { return RC_S_NULL_VALUE; }
  STRCPY(strName, (*size), m_strAggregator);
  return RC_S_OK;
}

const rc_t AggregatorImpl::GetStatus() const { return RC_S_OK; }

const rc_t AggregatorImpl::GetResourceSize(uint64_t* curr_size, uint64_t* free_size) const {

  if (NULL == curr_size || 0 == free_size) { return RC_S_NULL_VALUE; }

  (*curr_size) = 0x00;
  (*free_size) = 0x00;
  return RC_S_OK;
}

const rc_t AggregatorImpl::GetLastErr(uint32_t* err_no, char_t* err_msg, uint32_t* size) const {

  if (NULL == err_no || NULL == err_msg || NULL == size || 0 == (*size)) { return RC_S_NULL_VALUE; }

  (*err_no) = 0;
  STRCPY(err_msg, (*size), NULL_STR);
  return RC_S_OK;
}

const rc_t AggregatorImpl::GetInfo(char_t* info, uint32_t* size) const {

  if (NULL == info || NULL == size || 0 == (*size)) { return RC_S_NULL_VALUE; }
  
  uint32_t nDBPoolTotalSize = 0;
  uint32_t nDBPoolFreeSize = 0;
  getDBConnPoolSize(&nDBPoolTotalSize, &nDBPoolFreeSize);

  (*size) = SNPRINTF(info, (*size), (*size), _STR("%s: \n"
    "DBConnPool Total=%u, Free=%u\n"
    "ClientPool Total=%u, Free=%u\n"
    "Module Size=%u, Parser=%u, LogType=%u, Setting=%u\n"
    )

    , STR_VERSION
    , nDBPoolTotalSize, nDBPoolFreeSize
    , m_poolClient.total_size(), m_mapClient.size()
    , m_lstModule.size(), m_lstParseObject.size(), m_lstLogTypeObject.size(), m_lstSettingObject.size()
    );
  return RC_S_OK;
}

const rc_t AggregatorImpl::GetLastAliveTime(uint32_t* alive) const {

  if (NULL == alive) { return RC_S_NULL_VALUE; }

  (*alive) = m_timeAlive;
  return RC_S_OK;
}


//////////////////////////////////////////////////////////////////////////
// xml config
const char_t* strCFGFile                = _STR("aggregator.xml");
const char_t* strAggregator             = _STR("aggregator");
const char_t* strModules                = _STR("modules");
const char_t* strModule                 = _STR("module");

const char_t* strAttrName               = _STR("name");
const char_t* strAttrPath               = _STR("path");
const char_t* strAttrEnable             = _STR("enable");
const char_t* strValYes                 = _STR("yes");

const char_t* strLog                    = _STR("log");
const char_t* strAttrLevel              = _STR("level");
const char_t* strAttrPoolSize           = _STR("pool_size");

const char_t* strFileSystem             = _STR("filesystem");
//const char_t* strAttrCacheCount         = _STR("cache_count");
//const char_t* strAttrCCFileBlock        = _STR("cache_count_file_block");

const char_t* strDataBase               = _STR("database");
const char_t* strDBServer               = _STR("server");

const uint32_t gnPoolDBConnSize         = 32;
const uint32_t gnPoolDBConnInit         = 2;
const uint32_t gnPoolFileIteratorInit   = 2;

const uint32_t gnDefaultSize            = 1024 * 1024;

const ILogWriter::log_lv_t gnDefaultLogLv = ILogWriter::LV_INFO;
const uint32_t gnDefaultLogSize         = 4 * gnDefaultSize;
const uint32_t gnDefaultSubLogSize      = 1 * gnDefaultSize;

const uint32_t gnDefaultFSSize          = 64 * gnDefaultSize;

//////////////////////////////////////////////////////////////////////////

void AggregatorImpl::expatStart(void *data, const XML_Char *el, const XML_Char **attr) {

  if (NULL == data || NULL == el || NULL == attr) { return; }
  AggregatorImpl* pAggregatorImpl = (AggregatorImpl*)(data);
  ASSERT(pAggregatorImpl);
  ASSERT(pAggregatorImpl->m_autoRelIHostContext);

  // m_strAggregator
  if (0 == STRICMP(strAggregator, el)) {

    if (NULL == attr[0] || 0 != STRICMP(strAttrName, attr[0]) || NULL == attr[1]) { return; }
    const char_t* strVal = attr[1];

    pAggregatorImpl->SetAggregator(strVal);
  }
  // module
  else if (0 == STRICMP(strModule, el)) {

    if (NULL == attr[0] || 0 != STRICMP(strAttrPath, attr[0]) || NULL == attr[1]) { return; }
    if (NULL == attr[2] || 0 != STRICMP(strAttrEnable, attr[2]) 
      || NULL == attr[3] || 0 != STRICMP(strValYes, attr[3]) ) { return; }
    const char_t* strVal = attr[1];

    typedef std::basic_string<char_t> str_t;
    str_t strDyncFullPath(pAggregatorImpl->m_autoRelIHostContext->GetSystemExeFullPath());
    strDyncFullPath.append(STR_DIR_SEP);
    strDyncFullPath.append(strAggregator);
    strDyncFullPath.append(STR_DIR_SEP);
    strDyncFullPath.append(strVal);

    str_t strDyncFullDir(strDyncFullPath, 0, strDyncFullPath.find_last_of(STR_DIR_SEP));

    DynamicLibrary* dyncLib = new DynamicLibrary(strDyncFullPath.c_str(), strDyncFullDir.c_str());
    if (NULL == dyncLib) {
      pAggregatorImpl->m_autoRelIHostContext->Log(ILogWriter::LV_ERR, _STR("Load %s Failed."), strVal);
      return; }

    module_t module_dync;

    if (NULL == (module_dync.pfn_mci = dyncLib->LocateSymbol<PFN_MODULE_CREATE_INSTANCE>(strModuleCreateInstance))
      || NULL == (module_dync.module = (module_dync.pfn_mci)())
      ) {

      delete dyncLib;
      pAggregatorImpl->m_autoRelIHostContext->Log(ILogWriter::LV_ERR, _STR("Load %s Failed."), strVal);
      return;
    }

    // add to list
    module_dync.dyncLib = dyncLib;          
    pAggregatorImpl->m_lstModule.push_back(module_dync);
  }
  // log
  else if (0 == STRICMP(strLog, el)) {
/*
    ILogWriter::log_lv_t nLogLv = gnDefaultLogLv;
    uint32_t nPoolSize = gnDefaultLogSize;

    if (attr[0] && 0 == STRICMP(strAttrLevel, attr[0]) && attr[1]) {
      const char_t* strVal = attr[1];

      const char_t** strLevels = kStrLOG_LV;
      const char_t* strLevel;
      uint32_t idx = 0;
      while (NULL != (strLevel = *strLevels)) {

        if (0 == STRICMP(strLevel, strVal)) { nLogLv = (ILogWriter::log_lv_t)idx; break; }

        ++strLevels; ++idx;
      }      
    }

    // pool size
    if (attr[2] && 0 == STRICMP(strAttrPoolSize, attr[2]) && attr[3]) {
      const char_t* strVal = attr[3];

      nPoolSize = ATOI(strVal) * gnDefaultSize;
    }
*/
  }
  // filesystem
  else if (0 == STRICMP(strFileSystem, el)) {

    if (attr[0] && 0 == STRICMP(strAttrPoolSize, attr[0]) && attr[1]) {
      const char_t* strVal = attr[1];

      pAggregatorImpl->m_filesystem_buffer_size = ATOI(strVal) * gnDefaultSize;
    }
/*
    if (attr[2] && 0 == STRICMP(strAttrCacheCount, attr[2]) && attr[3]) {
      const char_t* strVal = attr[3];

      pAggregatorImpl->m_filesystem_cache_count = ATOI(strVal);
    }

    if (attr[4] && 0 == STRICMP(strAttrCCFileBlock, attr[4]) && attr[5]) {
      const char_t* strVal = attr[5];

      pAggregatorImpl->m_filesystem_cache_count_file_block = ATOI(strVal);
    }
*/
  }
  // database
  else if (0 == STRICMP(strDataBase, el)) {
    pAggregatorImpl->m_strLastEL = strDataBase;
  }

  else if (0 == STRICMP(strDBServer, el)) {
    if (strDataBase != pAggregatorImpl->m_strLastEL) { return; }

    // database
    {
      char_t strDBCS[MAX_DB_CS_LEN] = _STR("<server ");
      const char_t** strAttrs = attr;
      const char_t* strAttr;
      const char_t* strVal;
      while (NULL != (strAttr = *strAttrs++) && NULL != (strVal = *strAttrs++)) {

        SNPRINTF(strDBCS, MAX_DB_CS_LEN, MAX_DB_CS_LEN, _STR("%s%s=\"%s\" "), strDBCS, strAttr, strVal);
      }
      
      // save param
      pAggregatorImpl->m_listDBConnStr.push_back(db_conn_str_t());

      db_conn_str_t& dbConnStr = pAggregatorImpl->m_listDBConnStr.back();
      if (RC_S_OK != db_parse_cs(&dbConnStr, strDBCS)) { return; }

      if (pAggregatorImpl->m_mapDBConnPool.end() != pAggregatorImpl->m_mapDBConnPool.find(dbConnStr.db_driver)) { return;}

      //
      DBConn_pool_t* pDBConnPool = new DBConn_pool_t(dbConnStr.db_max_conn, dbConnStr.db_base_conn);
      if (NULL == pDBConnPool) { return; }

      // init pool
      pDBConnPool->init(&dbConnStr);

      uint32_t nKeepAlive = dbConnStr.db_keep_alive ? dbConnStr.db_keep_alive : kKeepDBConnTimeOut;
      pAggregatorImpl->m_tcDataBase.SetTimeOut(gcd(pAggregatorImpl->m_tcDataBase.GetTimeOut(), nKeepAlive));
      
      // add to map
      pAggregatorImpl->m_mapDBConnPool[dbConnStr.cs_name] = pDBConnPool;
    }
  }

  // other
}

void AggregatorImpl::expatEnd(void *data, const XML_Char *el) {
  //if (NULL == data || NULL == el) { return; }
  UNUSED_PARAM(data);
  UNUSED_PARAM(el);
}

void AggregatorImpl::defaultConfig() {

  // log


  // filesystem
  m_filesystem_buffer_size = gnDefaultFSSize;
  //
  m_strLastEL = NULL;
}

rc_t AggregatorImpl::readXmlConfig() {

  // read Configure
  char_t strPath[kMAX_PATH + 1] = {0};

  SNPRINTF(strPath, kMAX_PATH, _STR("%s/%s/%s"), m_autoRelIHostContext->GetSystemExeFullPath(), strAggregator, strCFGFile);

  AutoReleaseFile AutoRelFile(OpenFile(strPath, strAttrOpenRead));
  if (NULL == AutoRelFile) { return RC_S_NOTFOUND; }

  PlatformFileInfo file_info;
  if (FALSE == GetPlatformFileInfo(AutoRelFile, &file_info) || 0 == file_info.size) { return RC_S_NOTFOUND; }

  file_size_t xmlSize = file_info.size;
  AutoReleaseMemoryBase autoRelMem((uint8_t*)malloc((size_t)xmlSize));
  //uint8_t* strXML = new uint8_t[(size_t)xmlSize];
  if (NULL == autoRelMem) { return RC_E_NOMEM; }

  if (RC_S_OK != ReadFile(autoRelMem, &xmlSize, AutoRelFile, 0)) { return RC_S_NOTFOUND; }

  AutoReleaseXML autoRelXML(XML_ParserCreate(NULL));
  if (NULL == autoRelXML) { return RC_E_NOMEM; }

  XML_SetUserData(autoRelXML, this);
  XML_SetElementHandler(autoRelXML, expatStart, expatEnd);
  if (XML_STATUS_ERROR == XML_Parse(autoRelXML, (const char*)(uint8_t *)autoRelMem, (int)xmlSize, 1)) {
    return RC_S_FAILED;
  }
  return RC_S_OK;
}

//////////////////////////////////////////////////////////////////////////
struct ReadSettingReulst {

  AggregatorImpl* pAggregatorImpl;
  uint32_t        nCurrRow;
  const char_t*   pCurrSection;
  const char_t*   pCurrParam;  

  ReadSettingReulst(AggregatorImpl* pAggregatorImpl)
    : pAggregatorImpl(pAggregatorImpl)
    , nCurrRow(kuint32max)
    , pCurrSection(NULL)
    , pCurrParam(NULL)
  {}
};

typedef GetSetParamImpl<ReadSettingReulst*>   ReadSettingGSObject;
template<>
void ReadSettingGSObject::SetResult(uint32_t row, uint32_t col, const char_t* val, uint32_t len) {

  if (NULL == val || 0 == len) { return; }
  ASSERT(m_r);

  if (0 == col) { 

    m_r->nCurrRow = row; 
    m_r->pCurrSection = val;

    m_r->pCurrParam = NULL;
    return;
  }
  else {
    if (m_r->nCurrRow != row) { return; }

    else if (1 == col) { m_r->pCurrParam = val; return; }

    else if (2 == col && m_r->pCurrSection && m_r->pCurrParam) {

      if (0 == STRCMP(_STR("AGGREGATOR"), m_r->pCurrSection)) {

        if (0 == STRCMP(_STR("Name"), m_r->pCurrParam)) { 
          m_r->pAggregatorImpl->SetAggregator(val); return; }

        else if (0 == STRCMP(_STR("StepTimeResourceStatus"), m_r->pCurrParam)) {
          m_r->pAggregatorImpl->SetStepTimeResourceStatus(ATOI(val)); return; }

        else if (0 == STRCMP(_STR("StepTimeClientStatus"), m_r->pCurrParam)) {
          m_r->pAggregatorImpl->SetStepTimeClientStatus(ATOI(val)); return; }

        else if (0 == STRCMP(_STR("StepTimeRPCScan"), m_r->pCurrParam)) {
          m_r->pAggregatorImpl->SetStepTimeRPCInvoke(ATOI(val)); return; }

        else if (0 == STRCMP(_STR("CollectorFileLocalDir"), m_r->pCurrParam)) {
          m_r->pAggregatorImpl->SetCollectorLoaclPath(val); return; }

        else if (0 == STRCMP(_STR("StepTimeUpdateFile"), m_r->pCurrParam)) {
          m_r->pAggregatorImpl->SetStepTimeUpdateFile(ATOI(val)); return; }

        else if (0 == STRCMP(_STR("StepTimeCheckPoint"), m_r->pCurrParam)) {
          m_r->pAggregatorImpl->SetStepTimeCheckPoint(ATOI(val)); return; }

        else if (0 == STRCMP(_STR("FileSystemCacheSize"), m_r->pCurrParam)) {
          m_r->pAggregatorImpl->SetFileSystemCacheSize(ATOI(val) * gnDefaultSize); return; }

        else if (0 == STRCMP(_STR("WakeupParserTime"), m_r->pCurrParam)) {
          m_r->pAggregatorImpl->SetWakeupParserTime(ATOI(val)); return; }

        else if (0 == STRCMP(_STR("LongTickTime"), m_r->pCurrParam)) {
          m_r->pAggregatorImpl->SetLongTickTime(ATOI(val)); return; }

      }
    }
  }
}

rc_t AggregatorImpl::readSettingConfig() {

  ReadSettingReulst readSettingReulst(this);
  ReadSettingGSObject readSettingGSObject(&readSettingReulst);
  return GetSetting(&readSettingGSObject, kStrSystemSetting);
}

typedef GetSetParamImpl<char_t*>   CheckSystemGSObject;
template<>
void CheckSystemGSObject::SetResult(uint32_t row, uint32_t col, const char_t* val, uint32_t len) {

  if (NULL == val || 0 == len) { return; }
  ASSERT(m_r);

  if (0 == row && 0 == col) {
    uint32_t nReturnCode = ATOI(val);
    if (0 == nReturnCode) { (*m_r) = 0; }
  }
}

rc_t AggregatorImpl::checkSystem() {

  
  // get aggregator_dir
  char_t bOK = 1;
  CheckSystemGSObject checkSystemGSObject(&bOK);

  rc_t rc = GetSetting(&checkSystemGSObject, kStrCheckSystem);
  if (RC_S_OK != rc) { return rc; }

  return 0 == bOK ? RC_S_OK : RC_S_FAILED;
}

struct ReadSettingCollectorReulst {

  AggregatorImpl* pAggregatorImpl;
  const char_t*   id;
  const char_t*   size;
  const char_t*   satus;
  const char_t*   dir;
  const char_t*   name;
  const char_t*   ctime;
  const char_t*   collector;
  const char_t*   loacl_path;
  const char_t*   checksum;

  uint32_t        nMaxFileID;

  ReadSettingCollectorReulst(AggregatorImpl* pAggregatorImpl)
    : pAggregatorImpl(pAggregatorImpl)
    , id(NULL)
    , size(NULL)
    , satus(NULL)
    , dir(NULL)
    , name(NULL)
    , ctime(NULL)
    , collector(NULL)
    , loacl_path(NULL)
    , checksum(NULL)
    , nMaxFileID(IFileNode::INVALID_FILE_ID)
  {}
};


typedef GetSetParamImpl<ReadSettingCollectorReulst*>   ReadSettingCollectorGSObject;
template<>
void ReadSettingCollectorGSObject::SetResult(uint32_t row, uint32_t col, const char_t* val, uint32_t len) {

  //if (NULL == val || 0 == len) { return; }
  ASSERT(m_r);

  UNUSED_PARAM(len);
  UNUSED_PARAM(row);

  switch(col) {
    case 0: m_r->id = val; break;
    case 1: m_r->collector = val; break;
    case 2: m_r->loacl_path = val; break;
    case 3: m_r->dir = val; break;
    case 4: m_r->name = val; break;
    case 5: m_r->ctime = val; break;

    case 6: m_r->size = val; break;
    case 7: m_r->satus = val; break;
    case 8: m_r->checksum = val; {

      // add file
      ASSERT(m_r->pAggregatorImpl);
      if (NULL == m_r->id) { break; }
      file_id_t file_id = ATOI(m_r->id);
      file_size_t size = m_r->size ? ATOI64(m_r->size) : 0;      
      bool_t bFinish = m_r->satus ? (0 == STRCMP("PROCESS", m_r->satus) ? FALSE : TRUE) : FALSE;
      uint64_t create_time = ATOI64(m_r->ctime);
      m_r->pAggregatorImpl->AddFile(file_id, m_r->collector, m_r->loacl_path
        , m_r->dir, m_r->name, create_time, size
        , bFinish, m_r->checksum);

      if (m_r->nMaxFileID < file_id) { m_r->nMaxFileID = file_id; }
    }
    break;
  }
}

rc_t AggregatorImpl::readSettingCollector() {

  ReadSettingCollectorReulst readSettingCollectorReulst(this);
  // get aggregator_dir
  ReadSettingCollectorGSObject readSettingCollectorGSObject(&readSettingCollectorReulst);

  rc_t rc = GetSetting(&readSettingCollectorGSObject, kStrCollectorFile);

  if (RC_S_OK == rc) { m_acFileID = readSettingCollectorReulst.nMaxFileID; }
  return rc;
}

rc_t AggregatorImpl::Init(IHostContext* pHostContext) {

  m_autoRelIHostContext.Set(pHostContext);
  if (NULL == m_autoRelIHostContext) { return RC_S_NULL_VALUE; }

  m_autoRelIHostContext->Log(ILogWriter::LV_INFO, "Aggregator is working." STR_VERSION);

  STRCPY(m_strAggregatorDir, kMAX_PATH, m_autoRelIHostContext->GetSystemExeFullPath());
  size_t nLen = STRLEN(m_strAggregatorDir);
  while (nLen) {

    -- nLen;
    if (CHAR_DIR_SEP != m_strAggregatorDir[nLen] && '\\' != m_strAggregatorDir[nLen]) { break; }
  }

  m_strAggregatorDir[nLen + 1] = CHAR_DIR_SEP;
  m_strAggregatorDir[nLen + 2] = 0;

  STRCAT(m_strAggregatorDir, kMAX_PATH, strAggregator);

  // default
  defaultConfig();

  if (RC_S_OK != readXmlConfig()) { return RC_S_FAILED; }

  // filesystem
  if (RC_S_OK != initFileSystem()) { return RC_S_FAILED; }

  // fileiterator
  if (RC_S_OK != initFileIterator()) { return RC_S_FAILED; }

  // database
  if (RC_S_OK != initDatabase()) { return RC_S_FAILED; }

  // module
  if (RC_S_OK != initModule()) { return RC_S_FAILED; }

  // logtype
  if (RC_S_OK != initLogType()) { return RC_S_FAILED; }

  // client
  if (RC_S_OK != initClient()) { return RC_S_FAILED; }

  if (RC_S_OK != initRequestData()) { return RC_S_FAILED; }

  return RC_S_OK;
}

void AggregatorImpl::DeInit() {
}


USING_DATABASE(AutoInitDBThd);

void AggregatorImpl::ThreadMain(void* context) {

  thread_param_t* pThreadParam = (thread_param_t*)(context);

  if (NULL == pThreadParam) { return; }

  AutoInitDBThd autoInitDbThd;

  if (thread_param_t::MODULE == pThreadParam->type) {
    ASSERT(pThreadParam->pIModule);
    pThreadParam->pIModule->TickProc(pThreadParam->idx + 1);
  }
  else if (thread_param_t::PARSER == pThreadParam->type) {
    ASSERT(pThreadParam->pIParser);
    pThreadParam->pIParser->Start();
  }
}

rc_t AggregatorImpl::Start() {

  rc_t rc = RC_S_OK;

  if (NULL == m_autoRelIFileSystem) { return RC_S_FAILED; }

  m_autoRelIFileSystem->SetRootDir(m_strCollectorLoaclPath);

  rc = checkSystem();
  if (RC_S_OK != rc) {

    m_autoRelIHostContext->Log(ILogWriter::LV_ERR, _STR("CheckSystem Failed, check database or Config File."));
    return rc;
  }

  rc = readSettingConfig();
  if (RC_S_OK != rc) { return rc; }
  
  // read collector file
  /*rc = */readSettingCollector();
  //if (RC_S_OK != rc) { return rc; }

  // module
  {
    module_dynclib_t::iterator it_list, end;
    for (it_list = m_lstModule.begin(), end = m_lstModule.end(); RC_S_OK == rc && it_list != end; ++it_list) {

      IModule* pIModule = (*it_list).module;
      if (NULL == pIModule || 0 == pIModule->NeedMoreThread()) { continue; }
      
      uint32_t nThdCount = pIModule->NeedMoreThread();
      

      for (uint32_t idx = 0; idx < nThdCount; ++idx) {

        m_listThreadParam.push_back(thread_param_t());
        thread_param_t& threadParam = m_listThreadParam.back();
        threadParam.Set(thread_param_t::MODULE, pIModule, idx);

        if (NULL == m_thdgrpAggregatorImpl.create_thread(this, &threadParam)) {
          DynamicLibrary* dyncLib = (*it_list).dyncLib;
          if (NULL == dyncLib) {
            m_autoRelIHostContext->Log(ILogWriter::LV_ERR, _STR("Create Thread Error, unknown module"));
          }
          else {
            m_autoRelIHostContext->Log(ILogWriter::LV_ERR, _STR("Create Thread Error, %s"), dyncLib->LibraryName());
          }
        }
      } // for
    } // list
  }
  // parser
  {
    object_list_t::iterator it_list, end;
    for (it_list = m_lstParseObject.begin(), end = m_lstParseObject.end(); RC_S_OK == rc && it_list != end; ++it_list) {

      if (NULL == (*it_list)) { continue; }

      IParser* pIParser = (IParser*)((*it_list)->Cast(kUUID_IParser));
      if (NULL == pIParser) { continue; }

      m_listThreadParam.push_back(thread_param_t());
      thread_param_t& threadParam = m_listThreadParam.back();
      threadParam.Set(thread_param_t::PARSER, pIParser);

      if (NULL == m_thdgrpAggregatorImpl.create_thread(this, &threadParam)) {
        m_autoRelIHostContext->Log(ILogWriter::LV_ERR, _STR("Create Parser Thread Error")/*, pIParser->GetName()*/);
      }
    }
  }

  {
    static const char_t strTraceStart[] = _STR("Aggregator is working." STR_VERSION);
    const uint32_t nStrTraceStartLen = sizeof(strTraceStart) - 1;
    logDB(ILogWriter::LV_INFO, strAggregator, (uint8_t*)strTraceStart, nStrTraceStartLen, NULL, 0);
  }

  //
  m_timeStart = DayTime::now();
  return rc;
}

void AggregatorImpl::notifyFileIterator() {

  // log type file
  {
    logtype_map_t::iterator it_map, end;
    for (it_map = m_mapLogType.begin(), end = m_mapLogType.end(); it_map != end; ++it_map) {

      FileContainerImpl* pFileContainerImpl = it_map->second;
      //delete pFileContainerImpl;
      if (NULL == pFileContainerImpl) { continue; }
      pFileContainerImpl->Notify();
    }
  }
}

void AggregatorImpl::Stop() {

  // stop parser
  {
    notifyFileIterator();
    object_list_t::iterator it_list, end;
    for (it_list = m_lstParseObject.begin(), end = m_lstParseObject.end(); it_list != end; ++it_list) {

      if (NULL == (*it_list)) { continue; }

      IParser* pIParser = (IParser*)((*it_list)->Cast(kUUID_IParser));
      if (NULL == pIParser) { continue; }

      pIParser->Stop();
    }
  }

  notifyFileIterator();

  // stop thread
  m_thdgrpAggregatorImpl.join_all();
  m_listThreadParam.clear();

  clearMapFileUpdate();
  // 
  notifyFileIterator();

  {
    static const char_t strTraceStart[] = _STR("Aggregator is Stop");
    const uint32_t nStrTraceStartLen = sizeof(strTraceStart) - 1;
    logDB(ILogWriter::LV_INFO, strAggregator, (uint8_t*)strTraceStart, nStrTraceStartLen, NULL, 0);
  }

  // stop module
  {
    clearModule();
  }

  if (m_autoRelIFileSystem) { m_autoRelIFileSystem->CloseAllFile(); }
}

rc_t AggregatorImpl::NetProc(INetHandler* pINetHandler, const uint8_t* data, uint32_t len) {

  ASSERT(pINetHandler);
  rc_t rc;

  AutoInitDBThd autoInitDbThd;

  uint32_t nUniqueID = pINetHandler->GetSessionID();
  rc = packetProc(nUniqueID, len, data);
  if (RC_S_OK != rc) {
    //m_autoRelIHostContext->Log(ILogWriter::LV_WARN, "PacketProc failed. channelid=%u", nUniqueID);
    //return RC_S_CLOSED == rc || RC_E_ACCESS == rc ? RC_S_FAILED : RC_S_OK;
    return RC_S_FAILED;
  }

  AutoRelease<IMemoryNode*> autoRelMemNode(getResponsePacketData(nUniqueID));
  if (autoRelMemNode && autoRelMemNode->len()) {
    rc = pINetHandler->Send(NULL, autoRelMemNode->data(), autoRelMemNode->len());
    if (RC_S_OK != rc) {
      m_autoRelIHostContext->Log(ILogWriter::LV_WARN, "Send Data failed. channelid=%u", nUniqueID);
    }
    return rc;
  }

  return RC_S_OK;
}

uint32_t AggregatorImpl::GetTick() {

  uint32_t nGCD1 = gcd(m_tcFileUpdate.GetTimeOut(), m_tcDataBase.GetTimeOut());
  uint32_t nGCD2 = gcd(m_tcReportCount.GetTimeOut(), m_tcResourceStatus.GetTimeOut());
  uint32_t nGCD3 = gcd(m_tcClientStatus.GetTimeOut(), m_tcRPCInvoke.GetTimeOut());

  nGCD2 = gcd(nGCD3, nGCD2);
  nGCD1 = gcd(nGCD1, nGCD2);

  nGCD1 *= 1000;

  nGCD1 = gcd(nGCD1, m_nWakeupParserTime);
  return 0 == nGCD1 ? 1 * 1000 : nGCD1;
}

uint32_t AggregatorImpl::GetLongTick() {

  return m_nLongTickTime;
}

rc_t AggregatorImpl::TickProc() {

  notifyFileIterator();

  uint32_t now = DayTime::now();

  checkPoint(now);

  // check database link
  tickProc_Database(now);

  // Clinet
  reportClientStatus(now);

  // RPC
  reportRPC(now);

  reportCount(now);

  reportResourceStatus(now);

  reportMapFileUpdate(now);

  //
  if (m_autoRelIFileSystem) {
    m_autoRelIFileSystem->TickProc();
  }

  tickProc_Client();

  return RC_S_OK;
}


// module 
rc_t AggregatorImpl::LongTickProc() {

  AutoLock autoLock(m_lockModule);
  if (RC_S_OPEN != m_eModuleStatus) { return RC_S_STATUS; }

  module_dynclib_t::iterator it_list, end;
  for (it_list = m_lstModule.begin(), end = m_lstModule.end(); it_list != end; ++it_list) {

    IModule* pIModule = (*it_list).module;
    //if (NULL == pIModule || 0 == pIModule->NeedMoreThread()) { continue; }
    if (NULL == pIModule) { continue; }

    pIModule->TickProc(0);
  }

  return RC_S_OK;
}

//////////////////////////////////////////////////////////////////////////

const char_t* AggregatorImpl::m_strParserThdName    = _STR("ParserThread");

const char_t* kStrDefaultLocalDir       = _STR("CollectorDefaultLocal");
const char_t* kStrDefaultAggregator     = _STR("Aggregator");
//////////////////////////////////////////////////////////////////////////
AggregatorImpl::AggregatorImpl() 
  : REF_COUNT_CONSTRUCT

  , m_autoRelIHostContext(NULL)
  , m_timeStart(0)
  , m_timeAlive(0)

  // module
  , m_eModuleStatus(RC_S_UNKNOWN)
  , m_lstSettingObject()
  , m_lstParseObject()
  , m_lstLogTypeObject()
  , m_lstModule()

  // database
  , m_mapDBConnPool()
  , m_nPoolDBConnSize(gnPoolDBConnSize)
  , m_listDBConnStr()

  // file system
  , m_poolFileIterator(64, gnPoolFileIteratorInit)
  , m_autoRelIFileSystem(NULL)
  
  // parser
  , m_mapFileLogType()
  , Thread(m_strParserThdName)

  // log
  , m_LogReader()
  , m_listRequest()

  // client
  , m_mapClient()
  , m_poolClient(128, 1)
  , m_lockClient()

  , m_nCountFailedGetDBConn(0)

  , m_tcDataBase(60 * 60)
  , m_tcResourceStatus(60)
  , m_tcClientStatus(60)
  , m_tcRPCInvoke(60)
  , m_tcReportCount(30 * 60)

  , m_tcFileUpdate(30 * 60)
  , m_tcCheckPoint(30 * 60)

  , m_nWakeupParserTime(300)
  , m_nLongTickTime(60 * 1000)

{
  SetAggregator(kStrDefaultAggregator);
  SetCollectorLoaclPath(kStrDefaultLocalDir);
  BZERO_ARR(m_strAggregatorDir);
  STRCPY(m_strAggregatorDir, sizeof(m_strAggregatorDir), strAggregator);
  initLog();
}

AggregatorImpl::~AggregatorImpl() {

  clearFileIterator();
  clearLogType();

  clearClient();
  clearRequestData();

  clearDatabase();

  clearFileSystem();
  clearEncodeSystem();

  clearLog();
}

//////////////////////////////////////////////////////////////////////////
// setting
rc_t AggregatorImpl::SetSetting(char_t const* name, const IGetSetParam* pIGetSetParam) {

  rc_t rc = RC_S_FAILED;

  object_list_t::iterator it_list, end;
  for (it_list = m_lstSettingObject.begin(), end = m_lstSettingObject.end(); it_list != end; ++it_list) {

    if (NULL == (*it_list)) { continue; }

    ISetting* pISetting = (ISetting*)((*it_list)->Cast(kUUID_ISetting));
    if (NULL == pISetting) { continue; }

    if (RC_S_OK == pISetting->Set(name, pIGetSetParam)) { rc = RC_S_OK; }
  }
  return rc;
}

rc_t AggregatorImpl::GetSetting(IGetSetParam* pIGetSetParam, char_t const* name) {

  rc_t rc = RC_S_NOTFOUND;

  object_list_t::iterator it_list, end;
  for (it_list = m_lstSettingObject.begin(), end = m_lstSettingObject.end(); it_list != end; ++it_list) {

    if (NULL == (*it_list)) { continue; }

    ISetting* pISetting = (ISetting*)((*it_list)->Cast(kUUID_ISetting));
    if (NULL == pISetting) { continue; }

    if (RC_S_OK == pISetting->Get(pIGetSetParam, name)) { return RC_S_OK; }
  }

  return rc;
}

//////////////////////////////////////////////////////////////////////////
// log type
void AggregatorImpl::addLogTypes() {

  object_list_t::const_iterator it_list, end;
  for (it_list = m_lstLogTypeObject.begin(), end = m_lstLogTypeObject.end(); it_list != end; ++it_list) {

    if (NULL == (*it_list)) { continue; }

    ILogType* pILogType = (ILogType*)((*it_list)->Cast(kUUID_ILogType));
    if (NULL == pILogType) { continue; }

    addLogType(pILogType->LogType());    
  }
}

rc_t AggregatorImpl::setLogTypeFile(const file_id_t& file_id) {

  // get filenode
  if (NULL == m_autoRelIFileSystem) { return RC_S_NOTFOUND; }

  AutoRelease<IFileNode*> autoRelFileNode(m_autoRelIFileSystem->GetFileNode(file_id));
  if (NULL == autoRelFileNode) { return RC_S_NOTFOUND; }

  //file_size_t pos = 0;
  //AutoRelease<IMemoryNode*> autoMemNode(m_autoRelIFileSystem->GetData(file_id, &pos, 0x00));

  LogTypeIdx* pLogTypeIdx = NULL;
  file_logtype_map_t::iterator it_map = m_mapFileLogType.find(file_id);
  if (it_map == m_mapFileLogType.end() || NULL == it_map->second) { 

    pLogTypeIdx = new LogTypeIdx(m_lstLogTypeObject.size());
    if (NULL == pLogTypeIdx) { return RC_E_NOMEM; }

    m_mapFileLogType[file_id] = pLogTypeIdx;
  }
  else {
    pLogTypeIdx = it_map->second;
  }

  ASSERT(pLogTypeIdx);
  uint32_t idx;
  object_list_t::iterator it_list, end;
  for (idx = 0, it_list = m_lstLogTypeObject.begin(), end = m_lstLogTypeObject.end();
    idx < pLogTypeIdx->m_vecIdx.size() && it_list != end;
    ++idx, ++it_list) {

    if (pLogTypeIdx->m_vecIdx[idx]) { continue; }
    
    ILogType* pILogType = (ILogType*)((*it_list)->Cast(kUUID_ILogType));
    if (NULL == pILogType) { pLogTypeIdx->m_vecIdx[idx] = NULL_STR; continue; }

    const char_t* strLogType = pILogType->LogType(autoRelFileNode, NULL);
    // add logtype
    if (NULL == strLogType) { continue; }

    pLogTypeIdx->m_vecIdx[idx] = strLogType;
    (void) addLogTypeFile(strLogType, file_id);
  }

  return RC_S_OK;
}

//////////////////////////////////////////////////////////////////////////
rc_t AggregatorImpl::notifyFileIterator(const file_id_t& /*file_id*/) {

  // get log type 
  /*
  file_logtype_map_t::iterator it_map = m_mapFileLogType.find(file_id);
  if (m_mapFileLogType.end() == it_map || NULL == it_map->second) { return RC_S_NOTFOUND; }

  {
    const LogTypeIdx::logtype_idx_vector_t& vctLogTypeIdx = it_map->second->m_vecIdx;
    for (size_t idx = 0, end = vctLogTypeIdx.size(); idx != end; ++idx) {

      if (NULL == vctLogTypeIdx[idx]) { continue; }

      const char_t* strLogType = vctLogTypeIdx[idx];
      ASSERT(strLogType);
      AutoRelease<FileContainerImpl*> autoRelFileContainerImpl = getFileContainer(strLogType);
      if (NULL == autoRelFileContainerImpl) { continue; }

      autoRelFileContainerImpl->Notify();
    }
  }
  */
  return RC_S_OK;
}

//////////////////////////////////////////////////////////////////////////
// Module
rc_t AggregatorImpl::initModule() {

  // init moduel
  {
    //AutoLock autoLock(m_lockModule);
    module_dynclib_t::iterator it_list, end;
    for (it_list = m_lstModule.begin(), end = m_lstModule.end(); it_list != end; ++it_list) {

      IModule* pIModule = (*it_list).module;
      if (NULL == pIModule) { continue; }

      AddRef();
      if (RC_S_OK != pIModule->Init(this)) {

        m_autoRelIHostContext->Log(ILogWriter::LV_WARN, "Module Init Failed. %s", pIModule->GetName());
      }

      m_eModuleStatus = RC_S_OPEN;
    }
  }

  return RC_S_OK;
}

void AggregatorImpl::clearModule() {

  // function
  {
    object_list_t::iterator it_list, end;
    for (it_list = m_lstSettingObject.begin(), end = m_lstSettingObject.end(); it_list != end; ++it_list) {

      if (NULL == (*it_list)) { continue; }

      ISetting* pISetting = (ISetting*)((*it_list)->Cast(kUUID_ISetting));
      if (NULL == pISetting) { continue; }

      pISetting->Release();
    }
    m_lstSettingObject.clear();

    for (it_list = m_lstParseObject.begin(), end = m_lstParseObject.end(); it_list != end; ++it_list) {

      if (NULL == (*it_list)) { continue; }

      IParser* pIParser = (IParser*)((*it_list)->Cast(kUUID_IParser));
      if (NULL == pIParser) { continue; }

      pIParser->Release();
    }
    m_lstParseObject.clear();

    for (it_list = m_lstLogTypeObject.begin(), end = m_lstLogTypeObject.end(); it_list != end; ++it_list) {

      if (NULL == (*it_list)) { continue; }

      ILogType* pILogType = (ILogType*)((*it_list)->Cast(kUUID_ILogType));
      if (NULL == pILogType) { continue; }

      pILogType->Release();
    }
    m_lstLogTypeObject.clear();
  }

  // dynamiclibrary
  {
    //AutoLock autoLock(m_lockModule);
    module_dynclib_t::iterator it_list, end;
    for (it_list = m_lstModule.begin(), end = m_lstModule.end(); it_list != end; ++it_list) {

      IModule* pIModule = (*it_list).module;
      if (NULL == pIModule) { continue; }

      pIModule->DeInit();
      pIModule->Release();
      delete (*it_list).dyncLib;
    }
    m_lstModule.clear();
    m_eModuleStatus = RC_S_CLOSED;
  }
}

//////////////////////////////////////////////////////////////////////////
// filesystem
rc_t AggregatorImpl::initFileSystem() {

  if (m_autoRelIFileSystem) { return RC_S_OK; }
  
  m_autoRelIFileSystem.Set(IFileSystem::CreateInstance(m_filesystem_buffer_size / gnDefaultSize));

  if (NULL == m_autoRelIFileSystem) { return RC_E_NOMEM; }
  return RC_S_OK;
}

void AggregatorImpl::clearFileSystem() {
  m_autoRelIFileSystem.Release();
}

void AggregatorImpl::SetFileSystemCacheSize(size_t size) {

  if (size > m_filesystem_buffer_size) {
    m_filesystem_buffer_size = size;
  }
}

// tFileIterator
rc_t AggregatorImpl::initFileIterator() {

  FileIteratorParam_t fileIteratorParam;
  fileIteratorParam.pIAggregator = this;
  fileIteratorParam.pIFileSystem = m_autoRelIFileSystem;
  m_poolFileIterator.init(fileIteratorParam);

  return RC_S_OK;
}

void AggregatorImpl::clearFileIterator() {

  m_poolFileIterator.clear();
}

//////////////////////////////////////////////////////////////////////////
// logtype
rc_t AggregatorImpl::initLogType() {

  addLogTypes();

  return RC_S_OK;
}

void AggregatorImpl::clearLogType() {

  // log type file
  {
    logtype_map_t::iterator it_map, end;
    for (it_map = m_mapLogType.begin(), end = m_mapLogType.end(); it_map != end; ++it_map) {

      FileContainerImpl* pFileContainerImpl = it_map->second;
      delete pFileContainerImpl;
    }
    m_mapLogType.clear();
  }

  // File LogType
  {
    file_logtype_map_t::iterator it_map, end;
    for (it_map = m_mapFileLogType.begin(), end = m_mapFileLogType.end(); it_map != end; ++it_map) {

      delete it_map->second;
    }
    m_mapFileLogType.clear();
  }
}

AggregatorImpl::FileContainerImpl* AggregatorImpl::getFileContainer(const char_t* strType) {

  logtype_map_t::iterator it_map = m_mapLogType.find(strType);
  if (m_mapLogType.end() == it_map || NULL == it_map->second) { return NULL; }

  FileContainerImpl* pFileContainerImpl = it_map->second;
  pFileContainerImpl->AddRef();

  // add log type 
  return pFileContainerImpl;
}

rc_t AggregatorImpl::addLogType(const char_t* strLogType) {

  FileContainerImpl* pFileContainerImpl = new FileContainerImpl();
  if (NULL == pFileContainerImpl) { return RC_E_NOMEM; }

  if (m_mapLogType.end() != m_mapLogType.find(strLogType)) {
    return RC_S_DUPLICATE;
  }

  m_mapLogType[strLogType] = pFileContainerImpl;
  return RC_S_OK;
}

rc_t AggregatorImpl::addLogTypeFile(const char_t* strType, const file_id_t& file_id) {

  logtype_map_t::iterator it_map = m_mapLogType.find(strType);
  if (m_mapLogType.end() == it_map || NULL == it_map->second) { return RC_S_NOTFOUND; }

  FileContainerImpl* pFileContainerImpl = it_map->second;
  ASSERT(pFileContainerImpl);

  pFileContainerImpl->AddFile(file_id);
  return RC_S_OK;
}

//////////////////////////////////////////////////////////////////////////
// Database
rc_t AggregatorImpl::initDatabase() {

  return RC_S_OK;
}

void AggregatorImpl::clearDatabase() {
  // database
  {
    USING_DATABASE(db_lib_init);
    // TS Release mysql
    db_lib_init();
    AutoInitDBThd autoInitDbThd;

    dbconn_pool_map_t::iterator it_map, end;
    for (it_map = m_mapDBConnPool.begin(), end = m_mapDBConnPool.end(); it_map != end; ++it_map) {

      delete it_map->second;
    }
    m_mapDBConnPool.clear();
  }
}

// keepaliveDatabase
void AggregatorImpl::tickProc_Database(uint32_t now) {

  dbconn_pool_map_t::iterator it_map, end;
  for (it_map = m_mapDBConnPool.begin(), end = m_mapDBConnPool.end(); it_map != end; ++it_map) {

    DBConn_pool_t* pDBConnPool = it_map->second;
    if (NULL == pDBConnPool) { continue; }
    
    AutoRelease<DBConnImpl*> autoRelDBConn(NULL);
    do {
      autoRelDBConn.Set(pDBConnPool->get_no_create());
      if (NULL == autoRelDBConn || RC_S_OK != autoRelDBConn->KeepAlive(now)) { break; }
    } while (autoRelDBConn);
  }
}

//////////////////////////////////////////////////////////////////////////
// log
rc_t AggregatorImpl::initLog() {

  //m_LogReader.Start
  return RC_S_OK;
}

void AggregatorImpl::clearLog() {

  m_LogReader.Stop();
}

rc_t AggregatorImpl::logDB(ILogWriter::log_lv_t lv, const char_t* from
           , const uint8_t* data, uint32_t dlen
           , const uint8_t* content, uint32_t clen) {

  //
  if (lv > ILogWriter::LV_TRACE) { return RC_E_ACCESS; }
  NormalSetGSObject logDBGSObject(NULL, kStrLOG_LV[lv], 0, from, 0
    , (const char_t*)data, dlen, (const char_t*)content, clen);
  return SetSetting(kStrCollectLog, &logDBGSObject);
}

//////////////////////////////////////////////////////////////////////////
rc_t AggregatorImpl::makeFileID(file_id_t& file_id, const char_t* collector, const char_t* dir, const char_t* name, uint64_t create_time) {

  ASSERT(collector && dir && name);
  // make sure this is a new file

  file_id = ++m_acFileID;

  return AddFile(file_id, collector, dir, name, create_time);
  /*
  char_t strCreateTime[64] = {0};
  I64TOA(create_time, strCreateTime, 10);
  FileGSObject fileGSObject(&file_id, collector, 0, dir, 0, name, 0, strCreateTime, 0);
  rc_t rc = GetSetting(&fileGSObject, kStrGetFileID);
  if (RC_S_OK != rc) { file_id = IFileNode::INVALID_FILE_ID; return rc; }
  //file_id = fileGSObject.GetResult();
  return AddFile(file_id, collector, dir, name, create_time);
  */
}

rc_t AggregatorImpl::updateFileData(ICoder* pICoderDefalte, const file_id_t& file_id, file_size_t pos
                                    , EncodeType_e type, uint32_t olen
                                    , uint32_t len, const uint8_t* data) {

  // check
  if (IFileNode::INVALID_FILE_ID == file_id || 0 == len || NULL == data) {
    m_autoRelIHostContext->Log(ILogWriter::LV_ERR, _STR("updateFileData Failed, NULL VALUE."));
    return RC_S_NULL_VALUE; }

  if (NULL == m_autoRelIFileSystem) {
    m_autoRelIHostContext->Log(ILogWriter::LV_ERR, _STR("updateFileData Failed, NULL FileSystem."));
    return RC_S_FAILED; }

  AutoLock autoLock(m_lockFileSystem);

  AutoRelease<IFileNode*> autoRelFileNode(m_autoRelIFileSystem->GetFileNode(file_id));
  if (NULL == autoRelFileNode) {
    m_autoRelIHostContext->Log(ILogWriter::LV_ERR, _STR("updateFileData Failed, NULL FileNode.%u"), file_id);
    return RC_S_FAILED; }

  if (autoRelFileNode->Size() < pos) { return RC_S_OK; }

  if (DEFLATE == type) {

    if (NULL == pICoderDefalte) { return RC_S_NULL_VALUE; }

    // get memory
    AutoRelease<IMemoryNode*> autoRelMemNode(m_autoRelIFileSystem->GetMemory(olen));
    if (NULL == autoRelMemNode) {
      m_autoRelIHostContext->Log(ILogWriter::LV_ERR, _STR("updateFileData Failed, NO Memory"));
      return RC_E_NOMEM; }

    if (olen > autoRelMemNode->len()) {
      m_autoRelIHostContext->Log(ILogWriter::LV_ERR, _STR("updateFileData Failed, Org Data Len Too Big, %u"), olen);
      return RC_S_FAILED; }

    uint32_t decode_size = olen;
    pICoderDefalte->decode(autoRelMemNode->data(), &decode_size, data, len);
    if (decode_size != olen) {
      m_autoRelIHostContext->Log(ILogWriter::LV_ERR, _STR("updateFileData Failed, DeCode Data Failed"));
      return RC_S_FAILED; }

    // default.
    return SetData(file_id, autoRelMemNode, pos, decode_size);
  }
  else if (NONE == type) {

    AutoRelease<IMemoryNode*> autoRelMemNode(m_autoRelIFileSystem->GetMemory(len));
    if (NULL == autoRelMemNode) {
      m_autoRelIHostContext->Log(ILogWriter::LV_ERR, _STR("updateFileData Failed, NO Memory"));
      return RC_E_NOMEM; }

    ASSERT(len <= autoRelMemNode->len());
    MEMCPY(autoRelMemNode->data(), autoRelMemNode->len(), data, len);

    return SetData(file_id, autoRelMemNode, pos, len);
  }

  return RC_E_UNSUPPORTED;
}

IFileNode* AggregatorImpl::getFileNode(const file_id_t& file_id) {

  // get filenode
  if (NULL == m_autoRelIFileSystem) { return NULL; }

  return m_autoRelIFileSystem->GetFileNode(file_id);
}

//////////////////////////////////////////////////////////////////////////
// client
AggregatorImpl::CollectClient* AggregatorImpl::addClient(const char_t* strCollector) {

  if (NULL == strCollector) { return NULL; }
  AutoLock autoLock(m_lockClient);

  client_map_t::iterator it_map = m_mapClient.find(strCollector);
  if (m_mapClient.end() != it_map) { return it_map->second; }

  // new client
  CollectClient* pCollectClient = m_poolClient.get();
  if (NULL == pCollectClient) { return NULL; }

  pCollectClient->SetName(strCollector);
  pCollectClient->SetLogon(TRUE);

  m_mapClient[pCollectClient->GetName()] = pCollectClient;
  return pCollectClient;
}

AggregatorImpl::CollectClient* AggregatorImpl::findClient(const char_t* strCollector) {

  if (NULL == strCollector) { return NULL; }
  AutoLock autoLock(m_lockClient);

  client_map_t::iterator it_map = m_mapClient.find(strCollector);
  if (m_mapClient.end() == it_map) { return NULL; }

  return it_map->second;
}

rc_t AggregatorImpl::removeClient(const char_t* strCollector) {

  if (NULL == strCollector) { return RC_S_NULL_VALUE; }
  AutoLock autoLock(m_lockClient);

  client_map_t::iterator it_map = m_mapClient.find(strCollector);
  if (m_mapClient.end() == it_map) { return RC_S_NOTFOUND; }
  if (it_map->second) { m_poolClient.release(it_map->second); }
  m_mapClient.erase(it_map);
  return RC_S_OK;
}

rc_t AggregatorImpl::initClient() {

  m_poolClient.init(NULL);

  return RC_S_OK;
}

void AggregatorImpl::clearClient() {

  client_map_t::iterator it_map, end;
  for (it_map = m_mapClient.begin(), end = m_mapClient.end(); it_map != end; ++it_map) {

    if (NULL == it_map->second) { continue; }

    if (it_map->second) { m_poolClient.release(it_map->second); }
  }
  m_mapClient.clear();
}

void AggregatorImpl::tickProc_Client() {

  uint32_t now = DayTime::now();

  AutoLock autoLock(m_lockClient);

  client_map_t::iterator it_map, end;
  for (it_map = m_mapClient.begin(), end = m_mapClient.end(); it_map != end; /*++it_map*/) {

    if (NULL == it_map->second) { continue; }
    CollectClient* pCollectClient = it_map->second;
    if (now > pCollectClient->GetKeepAlive() + kKeepClientTimeOut) {

      // 
      m_poolClient.release(pCollectClient);
      it_map = m_mapClient.erase(it_map);
    }
    else {
      ++it_map;
    }
  }
}

//////////////////////////////////////////////////////////////////////////
// count
void AggregatorImpl::reportCount(uint32_t now) {

  if (m_tcReportCount.GetIdleTime(now)) { return; }

  if (64 <= m_nCountFailedGetDBConn) {

    static const char_t strTraceStart[] = _STR("DataBase Cann't Get Connection, You can modify Aggregator config file");
    const uint32_t nStrTraceStartLen = sizeof(strTraceStart) - 1;
    logDB(ILogWriter::LV_WARN, strAggregator, (uint8_t*)strTraceStart, nStrTraceStartLen, NULL, 0);
  }

  m_nCountFailedGetDBConn = 0;
}

//////////////////////////////////////////////////////////////////////////


rc_t AggregatorImpl::reportResourceStatus(const IResource* pIResource) {

  ASSERT(pIResource);
  //
  char_t strName[64] = {0};
  char_t strCurrSize[64] = {0};
  char_t strFreeSize[64] = {0};
  char_t strErrMsg[128] = {0};
  char_t strInfo[256] = {0};

  uint64_t nCurrSize = 0;
  uint64_t nFreeSize = 0;
  uint32_t nErrNo = 0;
  uint32_t nLastAliveTime = 0;

  uint32_t nStrLen = sizeof(strName);
  pIResource->GetName(strName, &nStrLen);
  pIResource->GetResourceSize(&nCurrSize, &nFreeSize);
  I64TOA(nCurrSize, strCurrSize, sizeof(strCurrSize), 10);
  I64TOA(nFreeSize, strFreeSize, sizeof(strFreeSize), 10);

  nStrLen = sizeof(strErrMsg);
  pIResource->GetLastErr(&nErrNo, strErrMsg, &nStrLen);
  pIResource->GetLastAliveTime(&nLastAliveTime);

  nStrLen = sizeof(strInfo);
  pIResource->GetInfo(strInfo, &nStrLen);

  NormalSetGSObject collectorStatusGSObject(NULL, strName, 0, strCurrSize, nLastAliveTime
    , strFreeSize, nErrNo, strErrMsg, 0, strInfo);
  SetSetting(kStrResourceStatus, &collectorStatusGSObject);

  return RC_S_OK;
}

void AggregatorImpl::reportResourceStatus(uint32_t now) {

  if (m_tcResourceStatus.GetIdleTime(now)) { return; }

  // Aggregator
  {
    const IResource* pIResource = (this);
    if (pIResource) { reportResourceStatus(pIResource); }
  }

  // file system
  {
    const IResource* pIResource = (const IResource*)(m_autoRelIFileSystem);
    if (pIResource) { reportResourceStatus(pIResource); }
  }

  // module 
  {
    AutoLock autoLock(m_lockModule);

    // parser
    object_list_t::const_iterator it_list, end;
    for (it_list = m_lstParseObject.begin(), end = m_lstParseObject.end(); it_list != end; ++it_list) {

      IParser* pIParser = (IParser*)((*it_list)->Cast(kUUID_IParser));
      if (NULL == pIParser) { continue; }

      reportResourceStatus(pIParser);
    }

    // LogType
    //object_list_t::const_iterator it_list, end;
    for (it_list = m_lstLogTypeObject.begin(), end = m_lstLogTypeObject.end(); it_list != end; ++it_list) {

      ILogType* pILogType = (ILogType*)((*it_list)->Cast(kUUID_ILogType));
      if (NULL == pILogType) { continue; }

      reportResourceStatus(pILogType);
    }

    // Setting
    //object_list_t::const_iterator it_list, end;
    for (it_list = m_lstSettingObject.begin(), end = m_lstSettingObject.end(); it_list != end; ++it_list) {

      ISetting* pISetting = (ISetting*)((*it_list)->Cast(kUUID_ISetting));
      if (NULL == pISetting) { continue; }

      reportResourceStatus(pISetting);
    }
  }
}

void AggregatorImpl::reportClientStatus(uint32_t now) {

  if (m_tcClientStatus.GetIdleTime(now)) { return; }

  // aggregator
  NormalSetGSObject collectorStatusGSObject(NULL, m_strAggregator, 0, _STR("0"), 0
    , _STR("0"), m_timeStart, _STR("0"), DayTime::now());
  SetSetting(kStrCollectorStatus, &collectorStatusGSObject);

  // client
  AutoLock autoLock(m_lockClient);
  client_map_t::iterator it_map, end;
  for (it_map = m_mapClient.begin(), end = m_mapClient.end(); it_map != end; ++it_map) {

    if (NULL == it_map->second) { continue; }
    CollectClient* pCollectClient = it_map->second;

    char_t strRecvFileDataReport[64] = {0};
    char_t strRecvFileData[64] = {0};
    char_t strRecvFileDataReal[64] = {0};

    I64TOA(pCollectClient->GetRecvFileDataReport(), strRecvFileDataReport, sizeof(strRecvFileDataReport), 10);
    I64TOA(pCollectClient->GetRecvFileData(), strRecvFileData, sizeof(strRecvFileData), 10);
    I64TOA(pCollectClient->GetRecvFileDataReal(), strRecvFileDataReal, sizeof(strRecvFileDataReal), 10);
    
    NormalSetGSObject collectorStatusGSObject(NULL, pCollectClient->GetName(), 0, strRecvFileDataReport, 0
      , strRecvFileData, pCollectClient->GetLogonTime(), strRecvFileDataReal, pCollectClient->GetKeepAlive());
    SetSetting(kStrCollectorStatus, &collectorStatusGSObject);
  }
}


struct RPCInvokeParam {

  AggregatorImpl* pAggregatorImpl;
  const char_t*   id;
  const char_t*   from;
  const char_t*   target;
  const char_t*   name;

  RPCInvokeParam(AggregatorImpl* pAggregatorImpl)
    : pAggregatorImpl(pAggregatorImpl)
    , id(NULL)
    , from(NULL)
    , target(NULL)
    , name(NULL)
  {}
};


typedef GetSetParamImpl<RPCInvokeParam*>   RPCInvokeGSObject;
template<>
void RPCInvokeGSObject::SetResult(uint32_t row, uint32_t col, const char_t* val, uint32_t len) {

  //if (NULL == val || 0 == len) { return; }
  ASSERT(m_r);

  UNUSED_PARAM(len);
  UNUSED_PARAM(row);

  switch(col) {
    case 0: m_r->id = val; break;
    case 1: m_r->from = val; break;
    case 2: m_r->target = val; break;
    case 3: m_r->name = val; break;
    case 4: {

      if (NULL == m_r->id) { break; }
      uint32_t id = ATOI(m_r->id);

      m_r->pAggregatorImpl->AddRPCInvoke(id, m_r->from, m_r->target, m_r->name, (const uint8_t*)(val), len);
      break;
    }
  }
}

void AggregatorImpl::reportRPC(uint32_t now) {

  if (m_tcRPCInvoke.GetIdleTime(now)) { return; }

  // get rpc
  RPCInvokeParam rpcInvokeParam(this);

  RPCInvokeGSObject rpcInvokeGSObject(&rpcInvokeParam);
  GetSetting(&rpcInvokeGSObject, kStrGetRPCInvoke);

  // 
  rpc_node_list_t::iterator it_list, end;
  for (it_list = m_listRPCNode.begin(), end = m_listRPCNode.end(); it_list != end; ++it_list) {

    const RPCInvokeNode& rpcInvokeNode = *(it_list);

    //
    if (0 == STRCMP("CollectNow", rpcInvokeNode.name)) { rpcCollectNow(); }

    reportRPC(rpcInvokeNode.id, RC_S_OK, NULL_STR, NULL, 0);
  }

  m_listRPCNode.clear();
}

rc_t AggregatorImpl::AddRPCInvoke(uint32_t id, const char_t* from, const char_t* target, const char_t* name
                                  , const uint8_t* param, uint32_t len) {

  // 
  if (NULL == from || NULL == target || NULL == name) {
    return reportRPC(id, RC_E_ILLEGAL_PARAM, NULL_STR, NULL, 0);
  }

  m_listRPCNode.push_back(RPCInvokeNode());
  RPCInvokeNode& rpcInvokeNode = m_listRPCNode.back();

  rpcInvokeNode.id = id;
  STRCPY(rpcInvokeNode.from, sizeof(rpcInvokeNode.from), from);
  STRCPY(rpcInvokeNode.target, sizeof(rpcInvokeNode.target), target);
  STRCPY(rpcInvokeNode.name, sizeof(rpcInvokeNode.name), name);

  if (len > 1024) { MEMCPY(rpcInvokeNode.param, sizeof(rpcInvokeNode.param), param, 1024); }
  else { MEMCPY(rpcInvokeNode.param, sizeof(rpcInvokeNode.param), param, len); }

  return RC_S_OK;
}

rc_t AggregatorImpl::reportRPC(uint32_t id, rc_t err_no, const char_t* err_msg, const uint8_t* data, uint32_t len) {

  char_t strID[32] = {0};
  char_t strErrNo[32] = {0};

  ITOA(id, strID, 10);
  ITOA(err_no, strErrNo, 10);

  NormalSetGSObject rpcInvokeGSObject(NULL, strID, 0, strErrNo, 0
    , err_msg, 0, (const char_t*)data, len);
  return SetSetting(kStrSetRPCInvoke, &rpcInvokeGSObject);
}

rc_t AggregatorImpl::rpcCollectNow() {

  AutoLock autoLock(m_lockClient);

  client_map_t::iterator it_map, end;
  for (it_map = m_mapClient.begin(), end = m_mapClient.end(); it_map != end; ++it_map) {

    if (NULL == it_map->second) { continue; }
    CollectClient* pCollectClient = it_map->second;

    pCollectClient->SetCollectNow();    
  }

  return RC_S_OK;
}

//////////////////////////////////////////////////////////////////////////
// checkPoint
void AggregatorImpl::checkPoint(uint32_t now) {

  if (m_tcCheckPoint.GetIdleTime(now)) { return; }
  updateMapFileUpdate();

  AutoLock autoLock(m_lockModule);
  if (RC_S_OPEN != m_eModuleStatus) { return; }

  module_dynclib_t::iterator it_list, end;
  for (it_list = m_lstModule.begin(), end = m_lstModule.end(); it_list != end; ++it_list) {

    IModule* pIModule = (*it_list).module;
    if (NULL == pIModule || 0 == pIModule->NeedMoreThread()) { continue; }

    pIModule->CheckPoint();
  }

  // update checkpoint
  SetSetting(kStrCheckPoint, NULL);
}
//////////////////////////////////////////////////////////////////////////
// encode 
rc_t AggregatorImpl::initEncodeSystem() {

  return RC_S_OK;
}

void AggregatorImpl::clearEncodeSystem() { }
//////////////////////////////////////////////////////////////////////////
AggregatorImpl::CollectClient* AggregatorImpl::checkLogon(rc_t& rc, const char_t* strCollector, bool_t bSet) {

  CollectClient* pCollectClient = addClient(strCollector);
  if (NULL == pCollectClient) { rc = RC_S_FAILED; return NULL; }

  CollectorConfig& collectorConfig = pCollectClient->GetCollectorConfig();

  rc = collectorConfig.UpdateConfig(this, pCollectClient);
  if (RC_S_OK != rc) { return NULL; }

  if (TRUE == bSet) { pCollectClient->UpdateSessionID(); }
  rc = RC_S_OK;
  return pCollectClient;
}

//////////////////////////////////////////////////////////////////////////
IAggregator* IAggregator::CreateInstance() { return new AggregatorImpl(); }

// for C
IAggregator* Aggregator_CreateInstance() { return IAggregator::CreateInstance(); }

END_NAMESPACE_AGGREGATOR
