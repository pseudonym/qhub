#ifndef __COMPAT_HASH_
#define __COMPAT_HASH_

#include <string>

#include "config.h"

#ifdef HAVE_STD__HASH_MAP
#include <hash_map>
#endif

#ifdef HAVE___GNU_CXX__HASH_MAP
#include <hash_map>
namespace std {
using __gnu_cxx::hash_map;
};
#endif

#ifdef HAVE_STD__EXT_HASH_MAP
#include <ext/hash_map>
#endif

#ifdef HAVE___GNU_CXX__EXT_HASH_MAP
#include <ext/hash_map>
namespace std {
using __gnu_cxx::hash_map;
}
#endif


/** BEGIN FIX **/
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


#endif


