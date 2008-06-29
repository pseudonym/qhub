#ifndef QHUB_COMPAT_HASHTABLE_H
#define QHUB_COMPAT_HASHTABLE_H

/*
 * This assumes that hash_set is defined in the same way as hash_map.
 * Probably a fairly safe assumption, and if it's not, I want whatever
 * the library provider is smoking.
 */

#include "qhub.h"

#ifdef HAVE_STD__HASH_MAP
#include <hash_map>
#include <hash_set>
#endif

#ifdef HAVE___GNU_CXX__HASH_MAP
#include <hash_map>
#include <hash_set>
namespace std {
using __gnu_cxx::hash_map;
using __gnu_cxx::hash_set;
}
#endif

#ifdef HAVE_STD__EXT_HASH_MAP
#include <ext/hash_map>
#include <ext/hash_set>
#endif

#ifdef HAVE___GNU_CXX__EXT_HASH_MAP
#include <ext/hash_map>
#include <ext/hash_set>
namespace std {
using __gnu_cxx::hash_map;
using __gnu_cxx::hash_set;
}
#endif


/** BEGIN FIX **/
#include <string>

#if defined(HAVE___GNU_CXX__EXT_HASH_MAP) || defined(HAVE___GNU_CXX__HASH_MAP)
namespace __gnu_cxx
#else
namespace std
#endif
{
template<> struct hash< std::string >
{
	size_t operator()( const std::string& x ) const
	{
		return hash< const char* >()( x.c_str() );
	}
};
}
/** END FIX **/


#endif //QHUB_COMPAT_HASHTABLE_H
