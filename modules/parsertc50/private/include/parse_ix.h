/*

*/

#ifndef PARSER_IX_H_
#define PARSER_IX_H_

#include "aggregator.h"

BEGIN_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////
interface IDictIX {
public:
  virtual void Release() = 0;

public:
  virtual uint32_t GetVersion() /*const*/ = 0;
  virtual bool_t isValid() /*const*/ = 0;
  virtual bool_t Load(const uint8_t*, uint32_t) = 0;
  virtual void* GetDict() const = 0;

public:
  virtual rc_t HaveFuncID(uint16_t, uint16_t, uint16_t*) const = 0;

public:
  static IDictIX* CreateInstance(const char_t*, const uint8_t*, uint32_t);
}; // IDictIX

static const char_t* kStrWtDict2        = _STR("WtDict2");

interface IParseIX {
public:
  virtual void Release() = 0;

public:
  virtual bool_t isValid() const = 0;
  virtual rc_t SetDict(const IDictIX*) = 0;
  virtual rc_t Parse(uint16_t wFuncID, const char_t*, uint32_t) = 0;

public:
  virtual rc_t get_data(uint16_t idx, char_t* val, uint32_t len) = 0;
  virtual rc_t get_data(uint16_t idx, uint32_t* val) = 0;

public:
  virtual rc_t GetReturnNo(int32_t*) = 0;
  virtual rc_t GetErrmsg(char_t*, uint32_t) = 0;

public:
  virtual uint32_t GetLineCount() = 0;
  virtual rc_t GetCookies(char_t*, uint32_t) = 0;
  virtual rc_t Next() = 0;

  virtual rc_t HaveFuncID(uint16_t, uint16_t, uint16_t*) const = 0;

public:
  static IParseIX* CreateInstance(const char_t*, const IDictIX*);
}; // IParseIX

static const char_t* kStrParseWtComm    = _STR("ParseWtCom");
#define USE_WTCOM_PARSER_ANS
END_NAMESPACE_AGGREGATOR
#endif // PARSER_IX_H_

