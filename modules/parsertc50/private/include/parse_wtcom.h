/*

*/

#ifndef PARSE_WTCOM_H_
#define PARSE_WTCOM_H_

//////////////////////////////////////////////////////////////////////////
#include "aggregator_base.h"
#include "parse_ix.h"

#include "std_list"
#include "hash_tables.h"

// T2EE

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4201)
# pragma warning(disable: 4251)
#endif // MSC_VER

#undef _USE_32BIT_TIME_T

#define SAFEVCRT_NOAUTOLINK
#define DYNA_SAFEVCRT
#include <safevcrt.h>

#define CLIBHLPR_NOAUTOLINK
#include <clibhlpr.h>

//#define ASE_NODE
//#define ASELIB_STATIC
#define ASE_FOR_CLIENT
#include "ASE.h"

#ifdef _MSC_VER
# pragma warning(default: 4251)
#pragma warning(pop)
#endif // MSC_VER


BEGIN_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////
USING_BASE(hash_map);

class WtDict2 : public IDictIX {

  // IDictIX
public:
  void Release();

public:
  uint32_t GetVersion() /*const*/;
  bool_t isValid() /*const*/;
  bool_t Load(const uint8_t*, uint32_t);
  void* GetDict() const;

public:
  rc_t HaveFuncID(uint16_t, uint16_t, uint16_t*) const;

public:
  WtDict2(const uint8_t*, uint32_t);
  ~WtDict2();

private:
  bool_t    m_bLoad;
  CWtDict2  m_CWtDict2;

private:
  void makeDict();

#ifndef USE_WTCOM_PARSER_ANS
  class StructInfo {
  public:
    StructInfo() {}
    ~StructInfo() { m_mapFiled.clear(); }

  public:
    rc_t HasField(uint16_t nFuncID, uint16_t* nPos) const {

      field_map_t::const_iterator it = m_mapFiled.find(nFuncID);
      if (m_mapFiled.end() == it) { return RC_S_NOTFOUND; }

      (*nPos) = it->second;
      return RC_S_OK;
    }

    void InsertFuncID(uint16_t nFuncID, uint16_t nPos) {
      m_mapFiled[nFuncID] = nPos;
    }

  private:
    typedef hash_map<uint16_t, uint16_t> field_map_t;
    field_map_t m_mapFiled;
  };

  typedef hash_map<uint16_t, StructInfo> struct_map_t;

  struct_map_t  m_mapStructID;
#endif // USE_WTCOM_PARSER_ANS
}; // WtDict2

class ParseWtcom2 : public IParseIX {

  // IParseIX
public:
  void Release();

public:
  bool_t isValid() const { return m_pIDictIX ? TRUE : FALSE; }
  rc_t SetDict(const IDictIX* pDict);
  rc_t Parse(uint16_t wStructID, const char_t*, uint32_t);

public:
  rc_t get_data(uint16_t idx, char_t* val, uint32_t len);
  rc_t get_data(uint16_t idx, uint32_t* val);

public:
  rc_t GetReturnNo(int32_t*);
  rc_t GetErrmsg(char_t*, uint32_t);

public:
  uint32_t GetLineCount();
  rc_t GetCookies(char_t*, uint32_t);
  rc_t Next();
  rc_t HaveFuncID(uint16_t, uint16_t, uint16_t*) const;

public:
  ParseWtcom2(const IDictIX* pDict);
  ~ParseWtcom2();

private:
  CWtCommon   m_CWtCommon;
  const IDictIX*    m_pIDictIX;
}; // ParseWtcom

END_NAMESPACE_AGGREGATOR
#endif // PARSE_WTCOM_H_

