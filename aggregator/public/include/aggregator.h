/*

*/

#ifndef AGGREGATOR_H_
#define AGGREGATOR_H_

//////////////////////////////////////////////////////////////////////////
/*

*/
#include <config.h>
#include "error.h"


#if HAS_NAMESPACE
# define NAMESPACE_AGGREGATOR_NAME      aggregator
# define BEGIN_NAMESPACE_AGGREGATOR     BEGIN_NAMESPACE namespace NAMESPACE_AGGREGATOR_NAME {
# define END_NAMESPACE_AGGREGATOR       } END_NAMESPACE
# define USING_NAMESPACE_AGGREGATOR     using namespace NAMESPACE_NAME::NAMESPACE_AGGREGATOR_NAME
# define USING_AGGREGATOR(x)            using NAMESPACE_AGGREGATOR_NAME::x
#else
# define BEGIN_NAMESPACE_AGGREGATOR
# define END_NAMESPACE_AGGREGATOR
# define USING_NAMESPACE_AGGREGATOR
# define USING_AGGREGATOR(x)
#endif

#endif // AGGREGATOR_H_
