/*

*/

#ifndef NET_EXPORT_H_
#define NET_EXPORT_H_

#if defined(NET_IMPLEMENTATION)
#define NET_EXPORT                     SYMBOL_EXPORT
#define NET_EXPORT_PRIVATE             SYMBOL_EXPORT
#else
#define NET_EXPORT
#define NET_EXPORT_PRIVATE
#endif  // defined(NET_IMPLEMENTATION)

#endif // NET_EXPORT_H_
