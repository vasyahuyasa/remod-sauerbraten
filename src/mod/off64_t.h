#ifndef __MINGW32__

#include <sys/types.h>

#ifndef off64_t
#ifdef _off64_t
typedef _off64_t off64_t;
#elseif __off64_t
typedef __off64_t off64_t;
#else
typedef long long off64_t;
#endif
#endif

#ifndef off_t
#ifdef _off_t
typedef _off_t off_t;
#elseif __off_t
typedef __off_t off_t;
#else
typedef long int off_t;
#endif
#endif

#ifndef mode_t
#ifdef _mode_t
typedef _mode_t mode_t
#else
typedef unsigned short mode_t
#endif
#endif

#endif