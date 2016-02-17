/*

*/

#include "coder_deflate.h"

#if !defined(REUSE_STREAM)
# include <zlib.h>
#endif // REUSE_STREAM


#include "code_util.h"

BEGIN_NAMESPACE_CODER
//////////////////////////////////////////////////////////////////////////
USING_BASE(ParseString);

//////////////////////////////////////////////////////////////////////////

const char_t* DeflateCoder::strName     = _STR("Deflate");
const char_t* DeflateCoder::strAttrLevel= _STR("lv=");

DeflateCoder* DeflateCoder::CreateInstance(const char_t* strParam) {

  return new DeflateCoder(strParam);
}

void DeflateCoder::Release() { delete this; }

#if !defined(REUSE_STREAM)
DeflateCoder::DeflateCoder(const char_t* strParam)
  : m_nLevel(default_level)
{

  if (strParam) {

    char_t level[2] = {0};
    if (RC_S_OK == ParseString(level, sizeof(level), strParam, STRLEN(strParam), strAttrLevel, strAttrValueEnd)) {
      m_nLevel = ATOI(level);
    }
  }

  SNPRINTF(m_strParam, sizeof(m_strParam), _STR("%s%u"), strAttrLevel, m_nLevel);
}

DeflateCoder::~DeflateCoder() {}

class DeflateStream {
public:
  DeflateStream(int& err, int32_t lv) {

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    err = deflateInit(&stream, lv);
  }

  ~DeflateStream() { (void)deflateEnd(&stream); }

  z_stream stream;
}; // DeflateStream

rc_t DeflateCoder::encode(uint8_t* dest, uint32_t* destLen, const uint8_t* source, uint32_t sourceLen) {

  ASSERT(source && sourceLen && dest && destLen);
  ASSERT(*destLen >= getEncodeSize(source, sourceLen));

  int err = Z_OK;
  DeflateStream as(err, m_nLevel);
  if (Z_OK != err) { RC_RETURN(RC_S_FAILED + err); }

  as.stream.next_in = (Bytef*)source;
  as.stream.avail_in = (uInt)sourceLen;

#ifdef MAXSEG_64K
  /* Check for source > 64K on 16-bit machine: */
  if ((uLong)as.stream.avail_in != sourceLen) RC_RETURN(RC_S_FAILED + Z_BUF_ERROR);//return Z_BUF_ERROR;
#endif
  as.stream.next_out = dest;
  as.stream.avail_out = (uInt)*destLen;
  if ((uLong)as.stream.avail_out != *destLen) RC_RETURN(RC_S_FAILED + Z_BUF_ERROR);//return Z_BUF_ERROR;

  err = deflate(&as.stream, Z_FINISH);
  if (Z_STREAM_END != err) { RC_RETURN(RC_S_FAILED + err); }

  *destLen = as.stream.total_out;
  return RC_S_OK;
}

class InflateStream {
public:
  InflateStream(int& err) {

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    //stream.opaque = (voidpf)0;

    err = inflateInit(&stream);
  }

  ~InflateStream() { (void)inflateEnd(&stream); }

  z_stream stream;
}; // InflateStream

rc_t DeflateCoder::decode(uint8_t* dest, uint32_t* destLen, const uint8_t* source, uint32_t sourceLen) {

  ASSERT(source && sourceLen && dest && destLen);
  ASSERT(*destLen >= getDecodeSize(source, sourceLen));

  int err = Z_OK;
  InflateStream as(err);
  if (Z_OK != err) { RC_RETURN(RC_S_FAILED + err); }

  as.stream.next_in = (Bytef*)source;
  as.stream.avail_in = (uInt)sourceLen;
  /* Check for source > 64K on 16-bit machine: */
  if ((uLong)as.stream.avail_in != sourceLen) RC_RETURN(RC_S_FAILED + Z_BUF_ERROR);//return Z_BUF_ERROR;

  as.stream.next_out = dest;
  as.stream.avail_out = (uInt)*destLen;
  if ((uLong)as.stream.avail_out != *destLen) RC_RETURN(RC_S_FAILED + Z_BUF_ERROR);//return Z_BUF_ERROR;

  err = inflate(&as.stream, Z_FINISH);
  if (Z_STREAM_END != err) { RC_RETURN(RC_S_FAILED + err); }

  *destLen = as.stream.total_out;
  return RC_S_OK;
}
#else
DeflateCoder::DeflateCoder(const char_t* strParam)
: m_nLevel(default_level)
, m_nStatus(RC_S_UNKNOWN)
{

  if (strParam) {

    char_t level[2] = {0};
    if (RC_S_OK == ParseString(level, sizeof(level), strParam, STRLEN(strParam), strAttrLevel, kStrAttrValueEnd)) {
      m_nLevel = ATOI(level);
    }
  }

  m_de_stream.zalloc = (alloc_func)0;
  m_de_stream.zfree = (free_func)0;
  m_de_stream.opaque = (voidpf)0;

  int err = deflateInit(&m_de_stream, m_nLevel);
  if (Z_OK != err) { return; }

  m_in_stream.zalloc = (alloc_func)0;
  m_in_stream.zfree = (free_func)0;
  //stream.opaque = (voidpf)0;

  err = inflateInit(&m_in_stream);
  if (Z_OK != err) { (void)deflateEnd(&m_de_stream); return; }

  m_nStatus = RC_S_OK;
  SNPRINTF(m_strParam, sizeof(m_strParam), _STR("%s%u"), strAttrLevel, m_nLevel);
}

