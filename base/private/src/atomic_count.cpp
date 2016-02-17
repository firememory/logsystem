/*

*/
#include "atomic_count.h"

#if defined(PLATFORM_API_WINDOWS)

# include <windows.h>
# define INTERLOCKED_INCREMENT InterlockedIncrement
# define INTERLOCKED_DECREMENT InterlockedDecrement

#else
# error "INTERLOCKED is unknown"
#endif


BEGIN_NAMESPACE_BASE

long atomic_count::operator++()
{
  return INTERLOCKED_INCREMENT( &value_ );
}

long atomic_count::operator--()
{
  return INTERLOCKED_DECREMENT( &value_ );
}

END_NAMESPACE_BASE
