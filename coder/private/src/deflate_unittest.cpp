/*

*/

#include "test.h"

#ifdef DO_UNITTEST

#include "coder_deflate.h"
USING_NAMESPACE_CODER;

TEST_BEGIN(coder, deflate) {

  DeflateCoder* pCoder = DeflateCoder::CreateInstance("lv=5");
  ASSERT_NE(NULL, pCoder);

  TRACE(pCoder->getTypeName());
  TRACE(pCoder->getParam());

#if !defined(REUSE_STREAM)
  ASSERT_EQ(TRUE, pCoder->didThreadSafe());
#else
  ASSERT_EQ(FALSE, pCoder->didThreadSafe());
#endif // REUSE_STREAM

  //const uint32_t kCRC32Value = 0x26CA43FA;
  const char_t* src = _STR("hello world");
  uint32_t src_len = static_cast<uint32_t>(STRLEN(src));
  uint8_t dst[64];
  uint32_t dst_len = sizeof(dst);

  ASSERT_EQ(RC_S_OK, 
    pCoder->encode(dst, &dst_len, reinterpret_cast<const uint8_t*>(src), src_len)
    );

  uint8_t src_decode[64];
  src_len = sizeof(src_decode);
  ASSERT_EQ(RC_S_OK, 
    pCoder->decode(src_decode, &src_len, dst, dst_len)
    );

  ASSERT_EQ(0, MEMCMP(src_decode, src, src_len));

  // again
  src = _STR("0cc175b9c0f1b6a831c399e269772661");
  src_len = static_cast<uint32_t>(STRLEN(src));
  dst_len = sizeof(dst);
  ASSERT_EQ(RC_S_OK, 
    pCoder->encode(dst, &dst_len, reinterpret_cast<const uint8_t*>(src), src_len)
    );

  src_len = sizeof(src_decode);
  ASSERT_EQ(RC_S_OK, 
    pCoder->decode(src_decode, &src_len, dst, dst_len)
    );

  ASSERT_EQ(0, MEMCMP(src_decode, src, src_len));

  // binary
  unsigned char expected[] = { 0x84, 0x98, 0x3e, 0x44,
                               0x1c, 0x3b, 0xd2, 0x6e,
                               0xba, 0xae, 0x4a, 0xa1,
                               0xf9, 0x51, 0x29, 0xe5,
                               0xe5, 0x46, 0x70, 0xf1 };

  src_len = sizeof(expected);
  dst_len = sizeof(dst);
  ASSERT_EQ(RC_S_OK, 
    pCoder->encode(dst, &dst_len, reinterpret_cast<const uint8_t*>(expected), src_len)
    );

  src_len = sizeof(src_decode);
  ASSERT_EQ(RC_S_OK, 
    pCoder->decode(src_decode, &src_len, dst, dst_len)
    );

  ASSERT_EQ(0, MEMCMP(src_decode, expected, src_len));

  pCoder->Release();

} TEST_END

#endif // DO_UNITTEST