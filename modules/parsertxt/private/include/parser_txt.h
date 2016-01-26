/*

*/

#ifndef PARSER_TXT_H_
#define PARSER_TXT_H_

//////////////////////////////////////////////////////////////////////////
#include "aggregator_base.h"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4127)
#endif // MSC_VER

#include <hash_map>

#ifdef _MSC_VER
# pragma warning(default: 4127)
#pragma warning(pop)
#endif // MSC_VER


BEGIN_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////

class ParserTxt : public IParser {

  // IObject
public:
  const uuid& GetUUID() const { return m_gcUUID; }
  const char_t* GetName() const { return m_strName; }
  const rc_t GetStatus() const { return RC_S_OK; }
  const void* ConstCast(const uuid& id) const {
    if (m_gcUUID == id) { return this; }
    else if (kUUID_IParser == id) { return (IParser*)(this); }
    return NULL;
  }
  void* Cast(const uuid& id) {
    if (m_gcUUID == id) { return this; }
    else if (kUUID_IParser == id) { return (IParser*)(this); }
    return NULL;
  }

  // ref count
protected:
  void AddRef() {}

public:  
  void Release() {}

  // IParser
public:
  rc_t Start();
  void Stop();

public:
  ParserTxt();
  ~ParserTxt();

public:
  rc_t Init(IAggregatorBase* pIAggregator);

private:
  static rc_t didProcFile(IFileIteratorBase*, const IFileNode*, void* context);
  rc_t isFileOver(file_id_t);

  // filenode
private:

private:
  volatile bool_t             m_bRunning;

  typedef stdext::hash_map<file_id_t, file_size_t>    file_map_t;
  file_map_t                  m_mapFile;

private:
  uint32_t                    m_nwtGetFileIterator;

private:
  IAggregatorBase*            m_pIAggregator;
  static const char_t*        m_strName;
  static const uuid           m_gcUUID;
  DISALLOW_COPY_AND_ASSIGN(ParserTxt);
}; // ParserTxt

END_NAMESPACE_AGGREGATOR
#endif // PARSER_TXT_H_

