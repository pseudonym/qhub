// vim:ts=4:sw=4:noet
#ifndef QHUB_FAST_MAP_H
#define QHUB_FAST_MAP_H

#include "qhub.h"

#if defined HAVE_TR1_UNORDERED_MAP
# include <tr1/unordered_map>
# define QHUB_FAST_MAP std::tr1::unordered_map
#else
# include <map>
# define QHUB_FAST_MAP std::map
#endif

#endif // QHUB_FAST_MAP_H
