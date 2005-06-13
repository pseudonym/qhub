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
	enable_fd(fd, OOP_READ, this);
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
	readPos += ret;
	char* l = readBuffer;
	char* r = readBuffer + readPos;
	char* tmp;
	while((tmp = find(l, r, '\n')) != r) {
		string line(l, ++tmp);
		if(line.empty())
			continue;	//ignore keepalives
		StringList sl = Util::stringTokenize(line);
		sl.back().resize(sl.back().size()-1);	//remove ending newline
		for(StringList::iterator i = sl.begin(); i != sl.end(); ++i)
			*i = ADC::CSE(*i);
		Logs::line << getFd() << "<< " << line << endl;
		onLine(sl, line);
		if(disconnected)
			return;
		l = tmp;
	}
	readPos = r - l;
	if(readPos == BUF_SIZE)
		throw Exception("line limit of 1024 characters exceeded");

	::memmove(readBuffer, l, readPos);
}

void ADCSocket::onRead() throw()
{
	try {
		handleOnRead();
	} catch(const exception& e) {
		doError(e.what());
		disconnect(e.what());
	}
	// do this as the last thing before we return, see notes in realDisconnect
	if(disconnected && queue.empty())
		realDisconnect();
}

void ADCSocket::onWrite() throw()
{
	partialWrite();
	if(queue.empty()) {
		cancel_fd(fd, OOP_WRITE);
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
		cancel_fd(fd, OOP_WRITE);
	}
	close(fd);
	fd = -1;

	delete this;
}
