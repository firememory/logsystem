/*

*/

#include "test.h"

#ifdef DO_UNITTEST

#include "coder_null.h"
USING_NAMESPACE_CODER;

TEST_BEGIN(coder, null) {
  
  NullCoder* pCoder = NullCoder::CreateInstance();
  ASSERT_NE(NULL, pCoder);

  TRACE(pCoder->getTypeName());
  TRACE(pCoder->getParam());

  ASSERT_EQ(TRUE, pCoder->didThreadSafe());

  static const char_t* src = _STR("abcd");
  uint32_t src_len = static_cast<uint32_t>(STRLEN(src));
  uint8_t dst[64];
  uint32_t dst_len = sizeof(dst);

  ASSERT_EQ(RC_E_UNSUPPORTED, 
    pCoder->encode(dst, &dst_len, reinterpret_cast<const uint8_t*>(src), src_len)
    );

  uint8_t src_decode[64];
  src_len = sizeof(src_decode);
  ASSERT_EQ(RC_E_UNSUPPORTED, 
    pCoder->decode(src_decode, &src_len, dst, dst_len)
    );

  pCoder->Release();

} TEST_END

#endif // DO_UNITTEST