// vim:ts=4:sw=4:noet
#ifndef QHUB_FAST_SET_H
#define QHUB_FAST_SET_H

#include "qhub.h"

#if defined HAVE_TR1_UNORDERED_SET
# include <tr1/unordered_set>
# define QHUB_FAST_SET std::tr1::unordered_set
#else
# include <set>
# define QHUB_FAST_SET std::set
#endif

#endif // QHUB_FAST_SET_H
