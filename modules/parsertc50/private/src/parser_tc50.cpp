/*

*/

#include "parser_tc50.h"

#include "logtype_tc50.h"

#include "protocol.h"
#include "getset_param.h"

#include "code_util.h"
#include "time_util.h"
#include "file_util.h"

#include "thread.h"

BEGIN_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////
USING_BASE(Thread);
USING_BASE(AutoLock);

USING_BASE(DayTime);
USING_BASE(micro_time);
USING_BASE(GetFileName);

USING_DATABASE(IRecordSet);
USING_DATABASE(IRecord);
USING_BASE(ChangeDirStype);
//////////////////////////////////////////////////////////////////////////
rc_t ParserTC50::Start() { 

  ASSERT(m_pIAggregator);

  AutoRelease<IFileIteratorBase*> autoRelFileIterator(
    m_pIAggregator->GetFileIterator(LogTypeTC50::StrLogType(), m_nwtGetFileIterator));

  if (NULL == autoRelFileIterator) { return RC_S_FAILED; }

  while (TRUE == m_bRunning) {
    // file iterate
    autoRelFileIterator->file_iterate(this);
  }

  return RC_S_OK;
}

void ParserTC50::Stop() {

  m_acThread = m_nRealThreadNum;
  m_bRunning = FALSE;
  wakeUpThread();
  waitThread();
}


//////////////////////////////////////////////////////////////////////////
// resource
const rc_t ParserTC50::GetName(char_t* strName, uint32_t* size) const {
  if (NULL == strName || NULL == size || 0 == (*size)) { return RC_S_NULL_VALUE; }
  STRCPY(strName, (*size), m_strName);
  return RC_S_OK;
}

const rc_t ParserTC50::GetStatus() const { return RC_S_OK; }

const rc_t ParserTC50::GetResourceSize(uint64_t* curr_size, uint64_t* free_size) const {

  if (NULL == curr_size || 0 == free_size) { return RC_S_NULL_VALUE; }

  (*curr_size) = 0x00;
  (*free_size) = 0x00;
  return RC_S_OK;
}

const rc_t ParserTC50::GetLastErr(uint32_t* err_no, char_t* err_msg, uint32_t* size) const {

  if (NULL == err_no || NULL == err_msg || NULL == size || 0 == (*size)) { return RC_S_NULL_VALUE; }

  (*err_no) = 0;
  STRCPY(err_msg, (*size), NULL_STR);
  return RC_S_OK;
}

const rc_t ParserTC50::GetInfo(char_t* info, uint32_t* size) const {

  if (NULL == info || NULL == size || 0 == (*size)) { return RC_S_NULL_VALUE; }

  uint32_t nCountReq = 0;
  uint32_t nCountAns = 0;
  uint32_t nCountExcept = 0;
  uint32_t nCountConnect = 0;
  uint32_t nCountDisConnect = 0;

  for (size_t idx = 0; idx < kMAX_THEARD_NUM; ++idx) {
    nCountReq += m_PT[idx].m_TC50Log.nCountReq;
    nCountAns += m_PT[idx].m_TC50Log.nCountAns;
    nCountExcept += m_PT[idx].m_TC50Log.nCountExcept;
    nCountConnect += m_PT[idx].m_TC50Log.nCountConnect;
    nCountDisConnect += m_PT[idx].m_TC50Log.nCountDisConnect;
  }

  (*size) = SNPRINTF(info, (*size), (*size), _STR("%s: \n"
    "Req Count=%u\n"
    "Ans Count=%u\n"
    "Except Count=%u\n"
    "Conn Count=%u\n"
    "DisConn Count=%u\n"
    )

    , STR_VERSION
    , nCountReq
    , nCountAns
    , nCountExcept
    , nCountConnect
    , nCountDisConnect    
    );
  return RC_S_OK;
}

const rc_t ParserTC50::GetLastAliveTime(uint32_t* alive) const {

  if (NULL == alive) { return RC_S_NULL_VALUE; }

  (*alive) = m_timeAlive;
  return RC_S_OK;
}

//////////////////////////////////////////////////////////////////////////
const char_t* ParserTC50::m_strName     = _STR("ParserTC50");
const uuid ParserTC50::m_gcUUID         = 0x00003012;

const char_t* kStrHisDBName             = _STR("tls_his");
const char_t* kStrHisDBEng              = _STR("BDE");
//////////////////////////////////////////////////////////////////////////
static const uint32_t kALLOC_SQL_MEM_SIZE = 128 * 1024;
static const uint32_t kMAX_SQL_MEM_SIZE = kALLOC_SQL_MEM_SIZE - 32 * 1024;

static const uint32_t kDefaultTFPreRows = 1024 * 1024;

//////////////////////////////////////////////////////////////////////////
ParserTC50::ParserTC50()
  : m_pIAggregator(NULL)
  , m_bRunning(TRUE)
  , m_nwtGetFileIterator(10) // 10s
  , m_nwtGetDBConnection(10) // 10s

  , m_nThreadNum(1)
  , m_nRealThreadNum(0)

  , m_queueFileNode()
  , m_lockQueue()
  , m_acThread(0)
  , m_pIFileIteratorBase(NULL)  

  , m_lockCheckPoint()
  , m_nTimeSchedule(10000)
  , m_bScheduleOnce(FALSE)
  , m_bScheduleEnable(FALSE)

  , m_bDBRemote(FALSE)

  , m_setDict()
  , m_autoRelIParseIX(NULL)

  , m_TC50Dict()

  , m_nCountFailed(0)
  , m_timeAlive(DayTime::now())
  , m_nTransformRows(kDefaultTFPreRows)
{

  // clear logrule
  MEMSET(m_arrLogRule, kLOG_UNFLAG, sizeof(m_arrLogRule));

#ifdef DEBUG_ON
  MEMSET(m_arrLogRule, kLOG_FLAG, sizeof(m_arrLogRule));
#endif

  // init
  for (size_t idx = 0; idx < kMAX_THEARD_NUM; ++idx) {
    m_PT[idx].m_TC50Log.SetFuncRule(m_arrLogRule);
  }

  STRCPY(m_strHisDBName, sizeof(m_strHisDBName), kStrHisDBName);

  STRCPY(m_strDBEng, sizeof(m_strDBEng), kStrHisDBEng);
}

ParserTC50::~ParserTC50() {

  clearFileMap();
}

rc_t ParserTC50::Init(IAggregatorBase* pIAggregator) {

  if (NULL == pIAggregator) { return RC_S_NULL_VALUE; }

  m_pIAggregator = pIAggregator;

  for (size_t idx = 0; idx < kMAX_THEARD_NUM; ++idx) {
    if (FALSE == m_PT[idx].IsValid()) { return RC_S_FAILED; }
  }

  m_autoRelIParseIX.Set(IParseIX::CreateInstance(kStrParseWtComm, NULL));
  if (NULL == m_autoRelIParseIX) { return RC_E_NOMEM; }

  if (RC_S_OK != getCollectorDict()) { return RC_S_FAILED; }

  if (RC_S_OK != m_TC50Dict.LoadDefaultDict(m_pIAggregator->GetSystemFullPath())) {
    static const char_t strTraceStart[] = _STR("ParserTC50 LoadDefaultDict Failed");
    const uint32_t nStrTraceStartLen = sizeof(strTraceStart) - 1;
    logDB(ILogWriter::LV_INFO, m_strName, (uint8_t*)strTraceStart, nStrTraceStartLen, NULL, 0);
  }

  //
  clearFileMap();

  loadConfig();
  //m_acThread = m_nThreadNum;

  if (RC_S_OK != loadParseFile()) { return RC_S_FAILED; }
  if (RC_S_OK != loadLogRule()) { return RC_S_FAILED; }

  m_bRunning = TRUE;
  
  {
    static const char_t strTraceStart[] = _STR("ParserTC50 is working." STR_VERSION);
    const uint32_t nStrTraceStartLen = sizeof(strTraceStart) - 1;
    logDB(ILogWriter::LV_INFO, m_strName, (uint8_t*)strTraceStart, nStrTraceStartLen, NULL, 0);
  }
  return RC_S_OK;
}

