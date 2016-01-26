/*

*/

#ifndef BASE_EXPORT_H_
#define BASE_EXPORT_H_

#if defined(BASE_IMPLEMENTATION)
#define BASE_EXPORT                     SYMBOL_EXPORT
#define BASE_EXPORT_PRIVATE             SYMBOL_EXPORT
#else
#define BASE_EXPORT
#define BASE_EXPORT_PRIVATE
#endif  // defined(BASE_IMPLEMENTATION)

#endif // BASE_EXPORT_H_
