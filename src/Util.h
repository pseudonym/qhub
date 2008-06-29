// vim:ts=4:sw=4:noet
#ifndef QHUB_UTIL_H
#define QHUB_UTIL_H

#include <string>
#include <vector>
#include "compat_hashtable.h"
#include <boost/lexical_cast.hpp>
#include <cerrno>

namespace qhub {

class UserData;

typedef std::vector<std::string> StringList;
typedef std::hash_map<std::string,std::string> StringMap;
typedef void* voidPtr;

template<class T>
struct PtrHash {
	size_t operator()(T*const e) const
	{ return (size_t)e; }
};

class Util {
public:
	static std::string const emptyString;
	static int const emptyInt;
	static voidPtr const emptyVoidPtr;
	static StringList const emptyStringList;
	static UserData data;

	static std::string errnoToString(int err = errno) throw()
	{
		return strerror(err);
	}

	// seems to make sense to just make all these into one function
	template<typename To, typename From>
	static To convert(const From& v) throw(boost::bad_lexical_cast)
	{
		return boost::lexical_cast<To>(v);
	}

	static int toInt(char const* p) throw(boost::bad_lexical_cast)
	{
		return boost::lexical_cast<int>(p);
	}
	static int toInt(std::string const& s) throw(boost::bad_lexical_cast)
	{
		return boost::lexical_cast<int>(s);
	}

	template<class T>
	static std::string toString(const T& val) throw(boost::bad_lexical_cast)
	{
		return boost::lexical_cast<std::string>(val);
	}

	static std::vector<uint8_t> genRand(int bytes) throw();
	static void daemonize() throw();

	static StringList stringTokenize(std::string const& msg, char token = ' ') throw();
	static StringList lazyStringTokenize(std::string const& msg, char token = ' ') throw();
	static StringList lazyQuotedStringTokenize(std::string const& msg) throw(); // token = ' ', quote = '"'
};

} //namespace qhub

#endif //QHUB_UTIL_H
