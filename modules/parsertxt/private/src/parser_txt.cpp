/*

*/

#include "parser_txt.h"

BEGIN_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////

const char_t* strLogTypes[] = {_STR("TXT"), NULL};

rc_t ParserTxt::Start() { 

  ASSERT(m_pIAggregator);

  const char_t* strLogType = strLogTypes[0];

  while (TRUE == m_bRunning) {

    AutoRelease<IFileIteratorBase*> autoRelFileIterator(m_pIAggregator->GetFileIterator(strLogType, m_nwtGetFileIterator));

    if (NULL == autoRelFileIterator) { continue; }

    // file iterate
    autoRelFileIterator->file_iterate(didProcFile, this);
  }
  return RC_S_OK;
}

void ParserTxt::Stop() {

  m_bRunning = FALSE;
}

//////////////////////////////////////////////////////////////////////////
const char_t* ParserTxt::m_strName      = _STR("ParserText");
const uuid ParserTxt::m_gcUUID          = 0x00003012;
//////////////////////////////////////////////////////////////////////////
ParserTxt::ParserTxt()
  : m_pIAggregator(NULL)
  , m_bRunning(TRUE)
  , m_nwtGetFileIterator(10) // 10s
{}

ParserTxt::~ParserTxt() {

  m_mapFile.clear();
}

rc_t ParserTxt::Init(IAggregatorBase* pIAggregator) {

  if (NULL == pIAggregator) { return RC_S_NULL_VALUE; }

  m_pIAggregator = pIAggregator;
  m_mapFile.clear();
  return RC_S_OK;
}

rc_t ParserTxt::didProcFile(IFileIteratorBase* pIFileIterator, const IFileNode* pIFileNode, void* context) {

  if (NULL == context || NULL == pIFileIterator || NULL == pIFileNode) { return RC_S_OK; }

  ParserTxt* pParserTxt = (ParserTxt*)(context);
  ASSERT(pParserTxt);

  file_id_t file_id = pIFileNode->ID();

  file_map_t::iterator it_map = pParserTxt->m_mapFile.find(file_id);
  if (pParserTxt->m_mapFile.end() == it_map) {

    // new file
    pParserTxt->m_mapFile[file_id] = 0;
    it_map = pParserTxt->m_mapFile.find(file_id);
    ASSERT(pParserTxt->m_mapFile.end() != it_map);
  }
  else {
    if (RC_S_OK == pParserTxt->isFileOver(file_id)) { return RC_S_CLOSED; }
  }

  if (pIFileNode->Size() < it_map->second) { return RC_S_OK; }

  // Parse 
  return RC_S_OK;
}

rc_t ParserTxt::isFileOver(file_id_t file_id) {

  UNUSED_PARAM(file_id);

  bool_t bOver = FALSE;
  return TRUE == bOver ? RC_S_OK : RC_S_FAILED;
}

END_NAMESPACE_AGGREGATOR
