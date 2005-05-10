// vim:ts=4:sw=4:noet
#include <cstring>
#include <cassert>
#include <cstdio>

#include "Util.h"

using namespace std;
using namespace qhub;

string const Util::emptyString;
int const Util::emptyInt = 0;
voidPtr const Util::emptyVoidPtr = NULL;
StringList const Util::emptyStringList;

StringList Util::stringTokenize(string const& msg, char token /*= ' '*/) throw()
{
	StringList sl;
	string::size_type i, j = 0;
	while((i = msg.find(token, j)) != string::npos) {
		sl.push_back(msg.substr(j, i - j));
		j = i + 1;
	}
	sl.push_back(msg.substr(j));
	return sl;
}

StringList Util::lazyStringTokenize(string const& msg, char token /*= ' '*/) throw()
{
	StringList sl = stringTokenize(msg, token);
	for(int i = sl.size() - 1; i >= 0; --i) {
		if(sl[i].empty())
			sl.erase(sl.begin() + i);
	}
	return sl;
}

StringList Util::lazyQuotedStringTokenize(string const& msg) throw() // token = ' ', quote = '"'
{
	StringList sl;
	int state = 0; // 0 = space, 1 = data, 2 = quoted-data
	string empty;
	for(string::const_iterator i = msg.begin(); i != msg.end(); ++i) {
		switch(*i) {
		case ' ':
			if(state == 0) {
				// do nada
			} else if(state == 1) {
				state = 0;
			} else if(state == 2) {
				sl.back().append(string(1, ' '));
			} else {
				assert(0);
			}
			break;
		case '"':
			if(state == 0) {
				sl.push_back(empty);
				state = 2;
			} else if(state == 1) {
				state = 2;
			} else if(state == 2) {
				state = 1;
			} else {
				assert(0);
			}
			break;
		default:
			if(state == 0) {
				state = 1;
				sl.push_back(string(1, *i));
			} else if(state == 1 || state == 2) {
				sl.back().append(string(1, *i));
			} else {
				assert(0);
			}
			break;
		}
	}
	return sl;
}
