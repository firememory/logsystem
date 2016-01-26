/*

*/

#include "logtype_tc50.h"

#include "protocol.h"
#include "getset_param.h"

#include "time_util.h"

BEGIN_NAMESPACE_AGGREGATOR

USING_BASE(DayTime);


#define STR_VERSION           "v1.0.0.721"

//////////////////////////////////////////////////////////////////////////
// resource
const rc_t LogTypeTC50::GetName(char_t* strName, uint32_t* size) const {
  if (NULL == strName || NULL == size || 0 == (*size)) { return RC_S_NULL_VALUE; }
  STRCPY(strName, (*size), m_strName);
  return RC_S_OK;
}

const rc_t LogTypeTC50::GetStatus() const { return RC_S_OK; }

const rc_t LogTypeTC50::GetResourceSize(uint64_t* curr_size, uint64_t* free_size) const {

  if (NULL == curr_size || 0 == free_size) { return RC_S_NULL_VALUE; }

  (*curr_size) = 0x00;
  (*free_size) = 0x00;
  return RC_S_OK;
}

const rc_t LogTypeTC50::GetLastErr(uint32_t* err_no, char_t* err_msg, uint32_t* size) const {

  if (NULL == err_no || NULL == err_msg || NULL == size || 0 == (*size)) { return RC_S_NULL_VALUE; }

  (*err_no) = 0;
  STRCPY(err_msg, (*size), NULL_STR);
  return RC_S_OK;
}

const rc_t LogTypeTC50::GetInfo(char_t* info, uint32_t* size) const {

  if (NULL == info || NULL == size || 0 == (*size)) { return RC_S_NULL_VALUE; }

  (*size) = SNPRINTF(info, (*size), (*size), _STR("%s: \n"
    "Type Count=%u\n"
    )

    , STR_VERSION
    , m_nCountType
    );
  return RC_S_OK;
}

const rc_t LogTypeTC50::GetLastAliveTime(uint32_t* alive) const {

  if (NULL == alive) { return RC_S_NULL_VALUE; }

  (*alive) = m_timeAlive;
  return RC_S_OK;
}


//////////////////////////////////////////////////////////////////////////
static const uint32_t kJUDGE_RULE_DIR    = 0x00000001;
static const uint32_t kJUDGE_RULE_NAME   = 0x00000002;
static const uint32_t kJUDGE_RULE_HEAD   = 0x00000004;
//////////////////////////////////////////////////////////////////////////
const char_t* LogTypeTC50::LogType(const IFileNode* pIFileNode, const IMemoryNode* pIMemoryNode) {

  if (NULL == pIFileNode) { return NULL; }
  UNUSED_PARAM(pIMemoryNode);

  const char_t* strRemotePath = pIFileNode->RemotePath();
  const char_t* strName = getFileName(strRemotePath);
  if (NULL == strName) { return NULL; }

  log_type_rule_list_t::iterator it_list, end;
  for (it_list = m_listLogTypeRule.begin(), end = m_listLogTypeRule.end(); it_list != end; ++it_list) {

    LogTypeRule& logTypeRule = (*it_list);

    if (logTypeRule.judge_rule & kJUDGE_RULE_DIR) {
      if (logTypeRule.regexDir.didCompile() && FALSE == logTypeRule.regexDir.isMatchOnly(strRemotePath)) {
        continue;
      }
    }

    if (logTypeRule.judge_rule & kJUDGE_RULE_NAME) {
      if (logTypeRule.regexName.didCompile() && FALSE == logTypeRule.regexName.isMatchOnly(strName)) {
        continue;
      }
    }

    m_timeAlive = DayTime::now();
    ++m_nCountType;
    return m_strLogType;

    /*
    if (logTypeRule.judge_rule & kJUDGE_RULE_HEAD
      && FALSE == logTypeRule.regexDir.isMatchOnly(strRemotePath)
    ) { 
      
      uint32_t len = pIMemoryNode->len();
      const uint8_t* data = pIMemoryNode->data();
      continue;
    }
    */
  }
  return NULL;
}

