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
typedef int ERRNO;


class Util {
public:
	static string const emptyString;
	static int const emptyInt;
		
	static string toString(ERRNO err) { return strerror(err); };

	static string8 genRand192() {
		u_int8_t buf[24];
		for(unsigned i = 0; i < 24; i += sizeof(int)) {
			int r = rand();
			memcpy(buf + i, &r, sizeof(int) <= 24 - i ? sizeof(int) : 24 - i);
		}
		return string8(buf, 24);
	}
};

} //namespace qhub

#endif //_INCLUDED_UTIL_H_
