// vim:ts=4:sw=4:noet
#include "ADC.h"

#include "error.h"
#include "Encoder.h"
#include "Util.h"

#include <cassert>

using namespace qhub;
using namespace std;

string ADC::ESC(string const& in) throw()
{
	string tmp;
	tmp.reserve(static_cast<size_t>(in.length()*1.2));
	for(string::const_iterator i = in.begin(); i != in.end(); ++i) {
		switch(*i) {
		case ' ':
			tmp += "\\s";
			break;
		case '\n':
			tmp += "\\n";
			break;
		case '\\':
			tmp += '\\';
			// fall through because it's l33t
		default:
			tmp += *i;
		}
	}
	return tmp;
}

string ADC::CSE(string const& in) throw(parse_error)
{
	string tmp;
	tmp.reserve(in.length());
	for(string::const_iterator i = in.begin(); i != in.end(); ++i) {
		if(*i == '\\') {
			++i;
			if(i == in.end())
				throw parse_error("ADC::CSE invalid escape:\n\t" + in);
			switch (*i) {
			case 'n':
				tmp += '\n';
				break;
			case 's':
				tmp += ' ';
				break;
			case '\\':
				tmp += '\\';
				break;
			default:
				throw parse_error("ADC::CSE invalid escape:\n\t" + in);
			}
			continue;
		}
		tmp += *i;
	}
	return tmp;
}
string ADC::fromSid(sid_type s) throw() {
	string str(4, '0');
	for(int i = 3; i >= 0; i--)
		str[3-i] = Encoder::toBase32((s >> (i*5)) & 31);
	return str;
}

sid_type ADC::toSid(const string& str) throw(parse_error) {
	if(str.size() != 4)
		throw parse_error("invalid sid parameter");
	sid_type s = 0;
	for(int i = 0; i < 4; i++) {
		int8_t t = Encoder::fromBase32(str[i]);
		if(t == -1)
			throw parse_error("invalid Base32 characters in sid");
		s <<= 5;
		s |= t;
	}
	return s;
}

string& ADC::toString(StringList const& sl, string& out) throw()
{
	assert(!sl.empty());
	out.clear();
	for(StringList::const_iterator i = sl.begin(); i != sl.end(); ++i)
		out += ESC(*i) + ' ';
	out[out.size()-1] = '\n';
	return out;
}
