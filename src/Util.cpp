// vim:ts=4:sw=4:noet
#include "Util.h"

#include "Logs.h"
#include "UserData.h"

#include <cassert>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;
using namespace qhub;

string const Util::emptyString;
int const Util::emptyInt = 0;
voidPtr const Util::emptyVoidPtr = NULL;
StringList const Util::emptyStringList;
UserData Util::data;

vector<uint8_t> Util::genRand(int bytes) throw() {
	int buf[(bytes + sizeof(int) - 1) / sizeof(int)];
	generate_n(buf, sizeof(buf)/sizeof(int), rand);
	uint8_t* b = reinterpret_cast<uint8_t*>(buf);
	return vector<uint8_t>(b, b + bytes);
}

void Util::daemonize() throw()
{
	switch(fork()) {
	case -1:
		Logs::err << "error with first fork(): " << errnoToString(errno) << endl;
		exit(EXIT_FAILURE);
	case 0:
		break;
	default:
		exit(EXIT_SUCCESS);
	}

	umask(0);
	if (chdir("/") < 0)
		Logs::err << "error with chdir():" << errnoToString(errno) << endl;

	if(setsid() < 0)
		Logs::err << "error with setsid(): " << errnoToString(errno) << endl;

	switch(fork()) {
	case -1:
		Logs::err << "error with second fork(): " << errnoToString(errno) << endl;
		exit(EXIT_FAILURE);
	case 0:
		break;
	default:
		exit(EXIT_SUCCESS);
	}

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	if(open("/dev/null", O_RDWR) < 0) {
		Logs::err << "error opening /dev/null:" << errnoToString(errno) << endl;
		exit(EXIT_FAILURE);
	}
	dup2(0, 1);
	dup2(0, 2);
}

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
				sl.push_back(emptyString);
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
