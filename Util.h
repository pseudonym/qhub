// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_UTIL_H_
#define _INCLUDED_UTIL_H_

#include <string>
#include <vector>

namespace qhub {

using namespace std;

typedef vector<string> StringList;

class Util {
public:
	static string emptyString;
		
	typedef int ERRNO;
	static string toString(ERRNO err) { return strerror(err); };
};

} //namespace qhub

#endif //_INCLUDED_UTIL_H_
