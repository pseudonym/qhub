// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_ADC_H_
#define _INCLUDED_ADC_H_

#include <string>
#include "Util.h"

using namespace std;

namespace qhub {

class ADC {
public:
	static string ESC(string const& in) throw() {
		string tmp;
		tmp.reserve(255);
		for(string::const_iterator i = in.begin(); i != in.end(); ++i) {
			switch(*i) {
			case ' ':
			case '\n':
			case '\\':
				tmp += '\\';
			default:
				tmp += *i;
			}
		}
		return tmp;
	}
	static string CSE(string const& in) throw() {
		string tmp;
		fprintf(stderr, "tmp %i %i\n", tmp.capacity(), in.length());
		tmp.reserve(in.length());
		for(string::const_iterator i = in.begin(); i != in.end(); ++i) {
			if(*i == '\\') {
				++i;
				assert(i != in.end()); // shouldn't happen if we parsed input correctly earlier
			}
			tmp += *i;
		}
		return tmp;
	}
	static string& toString(StringList const& sl, string& out) throw() {
		assert(!sl.empty());
		StringList::const_iterator i = sl.begin();
		out = ESC(*i);
		for(++i; i != sl.end(); ++i) {
			out += ' ' + ESC(*i);
		}
		out += '\n';
		return out;
	}
};

} //namespace qhub

#endif //_INCLUDED_ADC_H_
