// vim:ts=4:sw=4:noet
#include "ADCSocket.h"
#include "qhub.h"
#include "Util.h"
#include "ADC.h"

// fixme, do we need a start_buffer? we're not 'growing' it
#define START_BUFFER 512
#define READ_SIZE 512

// fixme, should be configurable or something
#define MAX_DATALEN 128		// arguments
#define MAX_LINELEN 1024	// one line

using namespace std;
using namespace qhub;

ADCSocket::ADCSocket(int fd, Domain domain, Hub* parent) throw()
		: Socket(fd, domain), readBufferSize(START_BUFFER), readBuffer(new unsigned char[readBufferSize]),
		state(NORMAL), escaped(false), hub(parent)
{
	enable_fd(fd, OOP_READ, this);
	setNoLinger();
}

ADCSocket::~ADCSocket() throw()
{
	delete[] readBuffer;
}

//this is an ugly way to "factor out" the check for disconnectedness
void ADCSocket::handleOnRead() throw()
{
	//fprintf(stderr, "ADCSocket::onRead\n");
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
				if(queue.empty()){
					return;
				}
			}

			switch(*p) {
			case '\\':
				escaped = !escaped;
				break;
			case '\n':
			case ' ':
				if(!escaped) {
					if(state == NORMAL) {
						data.push_back(ADC::CSE(string((char const*)s, p - s)));
						// huh? how come "/raw \n" doesn't work the first time, unless we add an fprintf here??
						s = p + 1;
						++dataLen;
						if(dataLen > MAX_DATALEN) {
							// disconnect is mandatory here, or we would need to seek
							// through the bad command to start at the next
							doError("Max arguments of 128 in command exceeded");
							disconnect("Max arguments of 128 in command exceeded");
							if(queue.empty()){
								return;
							}
						}
					} else if(state == PARTIAL) {
						data.back().append((char const*)s, p - s);
						data.back() = ADC::CSE(data.back()); // don't forget to unescape
						state = NORMAL;
						s = p + 1;
					} else {
						assert(0);
					}
					if(*p == '\n') {
						// Call our dear virtual command handler
						if(raw.empty()) {
							if(f != p) // !empty, if empty, then ignore
								onLine(data, string((char const*)f, p - f + 1));
						} else {
							onLine(data, raw + string((char const*)f, p - f + 1));
						}
						if(disconnected && queue.empty()){
							return;
						}
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
}

void ADCSocket::onRead() throw()
{
	handleOnRead();
	//do this as the last thing before we return, see notes in realDisconnect
	if(disconnected && queue.empty())
		realDisconnect();
}

void ADCSocket::onWrite() throw()
{
	//fprintf(stderr, "ADCSocket::onWrite\n");
	partialWrite();
	if(queue.empty()) {
		cancel_fd(fd, OOP_WRITE);
		writeEnabled = false;

		// Nothing left to write.. kill us
		//dont do anything after realDisconnect, see notes there
		if(disconnected)
			realDisconnect();
	}
}

void ADCSocket::disconnect(string const& msg)
{
	assert(!disconnected);
	Socket::disconnect();
	onDisconnected(Util::emptyString);
	// this::onRead calls:
	//   ADCClient::onLine -> this::disconnect -> this::realDisconnect
	// this::onRead tries to read/modify variables... not good...
	// so we can't have realDisconnect here. Test for queue.empty() after
	// every call to ADCClient::onLine or this::disconnect and call a
	// realDisconnect() (to avoid stalling disconnections).
}

void ADCSocket::realDisconnect()
{
	//this will "delete this", meaning: we CANNOT access anything via this-> anymore, and
	//we cannot dispatch any more member calls, and we cannot even access "this" anymore

	//this should not be necessary: the assumptions above forbid it
	assert(fd != -1 && "Assumptions of 'delete this' usage was probably broken!");

	fprintf(stderr, "Real Disconnect %d %p\n", fd, this);
	if(writeEnabled) {
		cancel_fd(fd, OOP_WRITE);
	}
	close(fd);
	fd = -1;

	delete this;
}
