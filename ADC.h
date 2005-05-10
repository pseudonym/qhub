// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_ADC_H_
#define _INCLUDED_ADC_H_

#include <string>
#include "Util.h"
#include "error.h"

using namespace std;

namespace qhub {

class ADC {
public:
	static string ESC(string const& in) throw();
	static string CSE(string const& in) throw(Exception);
	static string& toString(StringList const& sl, string& out) throw();
	static bool checkCID(const string& cid) throw();
};

} //namespace qhub

#endif //_INCLUDED_ADC_H_
