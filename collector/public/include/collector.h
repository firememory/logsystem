/*

*/

#ifndef COLLECTOR_H_
#define COLLECTOR_H_

//////////////////////////////////////////////////////////////////////////
/*

*/
#include <config.h>
#include "error.h"


#if HAS_NAMESPACE
# define NAMESPACE_COLLECTOR_NAME       collector
# define BEGIN_NAMESPACE_COLLECTOR      BEGIN_NAMESPACE namespace NAMESPACE_COLLECTOR_NAME {
# define END_NAMESPACE_COLLECTOR        } END_NAMESPACE
# define USING_NAMESPACE_COLLECTOR      using namespace NAMESPACE_NAME::NAMESPACE_COLLECTOR_NAME
# define USING_COLLECTOR(x)             using NAMESPACE_COLLECTOR_NAME::x
#else
# define BEGIN_NAMESPACE_COLLECTOR
# define END_NAMESPACE_COLLECTOR
# define USING_NAMESPACE_COLLECTOR
# define USING_COLLECTOR(x)
#endif

#endif // COLLECTOR_H_
