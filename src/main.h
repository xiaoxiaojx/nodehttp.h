#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define ASSERT(expr)                                      \
 do {                                                     \
  if (!(expr)) {                                          \
    fprintf(stderr,                                       \
            "Assertion failed in %s on line %d: %s\n",    \
            __FILE__,                                     \
            __LINE__,                                     \
            #expr);                                       \
    abort();                                              \
  }                                                       \
 } while (0)

#define DEFAULT_PORT 3000

#define DEFAULT_BACKLOG 128