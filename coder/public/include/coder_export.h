/*

*/

#ifndef CODER_EXPORT_H_
#define CODER_EXPORT_H_

#if defined(CODER_IMPLEMENTATION)
#define CODER_EXPORT                     SYMBOL_EXPORT
#define CODER_EXPORT_PRIVATE             SYMBOL_EXPORT
#else
#define CODER_EXPORT
#define CODER_EXPORT_PRIVATE
#endif  // defined(CODER_IMPLEMENTATION)

#endif // CODER_EXPORT_H_