//////////////////////////////////////////////////////////////////////////
rc_t ParserTC50::Begin(IFileIteratorBase* pIFileIteratorBase) {

  // clear queue
  ASSERT(m_queueFileNode.empty());
  ASSERT(NULL == m_pIFileIteratorBase);

  {
    // clear
    file_map_t::iterator it_map, end;
    for (it_map = m_mapFile.begin(), end = m_mapFile.end(); it_map != end; ++it_map) {
      it_map->second.SetDispatch(FALSE);
    }
  }

  m_lockCheckPoint.Acquire();

  m_pIFileIteratorBase = pIFileIteratorBase;
  /*
  m_nSQLSize = 0;
  if (NULL == m_autoRelIDBConn) { m_autoRelIDBConn.Set((IConnection*)(getDBConn())); }
  */
  return RC_S_OK;
}

rc_t ParserTC50::End() {

  m_acThread = m_nThreadNum;

  // wake up
  wakeUpThread();
  // wait
  waitThread();

  m_lockCheckPoint.Release();

  m_pIFileIteratorBase = NULL;
  if (TRUE == m_bRunning) { m_acThread = m_nThreadNum; }

  return RC_S_OK;
}

rc_t ParserTC50::FileProc(IFileIteratorBase* pIFileIterator, const IFileNode* pIFileNode) {

  if (NULL == pIFileIterator || NULL == pIFileNode) { return RC_S_OK; }

  if (FALSE == m_bRunning) { return RC_S_FAILED; }

  file_id_t file_id = pIFileNode->ID();

  file_map_t::iterator it_map = m_mapFile.find(file_id);
  if (m_mapFile.end() == it_map) {

    // new file
    AddToFileMap(file_id, 0);
    it_map = m_mapFile.find(file_id);
  }
/*
  if (NULL == m_autoRelIDBConn) {
    m_autoRelIDBConn.Set((IConnection*)(getDBConn()));

    if (NULL == m_autoRelIDBConn) { return RC_S_OK; }  
  }
*/
  LogFileNode& logFileNode = it_map->second;

  //if (TRUE == logFileNode.GetDispatch() || pIFileNode->Size() <= logFileNode.GetPos()) { return RC_S_OK; }
  if (TRUE == logFileNode.GetDispatch()) { return RC_S_OK; }

  // Parse  
  const char_t* strRemotePath = pIFileNode->RemotePath();
  if (NULL == strRemotePath) { return RC_S_OK; }

  logFileNode.SetDispatch(TRUE);
 
  // dict file
  //file_size_t nOldPos = logFileNode.GetPos();
  if (STRSTR(strRemotePath, sizeof(strRemotePath), _STR(".sto"))) {
    logParse_tc50_dict(pIFileIterator, logFileNode, pIFileNode);
  }
  else {
    //logParse_tc50(pIFileNode, pIFileIterator, logFileNode);
    logFileNode.SetFileNode(pIFileNode);

    logFileNode.SetHasError(FALSE);
    pushQueueFileNode(&logFileNode);
  }
/*
  if (nOldPos != logFileNode.GetPos()) {
    updateParseFile(pIFileNode->ID(), logFileNode.GetPos());
  }

  m_timeAlive = DayTime::now();
  if (TRUE == isFileOver(file_id)) { return RC_S_CLOSED; }
*/
  return RC_S_OK;
}

ParserTC50::LogFileNode* ParserTC50::popQueueFileNode() {

  AutoLock autoLock(m_lockQueue);
  if (m_queueFileNode.empty()) { return NULL; }

  LogFileNode* pLogFileNode = m_queueFileNode.front();
  m_queueFileNode.pop();
  return pLogFileNode;
}

void  ParserTC50::pushQueueFileNode(ParserTC50::LogFileNode* pLogFileNode) {

  if (NULL == pLogFileNode) { return; }

  AutoLock autoLock(m_lockQueue);
  m_queueFileNode.push(pLogFileNode);
}

void ParserTC50::ThreadProc(uint32_t idx) {

  ++m_nRealThreadNum;
  ++m_acThread;
  ParserThread& PT = m_PT[idx];

  // postion
  struct LogFileNodePos {
    LogFileNode* pLogFileNode;
    file_size_t nSavePos;
  };
  typedef std::list<LogFileNodePos> log_file_list_t;
  log_file_list_t listLogFile;

  while (TRUE == m_bRunning) {
    PT.m_ev.Wait();
    listLogFile.clear();

    PT.m_autoRelIDBConn.Set((IConnection*)(getDBConn()));
    if (NULL == PT.m_autoRelIDBConn) { continue; }

    ASSERT(0 == PT.m_nSQLSize);
    // get 
    LogFileNode* pLogFileNode = NULL;
    while (NULL != (pLogFileNode = popQueueFileNode())) {

      const IFileNode* pIFileNode = pLogFileNode->GetFileNode();
      if (NULL == pIFileNode || NULL == m_pIFileIteratorBase) { continue; }

      file_size_t nOldPos = pLogFileNode->GetPos();
      logParse_tc50(idx, pIFileNode, m_pIFileIteratorBase, *pLogFileNode);
      if (nOldPos != pLogFileNode->GetPos()) {
        updateParseFile(idx, pIFileNode->ID(), pLogFileNode->GetPos());

        // save postion
        listLogFile.push_back(LogFileNodePos());
        LogFileNodePos& lfnp = listLogFile.back();
        lfnp.pLogFileNode = pLogFileNode;
        lfnp.nSavePos = nOldPos;
      }

      pLogFileNode->SetFileNode(NULL);
    }

    // 
    if (PT.m_nSQLSize) {

      // 
      rc_t rc = execSQL(idx, PT.m_autoRelMemSQL);
      if (RC_S_OK != rc && RC_E_DB_QUERY != rc) {

        // restore file pos
        log_file_list_t::iterator it_list, end;
        for (it_list = listLogFile.begin(), end = listLogFile.end(); it_list != end; ++it_list) {

          LogFileNodePos& lfnp = (*it_list);
          if (NULL == lfnp.pLogFileNode) { continue; }
          lfnp.pLogFileNode->SetPos(lfnp.nSavePos);
        }
      }
      PT.m_autoRelMemSQL[0] = 0x00;
      PT.m_nSQLSize = 0x00;
    }

    PT.m_autoRelIDBConn.Set(NULL);
    --m_acThread;
  }
}

static const uint32_t kMAX_TC50_LOG_SIZE  = 64 * 1024;
static const uint32_t kMAX_COPY_BUF_SIZE  = 8 * 1024;