//////////////////////////////////////////////////////////////////////////
const char_t* LogTypeTC50::m_strLogType  = _STR("LT_TC50");
const char_t* LogTypeTC50::m_strName     = _STR("LogTypeTC50");
const uuid LogTypeTC50::m_gcUUID         = 0x00003250;

//////////////////////////////////////////////////////////////////////////
LogTypeTC50::LogTypeTC50()
  : m_pIAggregator(NULL)
  , m_nCountType(0)
  , m_timeAlive(0)
{}

LogTypeTC50::~LogTypeTC50() {
}

rc_t LogTypeTC50::Init(IAggregatorBase* pIAggregator) {

  if (NULL == pIAggregator) { return RC_S_NULL_VALUE; }

  m_pIAggregator = pIAggregator;

  // get data base rule

  return loadLogTypeRule();
}


//////////////////////////////////////////////////////////////////////////
// 
struct LogTypeRuleReulst {

  LogTypeTC50*    pLogTypeTC50;
  LogTypeRule*    pLogTypeRule;
  
  LogTypeRuleReulst(LogTypeTC50* pLogTypeTC50)
    : pLogTypeTC50(pLogTypeTC50)
    , pLogTypeRule(NULL)
  {}
};

typedef GetSetParamImpl<LogTypeRuleReulst*>   LogTypeRuleGSObject;
template<>
void LogTypeRuleGSObject::SetResult(uint32_t row, uint32_t col, const char_t* val, uint32_t len) {

  ASSERT(m_r);
  UNUSED_PARAM(row);

  if (0 == col) {

    LogTypeRule logTypeRule;
    m_r->pLogTypeTC50->m_listLogTypeRule.push_back(logTypeRule);
    m_r->pLogTypeRule = &(m_r->pLogTypeTC50->m_listLogTypeRule.back());
  }

  LogTypeRule* pLogTypeRule = m_r->pLogTypeRule;

  if (NULL == pLogTypeRule) { return; }
  if (NULL == val || 0 == len) { return; }
  switch(col) {
    case 0: {
      STRCPY(pLogTypeRule->strDir, kMAX_PATH, val);
      pLogTypeRule->regexDir.Compile(pLogTypeRule->strDir);
      break;
    }
    case 1: {
      STRCPY(pLogTypeRule->strName, kMAX_NAME_LEN, val);
      pLogTypeRule->regexName.Compile(pLogTypeRule->strName);
      break;
    }
    case 2: {

      if (STRSTR(val, len, "DIR")) { pLogTypeRule->judge_rule |= kJUDGE_RULE_DIR; }
      if (STRSTR(val, len, "NAME")) { pLogTypeRule->judge_rule |= kJUDGE_RULE_NAME; }
      if (STRSTR(val, len, "HEAD")) { pLogTypeRule->judge_rule |= kJUDGE_RULE_HEAD; }
      break;
    }
  }
}

rc_t LogTypeTC50::loadLogTypeRule() {

  ASSERT(m_pIAggregator);

  LogTypeRuleReulst logTypeRuleReulst(this);
  // get aggregator_dir
  LogTypeRuleGSObject logTypeRuleGSObject(&logTypeRuleReulst, m_strLogType);

  return m_pIAggregator->GetSetting(&logTypeRuleGSObject, kStrGetLogTypeRule);
}

const char_t* LogTypeTC50::getFileName(const char_t* strPath) {

  if (NULL == strPath) { return NULL; }

  const char_t* strLastFind = NULL;
  const char_t* strFind = strPath;

  while ((*strFind) && NULL != (strFind = STRCHR(strFind, STRLEN(strFind), CHAR_DIR_SEP))) {

    strLastFind = strFind;
    ++strFind;
  }

  return strLastFind ? strLastFind + 1 : NULL;
}

END_NAMESPACE_AGGREGATOR
