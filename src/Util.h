// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_UTIL_H_
#define _INCLUDED_UTIL_H_

#include <string>
#include <vector>
#include <cstdlib>
#include "compat_hash_map.h"
#include <boost/lexical_cast.hpp>
#include "error.h"
#include <cerrno>

using namespace std;

namespace qhub {

class UserData;

typedef vector<string> StringList;
typedef hash_map<string,string> StringMap;
typedef void* voidPtr;


class Util {
public:
	static string const emptyString;
	static int const emptyInt;
	static voidPtr const emptyVoidPtr;
	static StringList const emptyStringList;
	static UserData data;

	static string errnoToString(int err = errno) throw()
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

	static vector<u_int8_t> genRand(int bytes) throw();
	static void daemonize() throw();

	static StringList stringTokenize(string const& msg, char token = ' ') throw();
	static StringList lazyStringTokenize(string const& msg, char token = ' ') throw();
	static StringList lazyQuotedStringTokenize(string const& msg) throw(); // token = ' ', quote = '"'

};

} //namespace qhub

#endif //_INCLUDED_UTIL_H_
