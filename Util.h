// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_UTIL_H_
#define _INCLUDED_UTIL_H_

#include <string>
#include <vector>
#include <cstdlib>
#include "compat_hash_map.h"
#include <boost/lexical_cast.hpp>
#include "string8.h"
#include "error.h"

namespace qhub {

using namespace std;

typedef vector<string> StringList;
typedef hash_map<string,string> StringMap;
typedef void* voidPtr;


class Util {
public:
	static string const emptyString;
	static int const emptyInt;
	static voidPtr const emptyVoidPtr;
	static StringList const emptyStringList;

	static string errnoToString(int err) throw()
	{
		return strerror(err);
	};

	static int toInt(char const* p) throw(boost::bad_lexical_cast) {
		return boost::lexical_cast<int>(p);
	};
	static int toInt(string const& s) throw(boost::bad_lexical_cast) {
		return boost::lexical_cast<int>(s);
	};

	template<class T>
	static string toString(const T& val) throw(boost::bad_lexical_cast)
	{
		return boost::lexical_cast<string>(val);
	}

	static string8 genRand192() throw() {
		u_int8_t buf[24];
		for(unsigned i = 0; i < 24; i += sizeof(int)) {
			int r = rand();
			memcpy(buf + i, &r, sizeof(int) <= 24 - i ? sizeof(int) : 24 - i);
		}
		return string8(buf, 24);
	};

	static StringList stringTokenize(string const& msg, char token = ' ') throw();
	static StringList lazyStringTokenize(string const& msg, char token = ' ') throw();
	static StringList lazyQuotedStringTokenize(string const& msg) throw(); // token = ' ', quote = '"'

};

} //namespace qhub

#endif //_INCLUDED_UTIL_H_
