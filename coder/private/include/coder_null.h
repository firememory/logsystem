/*

*/

#ifndef CODER_NULL_H_
#define CODER_NULL_H_

#include "coder.h"

BEGIN_NAMESPACE_CODER
//////////////////////////////////////////////////////////////////////////
class NullCoder : public ICoder {
public:
  void Release() {};

public:
  rc_t encode(uint8_t* dst, uint32_t* dst_size, const uint8_t* src, uint32_t src_len) { 
    UNUSED_PARAM(src);
    UNUSED_PARAM(src_len);
    UNUSED_PARAM(dst);
    UNUSED_PARAM(dst_size);
    return RC_E_UNSUPPORTED; 
  }

  rc_t decode(uint8_t* dst, uint32_t* dst_size, const uint8_t* src, uint32_t src_len) { 
    UNUSED_PARAM(src);
    UNUSED_PARAM(src_len);
    UNUSED_PARAM(dst);
    UNUSED_PARAM(dst_size);
    return RC_E_UNSUPPORTED;
  }

  uint32_t getEncodeSize(const uint8_t* src, uint32_t src_len) const {
    UNUSED_PARAM(src);
    UNUSED_PARAM(src_len);
    return 0;
  }

  uint32_t getDecodeSize(const uint8_t* src, uint32_t src_len) const {
    UNUSED_PARAM(src);
    UNUSED_PARAM(src_len);
    return 0;
  }

  const char_t* getTypeName() const { return strName; }
  const char_t* getParam() const { return NULL_STR; }
  bool_t didThreadSafe() const { return TRUE; }

public:
  static NullCoder* CreateInstance(const char_t* strParam = NULL_STR) {
    UNUSED_PARAM(strParam);
    return &NULL_Coder;
  }

  static const char_t* strName;

private:
  static NullCoder NULL_Coder;

}; // NullCoder

END_NAMESPACE_CODER

#endif // CODER_NULL_H_
