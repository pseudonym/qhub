// vim:ts=4:sw=4:noet
#ifndef QHUB_FAST_MAP_H
#define QHUB_FAST_MAP_H

#include "qhub.h"

// currently not tested for in configure, but might as well future proof
#if defined HAVE_UNORDERED_MAP
# include <unordered_map>
#elif defined HAVE_TR1_UNORDERED_MAP
# include <tr1/unordered_map>
#else
# include <map>
#endif

namespace qhub {

// would be much better with template typedefs... for now, do it this way
template<typename Key, typename Data>
struct fast_map {
	typedef
#if defined HAVE_UNORDERED_MAP
	std::unordered_map<Key,Data>
#elif defined HAVE_TR1_UNORDERED_MAP
	std::tr1::unordered_map<Key,Data>
#else
	std::map<Key,Data>
#endif
	type;
};

} // namespace qhub

#endif // QHUB_FAST_MAP_H
