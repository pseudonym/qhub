#ifndef QHUB_TYPES_H
#define QHUB_TYPES_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STDINT_H
#include "stdint.h"
//nothing to do, it defines int64_t and uint64_t
//though C99 specifies only "long long int"
#else //HAVE_STDINT_H
typedef long long int int64_t;
typedef unsigned long long int uint64_t;
#endif //HAVE_STDINT_H

#endif // QHUB_TYPES_H
