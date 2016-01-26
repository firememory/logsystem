/*

*/

#ifndef GETSET_PARAM_H_
#define GETSET_PARAM_H_

#include "aggregator_base.h"

BEGIN_NAMESPACE_AGGREGATOR
/*
class GetSetParam : public IGetSetParam {

  // IObject
public:
  const uuid& GetUUID() const { return kUUID_IGetSetParam; }
  const void* ConstCast(const uuid& id) const { 
    if (id == kUUID_IGetSetParam) { return this; }
    return NULL;
  }
  void* Cast(const uuid& id) { 
    if (id == kUUID_IGetSetParam) { return this; }
    return NULL;
  }

  const char_t* GetName() const { return NULL_STR; }
  const rc_t GetStatus() const { return RC_S_OK; }

  // ref count
protected:
  void AddRef() {}

public:  
  void Release() {}
}; // GetSetParam
*/
/*
template<typename _RT
                      , typename _T0 = const char_t*, typename _T1 = const char_t*
                      , typename _T2 = const char_t*, typename _T3 = const char_t*>
*/
template<typename _RT>
class GetSetParamImpl : public IGetSetParam {
  // IGetSetParam
public:
  const char_t* GetCondition(uint32_t idx, uint32_t* len) const { 
    if (idx >= CONDITIONS) { return NULL; }
    if (len) { (*len) = m_l[idx]; }
    return m_c[idx];
  }

public:
  void SetResult(uint32_t /*row*/, uint32_t /*col*/, const char_t*, uint32_t) {} // do noting
  typedef typename _RT  result_type_t;
  
public:
  enum { RESULTS = 1, CONDITIONS = 6 };

  typedef const char_t*       _Ty;


  GetSetParamImpl(_RT r
    , _Ty t0 = NULL, uint32_t l0 = 0
    , _Ty t1 = NULL, uint32_t l1 = 0
    , _Ty t2 = NULL, uint32_t l2 = 0
    , _Ty t3 = NULL, uint32_t l3 = 0
    , _Ty t4 = NULL, uint32_t l4 = 0
    , _Ty t5 = NULL, uint32_t l5 = 0)
  {
    SetParsm(r, t0, l0, t1, l1, t2, l2, t3, l3, t4, l4, t5, l5);
  }

  void SetParsm(_RT r, _Ty t0 = NULL, uint32_t l0 = 0
    , _Ty t1 = NULL, uint32_t l1 = 0
    , _Ty t2 = NULL, uint32_t l2 = 0
    , _Ty t3 = NULL, uint32_t l3 = 0
    , _Ty t4 = NULL, uint32_t l4 = 0
    , _Ty t5 = NULL, uint32_t l5 = 0)
  {
    m_r = r;
    m_c[0] = t0; m_l[0] = l0;
    m_c[1] = t1; m_l[1] = l1;
    m_c[2] = t2; m_l[2] = l2;
    m_c[3] = t3; m_l[3] = l3;
    m_c[4] = t4; m_l[4] = l4;
    m_c[5] = t5; m_l[5] = l5;
  }

  _RT& GetResult() { return m_r; }

  // return
private:
  _RT       m_r;

  // Condition
  _Ty       m_c[CONDITIONS];
  uint32_t  m_l[CONDITIONS];
}; // GetSetParamImpl

//////////////////////////////////////////////////////////////////////////
typedef GetSetParamImpl<NullClass*>      NormalSetGSObject;

END_NAMESPACE_AGGREGATOR

#endif // GETSET_PARAM_H_