DeflateCoder::~DeflateCoder() {

  if (RC_S_OK == m_nStatus) {

    (void)deflateEnd(&m_de_stream);
    (void)inflateEnd(&m_in_stream);
  }
}

rc_t DeflateCoder::encode(uint8_t* dest, uint32_t* destLen, const uint8_t* source, uint32_t sourceLen) {

  ASSERT(source && sourceLen && dest && destLen);
  ASSERT(*destLen >= getEncodeSize(source, sourceLen));

  int err = deflateReset(&m_de_stream);
  if (Z_OK != err) { RC_RETURN(RC_S_FAILED + err); }

  m_de_stream.next_in = (Bytef*)source;
  m_de_stream.avail_in = (uInt)sourceLen;

#ifdef MAXSEG_64K
  /* Check for source > 64K on 16-bit machine: */
  if ((uLong)m_de_stream.avail_in != sourceLen) RC_RETURN(RC_S_FAILED + Z_BUF_ERROR);//return Z_BUF_ERROR;
#endif
  m_de_stream.next_out = dest;
  m_de_stream.avail_out = (uInt)*destLen;
  if ((uLong)m_de_stream.avail_out != *destLen) RC_RETURN(RC_S_FAILED + Z_BUF_ERROR);//return Z_BUF_ERROR;

  err = deflate(&m_de_stream, Z_FINISH);
  if (Z_STREAM_END != err) { RC_RETURN(RC_S_FAILED + err); }

  *destLen = m_de_stream.total_out;
  return RC_S_OK;
}

rc_t DeflateCoder::decode(uint8_t* dest, uint32_t* destLen, const uint8_t* source, uint32_t sourceLen) {

  ASSERT(source && sourceLen && dest && destLen);
  ASSERT(*destLen >= getDecodeSize(source, sourceLen));

  int err = inflateReset(&m_in_stream);
  if (Z_OK != err) { RC_RETURN(RC_S_FAILED + err); }

  m_in_stream.next_in = (Bytef*)source;
  m_in_stream.avail_in = (uInt)sourceLen;
  /* Check for source > 64K on 16-bit machine: */
  if ((uLong)m_in_stream.avail_in != sourceLen) RC_RETURN(RC_S_FAILED + Z_BUF_ERROR);//return Z_BUF_ERROR;

  m_in_stream.next_out = dest;
  m_in_stream.avail_out = (uInt)*destLen;
  if ((uLong)m_in_stream.avail_out != *destLen) RC_RETURN(RC_S_FAILED + Z_BUF_ERROR);//return Z_BUF_ERROR;

  err = inflate(&m_in_stream, Z_FINISH);
  if (Z_STREAM_END != err) { RC_RETURN(RC_S_FAILED + err); }

  *destLen = m_in_stream.total_out;
  return RC_S_OK;
}
#endif // 

uint32_t DeflateCoder::getEncodeSize(const uint8_t* src, uint32_t src_len) const {

  UNUSED_PARAM(src);
  UNUSED_PARAM(src_len);
  return 0;
}

uint32_t DeflateCoder::getDecodeSize(const uint8_t* src, uint32_t src_len) const {
  
  UNUSED_PARAM(src);
  UNUSED_PARAM(src_len);
  return 0;
}

const char_t* DeflateCoder::getParam() const { 
  
  return m_strParam; 
}

END_NAMESPACE_CODER
