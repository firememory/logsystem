/*

*/

#ifndef ENCODE_H_
#define ENCODE_H_

//////////////////////////////////////////////////////////////////////////
#include <config.h>
#include "error.h"

#include "coder_export.h"

#if HAS_NAMESPACE
# define NAMESPACE_CODER_NAME           coder
# define BEGIN_NAMESPACE_CODER          BEGIN_NAMESPACE namespace NAMESPACE_CODER_NAME {
# define END_NAMESPACE_CODER            } END_NAMESPACE
# define USING_NAMESPACE_CODER          using namespace NAMESPACE_NAME::NAMESPACE_CODER_NAME
# define USING_CODER(x)                 using NAMESPACE_CODER_NAME::x
#else
# define BEGIN_NAMESPACE_CODER
# define END_NAMESPACE_CODER
# define USING_NAMESPACE_CODER
# define USING_CODER(x)
#endif

BEGIN_NAMESPACE_CODER

class ICoder {
public:
  virtual void Release() = 0;

public:
  virtual rc_t encode(uint8_t* dst, uint32_t* dst_size, const uint8_t* src, uint32_t src_len) = 0;
  virtual rc_t decode(uint8_t* dst, uint32_t* dst_size, const uint8_t* src, uint32_t src_len) = 0;

  /* 
    return 0 when unsupport metoh
  */
  virtual uint32_t getEncodeSize(const uint8_t* src, uint32_t src_len) const = 0;
  virtual uint32_t getDecodeSize(const uint8_t* src, uint32_t src_len) const = 0;

  virtual const char_t* getTypeName() const = 0;
  virtual const char_t* getParam() const = 0;

  virtual bool_t didThreadSafe() const = 0;
  
public:
  static ICoder* CreateInstance(const char_t* strType, const char_t* strParam);
}; // ICoder

//////////////////////////////////////////////////////////////////////////
EXTERN_C CODER_EXPORT
ICoder* Coder_CreateInstance(const char_t* strType, const char_t* strParam);

END_NAMESPACE_CODER

#endif // ENCODE_H_
