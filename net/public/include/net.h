/*

*/

#ifndef NET_H_
#define NET_H_

#include <config.h>
#include "error.h"


#if HAS_NAMESPACE
# define NAMESPACE_NET_NAME             net
# define BEGIN_NAMESPACE_NET            BEGIN_NAMESPACE namespace NAMESPACE_NET_NAME {
# define END_NAMESPACE_NET              } END_NAMESPACE
# define USING_NAMESPACE_NET            using namespace NAMESPACE_NAME::NAMESPACE_NET_NAME
# define USING_NET(x)                   using NAMESPACE_NET_NAME::x
#else
# define BEGIN_NAMESPACE_NET
# define END_NAMESPACE_NET
# define USING_NAMESPACE_NET
# define USING_NET(x)
#endif

#endif // NET_H_
