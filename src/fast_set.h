// vim:ts=4:sw=4:noet
#ifndef QHUB_FAST_SET_H
#define QHUB_FAST_SET_H

#include "qhub.h"

// currently not tested for in configure, but might as well future proof
#if defined HAVE_UNORDERED_SET
# include <unordered_set>
#elif defined HAVE_TR1_UNORDERED_SET
# include <tr1/unordered_set>
#else
# include <set>
#endif

namespace qhub {

// would be much better with template typedefs... for now, do it this way
template<typename Key>
struct fast_set {
	typedef
#if defined HAVE_UNORDERED_SET
	std::unordered_set<Key>
#elif defined HAVE_TR1_UNORDERED_SET
	std::tr1::unordered_set<Key>
#else
	std::set<Key>
#endif
	type;
};

} // namespace qhub

#endif // QHUB_FAST_SET_H
