// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_UTIL_H_
#define _INCLUDED_UTIL_H_

#include <string>
#include <vector>
#include <stdlib.h>
#include "string8.h"

namespace qhub {

using namespace std;

typedef vector<string> StringList;


class Util {
public:
	static string const emptyString;
	static int const emptyInt;
		
	static string errnoToString(int err) throw()
	{
		return strerror(err);
	};
	
	static string toString(void* p) throw() {
		char buf[32];
		snprintf(buf, 32, "%p", p);
		return buf;
	};
	static string toString(int i) throw() {
		char buf[32];
		snprintf(buf, 32, "%i", i);
		return buf;
	};

	static string8 genRand192() throw() {
		u_int8_t buf[24];
		for(unsigned i = 0; i < 24; i += sizeof(int)) {
			int r = rand();
			memcpy(buf + i, &r, sizeof(int) <= 24 - i ? sizeof(int) : 24 - i);
		}
		return string8(buf, 24);
	};

	static StringList stringTokenize(string const& msg, char token = ' ') throw()
	{
		StringList sl;
		string::size_type i, j = 0;
		while((i = msg.find(token, j)) != string::npos) {
			sl.push_back(msg.substr(j, i - j));
			j = i + 1;
		}
		sl.push_back(msg.substr(j));
		return sl;
	};
	static StringList lazyStringTokenize(string const& msg, char token = ' ') throw()
	{
		StringList sl = stringTokenize(msg, token);
		for(int i = sl.size() - 1; i >= 0; --i) {
			if(sl[i].empty())
				sl.erase(sl.begin() + i);
		}
		return sl;
	};
		
};

} //namespace qhub

#endif //_INCLUDED_UTIL_H_
