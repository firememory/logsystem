/*

*/

#ifndef CODER_DEFLATE_H_
#define CODER_DEFLATE_H_

#include "coder.h"

//////////////////////////////////////////////////////////////////////////
#define REUSE_STREAM

#if defined(REUSE_STREAM)
# include <zlib.h>
#endif // REUSE_STREAM

BEGIN_NAMESPACE_CODER
//////////////////////////////////////////////////////////////////////////
class DeflateCoder : public ICoder {

// ICoder
public:
  void Release();  

public:
  rc_t encode(uint8_t* dst, uint32_t* dst_size, const uint8_t* src, uint32_t src_len);

  rc_t decode(uint8_t* dst, uint32_t* dst_size, const uint8_t* src, uint32_t src_len);

  uint32_t getEncodeSize(const uint8_t* src, uint32_t src_len) const;

  uint32_t getDecodeSize(const uint8_t* src, uint32_t src_len) const;

  const char_t* getTypeName() const { return strName; }
  const char_t* getParam() const;
  bool_t didThreadSafe() const { return FALSE; }

// Deflate
public:
  static DeflateCoder* CreateInstance(const char_t*);
  static const char_t* strName;
  static const char_t* strAttrLevel;

private:
  DeflateCoder(const char_t*);
  ~DeflateCoder();

private:
  char_t  m_strParam[8];

  enum { default_level = 6 };
  int32_t m_nLevel;

#if defined(REUSE_STREAM)
  z_stream m_de_stream;
  z_stream m_in_stream;
  rc_t m_nStatus;
#endif // REUSE_STREAM

}; // DeflateCoder

END_NAMESPACE_CODER

#endif // CODER_DEFLATE_H_
