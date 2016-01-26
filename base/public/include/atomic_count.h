/*

*/

#ifndef ATOMIC_COUNT_H_
#define ATOMIC_COUNT_H_

#include "base.h"
#include "base_export.h"

BEGIN_NAMESPACE_BASE

class BASE_EXPORT atomic_count {
public:

  explicit atomic_count( long v ) : value_( v )
  {}

  long operator++();
  long operator--();
//   long operator++()
//   {
//     return INTERLOCKED_INCREMENT( &value_ );
//   }
// 
//   long operator--()
//   {
//     return INTERLOCKED_DECREMENT( &value_ );
//   }

  operator long() const { return static_cast<long const volatile &>( value_ ); }

  void operator =(long value) { value_ = value; }

  atomic_count() : value_(0) {}
  atomic_count(const atomic_count& other) { value_ = (other); }

private:
  long value_;
}; // atomic_count


//////////////////////////////////////////////////////////////////////////

#define REF_COUNT_CONSTRUCT                   m_acRef(1)

#define DEFINE_REF_COUNT_RELEASE_DEL_SELF     \
private:                                      \
  atomic_count m_acRef;                       \
public:                                       \
  inline uint32_t RefCount() { return m_acRef; }   \
  inline void AddRef() { ++m_acRef;}          \
  inline virtual void Release() {             \
    ASSERT(m_acRef);                          \
    if (0 == --m_acRef) { delete this; }      \
  }                                           \


END_NAMESPACE_BASE

#endif // ATOMIC_COUNT_H_
