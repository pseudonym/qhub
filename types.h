#ifndef _types_h_
#define _types_h_

#include "config.h"

#ifdef HAVE_STDINT_H
#include "stdint.h"
//nothing to do, it defines int64_t and uint64_t
//though C99 specifies only "long long int"
#else //HAVE_STDINT_H
typedef long long int int64_t;
typedef unsigned long long int uint64_t;
#endif //HAVE_STDINT_H

#endif //_types_h_h