rc_t ParserTC50::logParse_tc50(uint32_t idx
                               ,const IFileNode* pIFileNode, IFileIteratorBase* pIFileIterator, LogFileNode& logFileNode) {

  //
  const char_t* strCollector = pIFileNode->Collector();
  file_id_t file_id = pIFileNode->ID();
  
  char_t strFileDir[kMAX_PATH] = {0};
  const char_t* strRemotePath = pIFileNode->RemotePath();
  const char_t* strFileName = GetFileName(strRemotePath);
  if (NULL == strFileName) { return RC_S_OK; }
  STRNCPY(strFileDir, kMAX_PATH, strRemotePath, strFileName - strRemotePath - sizeof(CHAR_DIR_SEP));

  uint32_t nRealLogLen = 0;
  uint32_t nRealPos = 0;
  // get data
  file_size_t pos = logFileNode.GetPos();
  AutoRelease<IMemoryNode*> autoRelMemNode(pIFileIterator->GetFileData(file_id, &pos));
  if (NULL == autoRelMemNode || NULL == autoRelMemNode->data() || 0 == autoRelMemNode->len()) { return RC_S_NULL_VALUE; }

  ASSERT(pos <= logFileNode.GetPos());
  if (autoRelMemNode->len() <= (uint32_t)(logFileNode.GetPos() - pos)) { return RC_S_NULL_VALUE; }
  nRealLogLen = autoRelMemNode->len() - (uint32_t)(logFileNode.GetPos() - pos);
  nRealPos = autoRelMemNode->len() - nRealLogLen;

  char_t strBuffer[kMAX_COPY_BUF_SIZE + 1] = {0};
  const char_t* strLogData = (const char_t*)autoRelMemNode->data() + nRealPos;
  uint32_t nLogPos = 0;
  uint32_t nLogLen = nRealLogLen;
  uint32_t nLogSize = 0;
  bool_t bCopy = FALSE;
  uint32_t nCopySize = 0;
  file_size_t nCurrentFileParsePos;

  ParserThread& PT = m_PT[idx];
  TC50Dict* pTC50Dict = &m_TC50Dict;
  TC50Log* pTC50Log   = &PT.m_TC50Log;

  ASSERT(pTC50Dict);
  ASSERT(pTC50Log);

  pTC50Log->nDate = logFileNode.m_nDate;

  do {
    nCurrentFileParsePos = logFileNode.GetPos();

    // have one line
    const void* strFindEnd = MEMCHR((const void*)strLogData, '\n', nLogLen);
    if (NULL == strFindEnd) {
      const uint32_t kMAX_FAILED_COUNT = 16;
      ++logFileNode.m_nFailedCount;
      if (kMAX_FAILED_COUNT < logFileNode.m_nFailedCount || nLogLen > kMAX_TC50_LOG_SIZE) {

        setFailedLog(idx, logFileNode.m_nDate, strCollector, file_id, nCurrentFileParsePos + nLogPos, nCurrentFileParsePos + nLogPos + nLogLen
          , (const uint8_t*)strLogData, nLogLen);

        logFileNode.UpdatePos(nLogLen);
        nLogPos += nLogLen;
      }
    }
    else {

      logFileNode.m_nFailedCount = 0;
      do {

        nLogSize = nLogLen - nLogPos;
        if (RC_S_OK != pTC50Log->Parse(strLogData + nLogPos, &nLogSize
          , pTC50Dict, pIFileIterator, strCollector, strFileDir)) {

          // is 0x00 
          // save null data log
          size_t nNullLogLen = STRNLEN(strLogData + nLogPos, nLogLen - nLogPos);
          if (nLogSize <= nNullLogLen) { break; }
          
          if (nNullLogLen < kMAX_COPY_BUF_SIZE) {
            
            char_t strNullLog[kMAX_COPY_BUF_SIZE + 8] = {0};
            size_t nNullLogSize = SNPRINTF(strNullLog, kMAX_COPY_BUF_SIZE, kMAX_COPY_BUF_SIZE
              , "%s\r\n", strLogData + nLogPos);

            pTC50Log->Parse(strNullLog, &nNullLogSize, pTC50Dict, pIFileIterator, strCollector, strFileDir);
          }

          nLogSize = (uint32_t)nNullLogLen + 1;

          // skip all zero
          do 
          {
            uint32_t ch = *(strLogData + nLogPos + nLogSize);
            if (0x00 == ch || 0x20 == ch || '\r' == ch || '\n' == ch) { ++nLogSize; }
            else { break; }
          } while(nLogPos + nLogSize < nLogLen);
          
        }

        // new log
        if (TC50Log::PRET_UNKNOW == pTC50Log->eLogType || TRUE == didMathLogRule(pTC50Log->nFuncID)) {
          pTC50Log->FixData();
          if (RC_S_FAILED == writeLog(idx, strCollector, file_id, nCurrentFileParsePos + nLogPos, nLogSize, pTC50Log)) {
            //logFileNode.UpdatePos(nLogSize);
            //return RC_S_OK;
            return RC_S_FAILED;
          }
        }

        // update 
        logFileNode.UpdatePos(nLogSize);
        nLogPos += nLogSize;
      } while (nLogPos < nLogLen);
    }

    if (FALSE == bCopy) {
      
      // save old data
      nLogSize = nLogLen - nLogPos;
      if (nLogSize > kMAX_COPY_BUF_SIZE) { return RC_S_FAILED; }
      MEMCPY(strBuffer, sizeof(strBuffer), strLogData + nLogPos, nLogSize);

      // get more data
      pos = logFileNode.GetPos() + nLogSize;
      autoRelMemNode.Set(pIFileIterator->GetFileData(file_id, &pos));
      if (NULL == autoRelMemNode || NULL == autoRelMemNode->data() || 0 == autoRelMemNode->len()) { return RC_S_NULL_VALUE; }

      ASSERT(pos <= logFileNode.GetPos() + nLogSize);
      nRealLogLen = autoRelMemNode->len() - (uint32_t)(logFileNode.GetPos() + nLogSize - pos);
      if (nRealLogLen > kMAX_COPY_BUF_SIZE - nLogSize) {

        nCopySize = kMAX_COPY_BUF_SIZE - nLogSize;
        nRealPos = autoRelMemNode->len() - nRealLogLen;
        MEMCPY(strBuffer + nLogSize, sizeof(strBuffer), autoRelMemNode->data() + nRealPos, nCopySize);
        nLogLen = kMAX_COPY_BUF_SIZE;
        bCopy = TRUE;
      }
      else {
        nCopySize = nRealLogLen;

        nRealPos = autoRelMemNode->len() - nRealLogLen;
        MEMCPY(strBuffer + nLogSize, sizeof(strBuffer), autoRelMemNode->data() + nRealPos, nCopySize);
        nLogLen = nLogSize + nCopySize;
      }

      strBuffer[nLogSize + nCopySize] = 0x00;
      strLogData = strBuffer;
    }
    else {

      // old data
      if (nLogLen - nLogPos > nCopySize) { return RC_S_FAILED; }
      nLogSize = nLogLen - nLogPos;
      ASSERT(autoRelMemNode);
      strLogData = (const char_t*)autoRelMemNode->data() + nRealPos + (nCopySize - nLogSize);
      nLogLen = autoRelMemNode->len() - nRealPos - (nCopySize - nLogSize);
      bCopy = FALSE;
    }
    nLogPos = 0;

  } while (nLogLen && autoRelMemNode && TRUE == m_bRunning);

  return RC_S_OK;
}

