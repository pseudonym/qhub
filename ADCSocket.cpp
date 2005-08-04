// vim:ts=4:sw=4:noet
#include "ADCSocket.h"
#include "qhub.h"
#include "Util.h"
#include "Logs.h"
#include "error.h"
#include "ADC.h"

#define BUF_SIZE 1024

using namespace std;
using namespace qhub;


ADCSocket::ADCSocket(int fd, Domain domain, Hub* parent) throw()
		: Socket(fd, domain), state(PROTOCOL),
		readBuffer(new char[BUF_SIZE]), readPos(0), hub(parent)
{
	enableMe(ev_read);
	setNoLinger();
}

ADCSocket::ADCSocket(Hub* parent) throw()
		: Socket(), state(PROTOCOL), readBuffer(new char[BUF_SIZE]),
		readPos(0), hub(parent) {}

ADCSocket::~ADCSocket() throw()
{
	delete[] readBuffer;
}

//this is an ugly way to "factor out" the check for disconnectedness
void ADCSocket::handleOnRead()
{
	int ret = ::read(fd, readBuffer+readPos, BUF_SIZE-readPos);
	if(ret <= 0) {
		// we're done, either by EOF or error
		while(!queue.empty())
			queue.pop();
		if(ret == 0) {
			disconnect("normal disconnect"); // normal disconnect
		} else {
			disconnect(Util::errnoToString(errno)); // error
		}
		return;
	}
	char* l = readBuffer;
	char* r = readBuffer + readPos + ret;
	char* tmp = readBuffer + readPos;
	while((tmp = find(tmp, r, '\n')) != r) {
		string line(l, tmp);
		l = ++tmp;
		if(line.empty())
			continue;	//ignore keepalives
		StringList sl = Util::stringTokenize(line);
		for(StringList::iterator i = sl.begin(); i != sl.end(); ++i)
			*i = ADC::CSE(*i);
		line += '\n';
		Logs::line << getFd() << "<< " << line;
		onLine(sl, line);
		if(disconnected)
			return;
	}
	readPos = r - l;
	if(readPos == BUF_SIZE)
		throw parse_error("line limit of 1024 characters exceeded");

	::memmove(readBuffer, l, readPos);
}

bool ADCSocket::onRead() throw()
{
	try {
		handleOnRead();
	} catch(const exception& e) {
		doError(e.what());
		disconnect(e.what());
	}
	// do this as the last thing before we return, see notes in realDisconnect
	if(disconnected && queue.empty()){
		realDisconnect();
		return false;
	}
	return true;
}

void ADCSocket::onWrite() throw()
{
	partialWrite();
	if(queue.empty()) {
		disableMe(ev_write);
		writeEnabled = false;

		// Nothing left to write.. kill us
		// dont do anything after realDisconnect, see notes there
		if(disconnected)
			realDisconnect();
	}
}

void ADCSocket::onAlarm() throw()
{
	if(!disconnected) {
		// Do a silent disconnect. We don't want to show our protocol to an unknown peer.
		//(we could try sending an NMDC-style message here)
		disconnect("send timeout");
	} else {
		//last thing before we return control to event-system: the right thing to do! :)
		realDisconnect();
	}
}

void ADCSocket::disconnect(string const& msg)
{
	Socket::disconnect(msg);
	onDisconnected(msg);
}

void ADCSocket::realDisconnect()
{
	//this will "delete this", meaning: we CANNOT access anything via this-> anymore, and
	//we cannot dispatch any more member calls, and we cannot even access "this" anymore

	//this should not be necessary: the assumptions above forbid it
	assert(fd != -1 && "Assumptions of 'delete this' usage was probably broken!");

	Logs::stat << format("Real Disconnect %d %p\n") % fd % this;
	if(writeEnabled) {
		disableMe(ev_write);
	}
	close(fd);
	fd = -1;

	delete this;
}
