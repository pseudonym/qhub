// vim:ts=4:sw=4:noet
#include "ADCSocket.h"
#include "qhub.h"
#include "Util.h"

// fixme, do we need a start_buffer? we're not 'growing' it
#define START_BUFFER 512
#define READ_SIZE 512

// fixme, should be configurable or something
#define MAX_DATALEN 128		// arguments
#define MAX_LINELEN 1024	// one line

using namespace std;
using namespace qhub;

string ADCSocket::esc(string const& in)
{
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

string ADCSocket::cse(string const& in)
{
	string tmp;
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

ADCSocket::ADCSocket(int fd, Domain domain, Hub* parent) throw()
: Socket(fd, domain), readBufferSize(START_BUFFER), readBuffer(new unsigned char[readBufferSize]),
		state(NORMAL), escaped(false), hub(parent)
{
	enable(fd, OOP_READ, this);
	setNoLinger();
}

ADCSocket::~ADCSocket() throw()
{
	delete[] readBuffer;
	realDisconnect();
}

void ADCSocket::on_read()
{
	int ret = read(fd, readBuffer, readBufferSize);
	if(ret <= 0) {
		// we're done, either by EOF or error
		while(!queue.empty())
			queue.pop();
		if(ret == 0) {
			disconnect(); // normal disconnect
		} else {
			disconnect(Util::errnoToString(errno)); // error
		}
	} else {
		unsigned char *p = readBuffer;	// cur
		unsigned char *e = p + ret;		// end
		unsigned char *f = p;			// after LF
		unsigned char *s = p;			// after SP/LF
		unsigned lineLen = raw.length();
		unsigned dataLen = data.size();
		for(; p != e; ++p, ++lineLen) {
			if(lineLen > MAX_LINELEN) {
				// disconnect is mandatory here, or we would need to seek
				// through the bad command to start at the next
				doError("Max input of 1024 chars in command exceeded");
				disconnect("Max input of 1024 chars in command exceeded");
				return;
			}
			
			switch(*p) {
			case '\\':
					escaped = !escaped;
					break;
			case '\n':
			case ' ':
				if(!escaped) {
					if(state == NORMAL) {
						data.push_back(cse(string((char const*)s, p - s)));
						// huh? how come "/raw \n" doesn't work the first time, unless we add an fprintf here??
						s = p + 1;
						++dataLen;
						if(dataLen > MAX_DATALEN) {
							// disconnect is mandatory here, or we would need to seek
							// through the bad command to start at the next
							doError("Max arguments of 128 in command exceeded");
							disconnect("Max arguments of 128 in command exceeded");
							return;
						}
					} else if(state == PARTIAL) {
						data.back().append((char const*)s, p - s);
						data.back() = cse(data.back()); // don't forget to unescape
						state = NORMAL;
						s = p + 1;
					} else {
						assert(0);
					}
					if(*p == '\n') {
						// Call our dear virtual command handler
						if(raw.empty())
							onLine(data, string((char const*)f, p - f + 1));
						else
							onLine(data, raw + string((char const*)f, p - f + 1));
						data.clear();
						raw.clear();
						f = s;
						lineLen = 0;
					}
				}
				// fall through
			default:
				escaped = false;
			}
		}
		// Add leftover data
		if(s != p) {
			if(state == NORMAL) {
				data.push_back(string((char const*)s, p - s));
				state = PARTIAL;
			} else if(state == PARTIAL) {
				data.back().append((char const*)s, p - s);
			} else {
				assert(0);
			}
		}
		if(f != p)
			raw += string((char const*)f, p - f);
	}

	// Check if we're disconnected
	if(disconnected && queue.empty()) {
		realDisconnect();
	}
}

void ADCSocket::on_write()
{
	//fprintf(stderr, "On_write\n");
	partialWrite();
	if(queue.empty()){
		cancel(fd, OOP_WRITE);
		writeEnabled = false;
	}

	// Check if we're disconnected
	if(disconnected && queue.empty()) {
		realDisconnect();
	}
}

void ADCSocket::realDisconnect()
{
	if(fd == -1) {
		// if we're already disconnecting, we would loop trying to delete ourselves again
		// as we're called from the destructor
		return;
	}

	if(!disconnected) {
		disconnect(); // notify others that we're dying
	}
		
	fprintf(stderr, "Real Disconnect %d %p\n", fd, this);
	close(fd);
	if(writeEnabled){
		cancel(fd, OOP_WRITE);
	}

	fd = -1;
	// deleting us
	delete this;
}