//////////////////////////////////////////////////////////////////////////
// dict
rc_t ParserTC50::logParse_tc50_dict(IFileIteratorBase* /*pIFileIterator*/, LogFileNode& logFileNode
                                    , const IFileNode* pIFileNode) {

  char_t strFileDir[kMAX_PATH] = {0};
  const char_t* strRemotePath = pIFileNode->RemotePath();
  const char_t* strFileName = GetFileName(strRemotePath);
  if (NULL == strFileName) { return RC_S_OK; }
  STRNCPY(strFileDir, kMAX_PATH, strRemotePath, strFileName - strRemotePath - sizeof(CHAR_DIR_SEP));
  const char_t* strDir = strFileDir;

  uint64_t file_create_time = micro_time::to_second(pIFileNode->CreateTime());
  file_id_t file_id = pIFileNode->ID();
  AddDict(pIFileNode->Collector(), strDir, file_id, file_create_time);
  // update pos
  logFileNode.SetPos(pIFileNode->Size());
  return RC_S_OK;
}

bool_t ParserTC50::isFileOver(file_id_t file_id) {

  UNUSED_PARAM(file_id);

  return FALSE;
}

void ParserTC50::clearFileMap() {

  m_mapFile.clear();
}

rc_t ParserTC50::AddToFileMap(file_id_t file_id, file_size_t pos) {

  //
  if(m_mapFile.end() != m_mapFile.find(file_id)) { return RC_S_DUPLICATE; }

  LogFileNode logFileNode(pos);
  m_mapFile[file_id] = logFileNode;
  return RC_S_OK;
}

//////////////////////////////////////////////////////////////////////////
// 
struct ParseFileReulst {

  ParserTC50*     pParserTC50;
  file_id_t       last_file_id;

  ParseFileReulst(ParserTC50* pParserTC50)
    : pParserTC50(pParserTC50)
    , last_file_id(0)
  {}
};

typedef GetSetParamImpl<ParseFileReulst*>         ParseFileReulstGSObject;
template<>
void ParseFileReulstGSObject::SetResult(uint32_t row, uint32_t col, const char_t* val, uint32_t len) {

  if (NULL == val || 0 == len) { return; }
  ASSERT(m_r);
  UNUSED_PARAM(row);
  
  switch(col) {
    case 0: { m_r->last_file_id = ATOI(val); break; }
    case 1: {
      file_size_t pos = ATOI64(val);
      if (m_r->last_file_id) { m_r->pParserTC50->AddToFileMap(m_r->last_file_id, pos); }
      m_r->last_file_id = 0;
      break;
    }
  }
}

rc_t ParserTC50::loadParseFile() {

  ASSERT(m_pIAggregator);

  ParseFileReulst parseFileReulst(this);
  // get aggregator_dir
  ParseFileReulstGSObject parseFileReulstGSObject(&parseFileReulst, m_strName);

  return m_pIAggregator->GetSetting(&parseFileReulstGSObject, kStrGetModuleSetting);
}

// 
// struct LogRuleReulst {
// 
//   ParserTC50*     pParserTC50;
// }

typedef GetSetParamImpl<ParserTC50*>         LogRuleReulstGSObject;
template<>
void LogRuleReulstGSObject::SetResult(uint32_t row, uint32_t col, const char_t* val, uint32_t len) {

  if (NULL == val || 0 == len) { return; }
  ASSERT(m_r);
  UNUSED_PARAM(row);

  if (0 == col) { m_r->SetLogRule(ATOI(val)); }
}

rc_t ParserTC50::loadLogRule() {

  ASSERT(m_pIAggregator);

  LogRuleReulstGSObject logRuleReulstGSObject(this);

  return m_pIAggregator->GetSetting(&logRuleReulstGSObject, kStrGetTC50LogRule);
}


struct DictFileReulst {

  ParserTC50*     pParserTC50;
  file_id_t       last_file_id;
  const char_t*   strCollector;
  const char_t*   strDir;

  DictFileReulst(ParserTC50* pParserTC50)
    : pParserTC50(pParserTC50)
    , last_file_id(0)
    , strCollector(NULL)
    , strDir(NULL)
  {}
};

typedef GetSetParamImpl<DictFileReulst*>         DictFileReulstGSObject;
template<>
void DictFileReulstGSObject::SetResult(uint32_t row, uint32_t col, const char_t* val, uint32_t len) {

  if (NULL == val || 0 == len) { return; }
  ASSERT(m_r);
  UNUSED_PARAM(row);

  switch(col) {
    case 0: { m_r->last_file_id = ATOI(val); break; }
    case 1: { m_r->strCollector = val; break; }
    case 2: { m_r->strDir = val; break; }
    case 3: { 
      if (m_r->last_file_id) { 
        uint64_t create_time = micro_time::to_second(ATOI64(val));
        m_r->pParserTC50->AddDict(m_r->strCollector, m_r->strDir, m_r->last_file_id, create_time);
      }
      m_r->last_file_id = 0;
      break;
    }
  } // switch
}

static const char_t* strDictRegEx = "[0-9a-zA-Z.]*.sto";
rc_t ParserTC50::getCollectorDict() {

  ASSERT(m_pIAggregator);

  DictFileReulst dictFileReulst(this);
  DictFileReulstGSObject dictFileReulstGSObject(&dictFileReulst, NULL, 0, strDictRegEx);
  return m_pIAggregator->GetSetting(&dictFileReulstGSObject, kStrCollectorFile);
}


struct ReadSettingReulst {

  ParserTC50*     pParserTC50;
  uint32_t        nCurrRow;
  const char_t*   pCurrSection;
  const char_t*   pCurrParam;  

  ReadSettingReulst(ParserTC50* pParserTC50)
    : pParserTC50(pParserTC50)
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

      if (0 == STRCMP(m_r->pParserTC50->GetName(), m_r->pCurrSection)) {

        if (0 == STRCMP(_STR("ThreadNum"), m_r->pCurrParam)) { 
          m_r->pParserTC50->SetThreadNum(ATOI(val)); return; }

        if (0 == STRCMP(_STR("ScheduleTime"), m_r->pCurrParam)) { 
          m_r->pParserTC50->SetScheduleTime(ATOI(val)); return; }

        if (0 == STRCMP(_STR("TransformRows"), m_r->pCurrParam)) { 
          m_r->pParserTC50->SetTransformRows(ATOI(val)); return; }

        if (0 == STRCMP(_STR("ScheduleEnable"), m_r->pCurrParam)) { 
          m_r->pParserTC50->SetScheduleEnable('Y' == *val ? TRUE : FALSE); return; }

        if (0 == STRCMP(_STR("HisDBName"), m_r->pCurrParam)) { 
          m_r->pParserTC50->SetHisDBName(val); return; }

        if (0 == STRCMP(_STR("HisDBRemote"), m_r->pCurrParam)) { 
          m_r->pParserTC50->SetHisDBRemote('Y' == *val ? TRUE : FALSE); return; }

        if (0 == STRCMP(_STR("HisDBEng"), m_r->pCurrParam)) { 
          m_r->pParserTC50->SetHisDBEng(val); return; }
        
      }
    }
  }
}

rc_t ParserTC50::loadConfig() {

  ASSERT(m_pIAggregator);
  ReadSettingReulst readSettingReulst(this);
  ReadSettingGSObject readSettingGSObject(&readSettingReulst);
  return m_pIAggregator->GetSetting(&readSettingGSObject, kStrSystemSetting);
}

//////////////////////////////////////////////////////////////////////////
// sql
static const uint32_t kMAX_SQL_LEN      = 32 * 1024;

IDBConnection* ParserTC50::getDBConn() {

  ASSERT(m_pIAggregator);
  return m_pIAggregator->GetDBConnection(/*"maindb"*/NULL_STR, m_nwtGetDBConnection);
}

