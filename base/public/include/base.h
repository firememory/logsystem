/*

*/

#ifndef BASE_H_
#define BASE_H_

#include <config.h>
#include "error.h"


#if HAS_NAMESPACE
# define NAMESPACE_BASE_NAME            base
# define BEGIN_NAMESPACE_BASE           BEGIN_NAMESPACE namespace NAMESPACE_BASE_NAME {
# define END_NAMESPACE_BASE             } END_NAMESPACE
# define USING_NAMESPACE_BASE           using namespace NAMESPACE_NAME::NAMESPACE_BASE_NAME
# define USING_BASE(x)                  using NAMESPACE_BASE_NAME::x
#else
# define BEGIN_NAMESPACE_BASE
# define END_NAMESPACE_BASE
# define USING_NAMESPACE_BASE
# define USING_BASE(x)
#endif

#endif // BASE_H_