rc_t ParserTC50::execSQL(uint32_t idx, const uint8_t* strSQLData) {

  ASSERT(strSQLData);
  const char_t* strSQL = (const char_t*)strSQLData;

  if (0 >= STRLEN(strSQL)) { return RC_S_NULL_VALUE; }

  ParserThread& PT = m_PT[idx];
  ASSERT(PT.m_autoRelIDBConn);

  return PT.m_autoRelIDBConn->execute(strSQL, FALSE);
}

rc_t ParserTC50::logDB(ILogWriter::log_lv_t lv, const char_t* from
           , const uint8_t* data, uint32_t dlen
           , const uint8_t* content, uint32_t clen) {

  //
  if (lv > ILogWriter::LV_TRACE) { return RC_E_ACCESS; }
  NormalSetGSObject logDBGSObject(NULL, kStrLOG_LV[lv], 0, from, 0
    , (const char_t*)data, dlen, (const char_t*)content, clen);
  return m_pIAggregator->SetSetting(kStrCollectLog, &logDBGSObject);
}

static const char_t* strSetParseFileFMT = _STR("call sp_module_set_setting('%s', '%u', %I64u, '', @err_no, @err_msg);");

rc_t ParserTC50::updateParseFile(uint32_t idx, file_id_t file_id, file_size_t pos) {

  // size
  //char_t strSQL[kMAX_SQL_LEN + 1] = {0};
  ParserThread& PT = m_PT[idx];

  if (PT.m_nSQLSize > kMAX_SQL_MEM_SIZE) {
    // exec sql
    //updateParseFile(idx, file_id, s_pos + e_pos);
    if (RC_S_OK != execSQL(idx, PT.m_autoRelMemSQL)) { return RC_S_FAILED; }

    PT.m_autoRelMemSQL[0] = 0x00;
    PT.m_nSQLSize = 0x00;
  }

  char_t* strSQL = (char_t*)((uint8_t*)(PT.m_autoRelMemSQL)) + PT.m_nSQLSize;
  size_t nFreeSize = kALLOC_SQL_MEM_SIZE - PT.m_nSQLSize;
  int nLen = SNPRINTF(strSQL, nFreeSize, nFreeSize, strSetParseFileFMT, m_strName, file_id, pos);
  PT.m_nSQLSize += nLen;
  return RC_S_OK;
  //return execSQL(strSQL);
}
/*
static const char_t* strSetFailedLogFMT_Head = _STR("call sp_tc50_set_failed_log('%s', '%s', %u, %I64u, %u, x'");
static const char_t* strSetFailedLogFMT_Tail = _STR("', @err_no, @err_msg);select @err_no, @err_msg;");
*/

static const char_t* strSetFailedFMT = _STR("call sp_tc50_set_failed_log(%d"
  ",'%s','%s',%u,%I64u,%u"
  ",@err_no,@err_msg);");

rc_t ParserTC50::setFailedLog(uint32_t idx, uint32_t log_date, const char_t* strCollector
                              , file_id_t file_id, file_size_t file_pos_start, file_size_t file_pos_end
                              , const uint8_t* data, uint32_t len) {

  char_t strSQL[kMAX_SQL_LEN + 1] = {0};
  UNUSED_PARAM(data);
  UNUSED_PARAM(len);
  /*
  int nLen = SNPRINTF(strSQL, kMAX_SQL_LEN, kMAX_SQL_LEN, strSetFailedLogFMT_Head
    , strCollector, file_id, file_pos_start, file_pos_end);

  MEMCPY(strSQL + nLen, kMAX_SQL_LEN - nLen, data, len);
  ToBase16((uint8_t*)(strSQL + nLen), len);

  STRCAT(strSQL + nLen + 2*len, kMAX_SQL_LEN - (nLen + 2*len), strSetFailedLogFMT_Tail);
  */
  SNPRINTF(strSQL, kMAX_SQL_LEN, kMAX_SQL_LEN, strSetFailedFMT
    , log_date, m_strName, strCollector, file_id, file_pos_start, file_pos_end);

  ++m_nCountFailed;
  return execSQL(idx, (const uint8_t*)strSQL);
}

//////////////////////////////////////////////////////////////////////////

// trade
static const char_t* strSetTradeReqFMT = _STR("call sp_tc50_set_trade_req(%d"
  ",'%s','%s',%u,%I64u,%u"
  ",'%s',%u,'%s','%s','%s',%u,%u,%u,%u,'%s',%u,'%s'"

    ",'%s','%s'"
    ",%u,'%s','%s',%u"
    ",'%s',%u,'%s',%u,%u,%u,'%s',%u,%u"
    ",%u"
    ",'%s','%s','%s'"

  ",@err_no,@err_msg);");

static const char_t* strSetTradeAnsFMT = _STR("call sp_tc50_set_trade_ans(%d"
  ",'%s','%s',%u,%I64u,%u"
  ",'%s',%u,'%s','%s','%s',%u,%u,%u,%u,'%s',%u,'%s'"

    ",%u,%u,%u,%d,'%s'"
    ",'%s','%s','%s'"
    ",'%s','%s','%s'"

  ",@err_no,@err_msg);");

// logon
static const char_t* strSetLogonReqFMT = _STR("call sp_tc50_set_logon_req(%d"
  ",'%s','%s',%u,%I64u,%u"
  ",'%s',%u,'%s','%s','%s',%u,%u,%u,%u,'%s',%u,'%s'"

    ",'%s','%s','%s'"
    ",%u,'%s','%s','%s'"
    ",%u,'%s',%u,'%s'"
    ",'%s','%s','%s'"

  ",@err_no,@err_msg);");

static const char_t* strSetLogonAnsFMT = _STR("call sp_tc50_set_logon_ans(%d"
  ",'%s','%s',%u,%I64u,%u"
  ",'%s',%u,'%s','%s','%s',%u,%u,%u,%u,'%s',%u,'%s'"
  
    ",%u,%u,%u,%d,'%s'"
    ",'%s','%s','%s'"
    ",'%s','%s','%s'"

  ",@err_no,@err_msg);");

// sc
static const char_t* strSetSCReqFMT = _STR("call sp_tc50_set_sc_req(%d"
  ",'%s','%s',%u,%I64u,%u"
  ",'%s',%u,'%s','%s','%s',%u,%u,%u,%u,'%s',%u,'%s'"

    ",'%s','%s','%s','%s','%s'"
    ",'%s','%s','%s','%s','%s'"
    ",'%s','%s','%s','%s','%s'"
    ",'%s','%s','%s','%s','%s'"
    ",'%s','%s'"
    ",'%s','%s','%s'"

  ",@err_no,@err_msg);");

static const char_t* strSetSCAnsFMT = _STR("call sp_tc50_set_sc_ans(%d"
  ",'%s','%s',%u,%I64u,%u"
  ",'%s',%u,'%s','%s','%s',%u,%u,%u,%u,'%s',%u,'%s'"

    ",%u,%u,%u,%d,'%s'"
    ",'%s','%s','%s'"
    ",'%s','%s','%s'"

  ",@err_no,@err_msg);");

static const char_t* strSetInfoFMT = _STR("call sp_tc50_set_info_log(%d"
  ",'%s','%s',%u,%I64u,%u"
  ",'%s',%u"
  
    ",'%s','%u','%s','%s','%s','%s'"

  ",@err_no,@err_msg);");

rc_t ParserTC50::writeLog(uint32_t idx, const char_t* strCollector, file_id_t file_id, file_size_t s_pos, uint32_t e_pos, TC50Log* pTC50Log) {

  ParserThread& PT = m_PT[idx];

  ASSERT(pTC50Log);
  ASSERT(PT.m_autoRelMemSQL);

  if (PT.m_nSQLSize > kMAX_SQL_MEM_SIZE) {
    // exec sql
    //updateParseFile(idx, file_id, s_pos + e_pos);
    if (RC_S_OK != execSQL(idx, PT.m_autoRelMemSQL)) { return RC_S_FAILED; }
    PT.m_autoRelMemSQL[0] = 0x00;
    PT.m_nSQLSize = 0x00;
  }

  char_t* strSQL = (char_t*)((uint8_t*)(PT.m_autoRelMemSQL)) + PT.m_nSQLSize;
  int nLen = 0;
  size_t nFreeSize = kALLOC_SQL_MEM_SIZE - PT.m_nSQLSize;
  
  if (TC50Log::PRET_REQ == pTC50Log->eLogType) {

    if (TC50Log::DICT_SIMPLE == pTC50Log->eDictType) {

      nLen = SNPRINTF(strSQL, nFreeSize, nFreeSize, strSetLogonReqFMT
        , pTC50Log->nDate
        , m_strName, strCollector, file_id, s_pos, e_pos
        , pTC50Log->strLogType
        , pTC50Log->nTime
        , pTC50Log->strIP
        , pTC50Log->strMAC
        , pTC50Log->strThreadID
        , pTC50Log->nChannelID
        , pTC50Log->nTransID
        , pTC50Log->nReqType, pTC50Log->nFuncID
        , pTC50Log->strFuncName
        , pTC50Log->nBranchID
        , pTC50Log->strBranchName

        //, pTC50Log->strLogData

        , pTC50Log->strXT_GTLB
        , pTC50Log->strKHH
        , pTC50Log->strKHMC

        , pTC50Log->nZHLB
        , pTC50Log->strZJZH
        , pTC50Log->strSHGD
        , pTC50Log->strSZGD

        , pTC50Log->nXT_CLITYPE
        , pTC50Log->strXT_CLIVER
        , pTC50Log->nXT_VIPFLAG
        , pTC50Log->strXT_MACHINEINFO

        , pTC50Log->strReserve_a
        , pTC50Log->strReserve_b
        , pTC50Log->strReserve_c
        );
    }
    else if (TC50Log::DICT_COMMON == pTC50Log->eDictType) {

      nLen = SNPRINTF(strSQL, nFreeSize, nFreeSize, strSetTradeReqFMT
        , pTC50Log->nDate
        , m_strName, strCollector, file_id, s_pos, e_pos
        , pTC50Log->strLogType
        , pTC50Log->nTime
        , pTC50Log->strIP
        , pTC50Log->strMAC
        , pTC50Log->strThreadID
        , pTC50Log->nChannelID
        , pTC50Log->nTransID
        , pTC50Log->nReqType, pTC50Log->nFuncID
        , pTC50Log->strFuncName
        , pTC50Log->nBranchID
        , pTC50Log->strBranchName

        , pTC50Log->strKHH
        , pTC50Log->strKHMC

        , pTC50Log->nZHLB
        , pTC50Log->strZJZH        
        , pTC50Log->strGDDM
        , pTC50Log->nOP_WTFS

        , pTC50Log->strWTBH
        , pTC50Log->nWTFS

        , pTC50Log->strZQDM
        , pTC50Log->nMMBZ
        , pTC50Log->nJYDW
        , pTC50Log->nWTSL
        , pTC50Log->strWTJG        
        , pTC50Log->nWTRQ
        , pTC50Log->nWTSJ

        , pTC50Log->nXT_CHECKRISKFLAG

        , pTC50Log->strReserve_a
        , pTC50Log->strReserve_b
        , pTC50Log->strReserve_c
        );
    }
    else if (TC50Log::DICT_SCNTR == pTC50Log->eDictType) {
      nLen = SNPRINTF(strSQL, nFreeSize, nFreeSize, strSetSCReqFMT
        , pTC50Log->nDate
        , m_strName, strCollector, file_id, s_pos, e_pos
        , pTC50Log->strLogType
        , pTC50Log->nTime
        , pTC50Log->strIP
        , pTC50Log->strMAC
        , pTC50Log->strThreadID
        , pTC50Log->nChannelID
        , pTC50Log->nTransID
        , pTC50Log->nReqType, pTC50Log->nFuncID
        , pTC50Log->strFuncName
        , pTC50Log->nBranchID
        , pTC50Log->strBranchName

        //, pTC50Log->strLogData

        , pTC50Log->strXT_GTLB
        , pTC50Log->strCA_KHH
        , pTC50Log->strCA_KHMC
        , pTC50Log->strCA_VER
        , pTC50Log->strCA_AQJB
        , pTC50Log->strCA_TXMM
        , pTC50Log->strCA_ISVIPHOST
        , pTC50Log->strCA_JQTZM
        , pTC50Log->strCA_SLOTSN
        , pTC50Log->strCA_CID

        , pTC50Log->strCA_CERTREQ
        , pTC50Log->strCA_USERCERDN
        , pTC50Log->strCA_ZSQSRQ
        , pTC50Log->strCA_ZSJZRQ
        , pTC50Log->strCA_CERTSN
        , pTC50Log->strCA_CERTINFO
        , pTC50Log->strCA_MACHINENAME

        , pTC50Log->strCA_DLSJ
        , pTC50Log->strCA_LASTIP
        , pTC50Log->strCA_MAC
        , pTC50Log->strCA_CSCS
        , pTC50Log->strCA_RESV

        , pTC50Log->strReserve_a
        , pTC50Log->strReserve_b
        , pTC50Log->strReserve_c
        );
    }
    else { return RC_E_UNSUPPORTED; }
    /*
    if (0 >= nLen) { return RC_S_FAILED; }
    return execSQL(strSQL);
    */
  }
  else if (TC50Log::PRET_SUCESS == pTC50Log->eLogType
    ||TC50Log::PRET_DEAERR == pTC50Log->eLogType
    || TC50Log::PRET_POLERR == pTC50Log->eLogType
    || TC50Log::PRET_FAILD == pTC50Log->eLogType
  ) {

    const char_t* strFMT = NULL;
    if (TC50Log::DICT_SIMPLE == pTC50Log->eDictType) { strFMT = strSetLogonAnsFMT; }
    else if (TC50Log::DICT_COMMON == pTC50Log->eDictType) { strFMT = strSetTradeAnsFMT; }
    else if (TC50Log::DICT_SCNTR == pTC50Log->eDictType) { strFMT = strSetSCAnsFMT; }
    else { return RC_E_UNSUPPORTED; }

    nLen = SNPRINTF(strSQL, nFreeSize, nFreeSize, strFMT
      , pTC50Log->nDate
      , m_strName, strCollector, file_id, s_pos, e_pos
      , pTC50Log->strLogType
      , pTC50Log->nTime
      , pTC50Log->strIP
      , pTC50Log->strMAC
      , pTC50Log->strThreadID
      , pTC50Log->nChannelID
      , pTC50Log->nTransID
      , pTC50Log->nReqType, pTC50Log->nFuncID
      , pTC50Log->strFuncName
      , pTC50Log->nBranchID
      , pTC50Log->strBranchName

      //, pTC50Log->strLogData

      , pTC50Log->nTimeA
      , pTC50Log->nTimeB
      , pTC50Log->nTimeA
      , pTC50Log->nReturnNO
      , pTC50Log->strReturnMsg

      , pTC50Log->strWTBH
      , pTC50Log->strXT_CHECKRISKFLAG
      , pTC50Log->strRETINFO

      , pTC50Log->strZJYE
      , pTC50Log->strZQSL
      , pTC50Log->strKMSL
      );
/*
    if (0 >= nLen) { return RC_S_FAILED; }
    return execSQL(strSQL);
*/
  }

  else if (TC50Log::PRET_SYS_INFO == pTC50Log->eLogType
    || TC50Log::PRET_SYS_ERROR == pTC50Log->eLogType
    || TC50Log::PRET_CONN_CON == pTC50Log->eLogType
    || TC50Log::PRET_CONN_DIS == pTC50Log->eLogType
  ) {

    nLen = SNPRINTF(strSQL, nFreeSize, nFreeSize, strSetInfoFMT
      , pTC50Log->nDate
      , m_strName, strCollector, file_id, s_pos, e_pos
      , pTC50Log->strLogType
      , pTC50Log->nTime

      //, pTC50Log->strLogData

      , pTC50Log->strSysInfo
      , pTC50Log->nChannelID
      , pTC50Log->strIP
      
      , pTC50Log->strOP_Organization
      , pTC50Log->strOP_Account
      , pTC50Log->strReason
      );
/*
    if (0 >= nLen) { return RC_S_FAILED; }
    return execSQL(strSQL);
*/
  }
  else if (TC50Log::PRET_UNKNOW == pTC50Log->eLogType) {

    //char_t strSQL[kMAX_SQL_LEN + 1] = {0};
    nLen = SNPRINTF(strSQL, nFreeSize, nFreeSize, strSetFailedFMT
      , pTC50Log->nDate
      , m_strName, strCollector, file_id, s_pos, e_pos
      //, pTC50Log->strLogData
      );
/*
    if (0 >= nLen) { return RC_S_FAILED; }
    return execSQL(strSQL);
*/
  }
  else if (TC50Log::PRET_SKIP == pTC50Log->eLogType) { return RC_S_OK; }
  else { return RC_E_UNSUPPORTED; }

  if (0 >= nLen) {

    (*strSQL) = 0;
    nLen = SNPRINTF(strSQL, nFreeSize, nFreeSize, strSetFailedFMT
      , pTC50Log->nDate
      , m_strName, strCollector, file_id, s_pos, e_pos
      //, pTC50Log->strLogData
      );

    if (0 >= nLen) { (*strSQL) = 0; return RC_S_FAILED; }
  }
  /*
  // write not normal log
  if (FALSE == pTC50Log->bNormal && TC50Log::PRET_UNKNOW != pTC50Log->eLogType) {

    int nLenEx = SNPRINTF(strSQL + nLen, nFreeSize - nLen, nFreeSize - nLen, strSetFailedFMT
      , pTC50Log->nDate
      , m_strName, strCollector, file_id, s_pos, e_pos
      //, pTC50Log->strLogData
    );
    if (0 >= nLenEx) { strSQL[nLen] = 0; }
    else { nLen += nLenEx; }
  }
  */
  PT.m_nSQLSize += nLen;
  return RC_S_OK;
}


//////////////////////////////////////////////////////////////////////////
ParserTC50::ParserThread::ParserThread()
  : m_ev(FALSE, FALSE)
  , m_TC50Log(NULL)

  , m_autoRelIDBConn(NULL)
  , m_autoRelMemSQL((uint8_t*)malloc(kALLOC_SQL_MEM_SIZE))
  , m_nSQLSize(0)
{}

//////////////////////////////////////////////////////////////////////////
void ParserTC50::LogFileNode::SetFileNode(const IFileNode* pIFileNode) {
  m_pIFileNode = pIFileNode;
  if (m_pIFileNode) {
    const char_t* strRemotePath = pIFileNode->RemotePath();
    if (strRemotePath) {
      const char_t* strFileName = GetFileName(strRemotePath);
      if (strFileName) { m_nDate = ATOI(strFileName); return; }
    }
  }

  m_nDate = 0;
}

void ParserTC50::waitThread() {
  while (m_acThread) { Thread::sleep(10); }
}

void ParserTC50::wakeUpThread() {
  for (size_t idx = 0; idx < kMAX_THEARD_NUM; ++idx) { m_PT[idx].m_ev.Signal(); }
}

void ParserTC50::CheckPoint() {

  uint32_t now = DayTime::to_string(DayTime::now());

  if (now < m_nTimeSchedule) { return; }

  if (now > m_nTimeSchedule + 10000) {
    if (TRUE == m_bScheduleOnce) { m_bScheduleOnce = FALSE; }
    return;
  }

  if (TRUE == m_bScheduleOnce) { return; }
 
  AutoRelease<IConnection*> autoRelIDBConn((IConnection*)(getDBConn()));
  if (NULL == autoRelIDBConn) { return; }

  // lock
  /*
  {
    char_t strSQL[512] = {0};
    STRCPY(strSQL, sizeof(strSQL), _STR("LOCK TABLES "));
    for (size_t idx = 0; idx < kLogTypeNum; ++idx) {
      SNPRINTF(strSQL, sizeof(strSQL), sizeof(strSQL)
        , "%s %s WRITE,"
        
        , strSQL
        , kStrTables[idx]
        );
    }
    size_t nStrLen = STRLEN(strSQL);
    strSQL[nStrLen - 1] = ';';
    if (RC_S_OK != autoRelIDBConn->execute(strSQL, FALSE)) { return; }
  }
  */
  uint32_t nTimeStart = DayTime::now();
  //
  {

    // get data dir
    char_t strDataDir[kMAX_PATH] = {0};

    {
      if (RC_S_OK != autoRelIDBConn->execute("SHOW VARIABLES LIKE 'datadir';", TRUE)) { return; }
      AutoRelease<IRecordSet*> autoRelRecSet(autoRelIDBConn->get_record_set());
      if (NULL == autoRelRecSet) { return; }
      if (RC_S_OK != autoRelRecSet->fetch()) { return; } 

      AutoRelease<IRecord*> autoRelRec(autoRelRecSet->get_record());
      if (NULL == autoRelRec) { return; } 

      uint8_t* data = NULL;
      uint32_t len = 0;
      if (RC_S_OK != autoRelRec->get_data(1, &data, &len)) { return; }
      if (NULL == data || 0 == len || len > kMAX_PATH) { return; }

      MEMCPY(strDataDir, kMAX_PATH, data, len);
      ChangeDirStype(strDataDir);
    }

    uint32_t nLogDate = 0;
    uint32_t nTableNameEx = 13;
    {
      // log_date
      uint32_t now = DayTime::now();
      static const uint32_t knTiem19900101 = DayTime::to_localtime(1990, 1, 1, 0, 0, 0);
      uint32_t nDataTime = now - knTiem19900101;
      const uint32_t nOneDaySec = 24 * 60 * 60;
      nLogDate = nDataTime / nOneDaySec;

      uint32_t yesterday = (nLogDate - 1) * nOneDaySec + knTiem19900101;
      DayTime::get_localtime(&nTableNameEx, NULL, NULL, NULL, NULL, NULL, yesterday);
      nTableNameEx = 0 == nTableNameEx ? 13 : nTableNameEx % 100;
    }

    // create table
    {
      char_t strHisDBName[128] = {0};
      SNPRINTF(strHisDBName, sizeof(strHisDBName), sizeof(strHisDBName), "`%s`.`", m_strHisDBName);

      char_t strHisTableNameEx[16] = {0};
      SNPRINTF(strHisTableNameEx, sizeof(strHisTableNameEx), sizeof(strHisTableNameEx), "_%02u`", nTableNameEx);

      char_t strSQL[16 * 1024] = {0};
      SNPRINTF(strSQL, sizeof(strSQL), sizeof(strSQL), kStrCreateSQLFmt
        , strHisDBName, strHisTableNameEx, m_strDBEng
        , strHisDBName, strHisTableNameEx, m_strDBEng
        , strHisDBName, strHisTableNameEx, m_strDBEng
        , strHisDBName, strHisTableNameEx, m_strDBEng
        , strHisDBName, strHisTableNameEx, m_strDBEng
        , strHisDBName, strHisTableNameEx, m_strDBEng
        , strHisDBName, strHisTableNameEx, m_strDBEng
        , strHisDBName, strHisTableNameEx, m_strDBEng
      );

      if (RC_S_OK != autoRelIDBConn->execute(strSQL, FALSE)) { return; }
    }

    //const char_t* strDBLoadDataEx = TRUE == m_bDBRemote ? "LOCAL" : NULL_STR;
    const char_t* strDBLoadDataEx = NULL_STR;
    // lock
    AutoLock autoLock(m_lockCheckPoint);

    for (size_t idx = 0; idx < kLogTypeNum; ++idx) {
      
      // work
      {
        // split
        char_t strSQL[16*1024];
        uint64_t nRows = 0;

        // get count
        {
          SNPRINTF(strSQL, sizeof(strSQL), sizeof(strSQL)
            , "SELECT count(1) FROM %s WHERE log_date < %u;"
            , kStrTables[idx], nLogDate
            );

          if (RC_S_OK != autoRelIDBConn->execute(strSQL, TRUE)) { continue; }
          AutoRelease<IRecordSet*> autoRelRecSet(autoRelIDBConn->get_record_set());
          if (NULL == autoRelRecSet) { continue; }
          if (RC_S_OK != autoRelRecSet->fetch()) { continue; } 

          AutoRelease<IRecord*> autoRelRec(autoRelRecSet->get_record());
          if (NULL == autoRelRec) { continue; }

          if (RC_S_OK != autoRelRec->get_data(0, &nRows)) { continue; }
          if (0 == nRows) { continue; }
        }

          // create temp table
        {
          SNPRINTF(strSQL, sizeof(strSQL), sizeof(strSQL)
            , 
            "DROP TABLE IF EXISTS `%s`.`TLS_TC50_TEMP`; CREATE TABLE `%s`.`TLS_TC50_TEMP` SELECT %s FROM %s LIMIT 0;"

            "ALTER TABLE `%s`.`TLS_TC50_TEMP` ENGINE=CSV;"

            , m_strHisDBName, m_strHisDBName, kStrTableFields[idx], kStrTables[idx]

            , m_strHisDBName
            );

          if (RC_S_OK != autoRelIDBConn->execute(strSQL, FALSE)) { autoRelIDBConn->rollback(); continue; }
          if (RC_S_OK != autoRelIDBConn->commit()) { continue; }
        }

        // transform data
        {
          uint32_t nLoopCount = (uint32_t)((nRows + m_nTransformRows -1 ) / m_nTransformRows);

          for (uint32_t loopIdx = 0; loopIdx < nLoopCount; ++loopIdx) {

            uint64_t nIDFirst = 0;
            uint64_t nIDLast = 0;

            uint64_t nLimitLast;
            if ((loopIdx + 1) == nLoopCount && (nRows % m_nTransformRows)) {
              nLimitLast = loopIdx * m_nTransformRows + (nRows % m_nTransformRows) - 1;
            }
            else {
              nLimitLast = loopIdx * m_nTransformRows + m_nTransformRows - 1;
            }
            // get pos
            SNPRINTF(strSQL, sizeof(strSQL), sizeof(strSQL)
              , 
              "SELECT A.`id`, B.`id` FROM "
              "(SELECT `id` FROM %s WHERE log_date < %u ORDER BY `id` LIMIT %u, 1) AS A"
              ","
              "(SELECT `id` FROM %s WHERE log_date < %u ORDER BY `id` LIMIT %u, 1) AS B"
              ";"
              , kStrTables[idx], nLogDate, loopIdx * m_nTransformRows

              , kStrTables[idx], nLogDate, (uint32_t)nLimitLast
            );

            static const uint32_t kRecSetFiledCount = 2;
            uint32_t nFiledCount = 0;

            if (RC_S_OK != autoRelIDBConn->execute(strSQL, TRUE)) { continue; }
            AutoRelease<IRecordSet*> autoRelRecSet(autoRelIDBConn->get_record_set());
            if (NULL == autoRelRecSet || RC_S_OK != autoRelRecSet->get_field_count(&nFiledCount) || 0 == nFiledCount) { continue; }
            if (RC_S_OK != autoRelRecSet->fetch()) { continue; } 

            AutoRelease<IRecord*> autoRelRec(autoRelRecSet->get_record());
            if (NULL == autoRelRec) { continue; }

            if (RC_S_OK != autoRelRec->get_data(0, &nIDFirst) || 0 == nIDFirst) { continue; }
            if (RC_S_OK != autoRelRec->get_data(1, &nIDLast)  || 0 == nIDLast) { continue; }

            SNPRINTF(strSQL, sizeof(strSQL), sizeof(strSQL)
              ,
              // insert
              // clean cvs table
              "TRUNCATE `%s`.`TLS_TC50_TEMP`;"

              "INSERT INTO `%s`.`TLS_TC50_TEMP` SELECT %s FROM %s WHERE `id` >= %I64u AND `id` <= %I64u;"

              // LOAD DATA LOW_PRIORITY LOCAL INFILE 
              "LOAD DATA LOW_PRIORITY %s INFILE '%s%s/TLS_TC50_TEMP.CSV' "
                "REPLACE INTO TABLE `%s`.`%s_%02u` "
                "FIELDS TERMINATED BY ',' "
                "OPTIONALLY ENCLOSED BY '\"' "
                "LINES TERMINATED BY '\n' "
                "(%s);"

              // clear data
              "DELETE FROM %s WHERE `id` >= %I64u AND `id` <= %I64u;"

              , m_strHisDBName

              , m_strHisDBName, kStrTableFields[idx], kStrTables[idx], nIDFirst, nIDLast

              , strDBLoadDataEx, strDataDir, m_strHisDBName
              , m_strHisDBName, kStrTables[idx], nTableNameEx
              , kStrTableFields[idx]

              , kStrTables[idx], nIDFirst, nIDLast
            );

            if (RC_S_OK != autoRelIDBConn->execute(strSQL, FALSE)) { autoRelIDBConn->rollback(); continue; }
            if (RC_S_OK != autoRelIDBConn->commit()) { continue; }

            // ok.
            --loopIdx;
            --nLoopCount;
          } // for
        } // transform data
      } // // work
    } // for
  }

  /*
  // unlock
  {
    const char_t* strSQL = _STR("UNLOCK TABLES;");
    autoRelIDBConn->execute(strSQL, FALSE);
  }
  */
  uint32_t nTimeEnd = DayTime::now();
  {
    static const char_t strTraceStart[] = _STR("ParserTC50 Transform");
    const uint32_t nStrTraceStartLen = sizeof(strTraceStart) - 1;

    char_t strTime[32] = {0};
    if (NULL == ITOA(nTimeEnd - nTimeStart, strTime, sizeof(strTime), 10)) { strTime[0] = 0;  }
    uint32_t nStrTimeLen = (uint32_t)STRLEN(strTime);

    logDB(ILogWriter::LV_INFO, m_strName, (uint8_t*)strTraceStart, nStrTraceStartLen, (uint8_t*)strTime, nStrTimeLen);
  }
  
  m_bScheduleOnce = true;  
}

END_NAMESPACE_AGGREGATOR
