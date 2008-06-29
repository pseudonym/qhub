// vim:ts=4:sw=4:noet
#include "ADCSocket.h"

#include "error.h"
#include "Command.h"
#include "ConnectionBase.h"
#include "Logs.h"

#define BUF_SIZE 1024

using namespace std;
using namespace qhub;

ADCSocket::ADCSocket(int fd, Domain domain) throw()
		: Socket(fd, domain),
		readBuffer(new char[BUF_SIZE]), readPos(0), conn(NULL)
{
	EventManager::instance()->enableRead(getFd(), this);
	setNoLinger();
}

ADCSocket::ADCSocket() throw()
		: Socket(), readBuffer(new char[BUF_SIZE]),
		readPos(0), conn(NULL) {}

ADCSocket::~ADCSocket() throw()
{
	EventManager::instance()->removeTimer(this);
	delete[] readBuffer;
}

//this is an ugly way to "factor out" the check for disconnectedness
void ADCSocket::handleOnRead()
{
	int ret = read(readBuffer+readPos, BUF_SIZE-readPos);

	char* l = readBuffer;
	char* r = readBuffer + readPos + ret;
	char* tmp = readBuffer + readPos;

	while((tmp = find(l, r, '\n')) != r) {
		if(tmp == l) {
			l = tmp+1;
			continue;	// ignore keepalives
		}
#ifdef DEBUG
		Logs::line << getFd() << "<< " << string(l, tmp) << endl;
#endif
		Command cmd(l, tmp);
		l = tmp+1;

		conn->onLine(cmd);
		if(disconnected)
			return;
	}
	readPos = r - l;
	if(readPos == BUF_SIZE)
		throw parse_error("line limit of 1024 characters exceeded");

	::memmove(readBuffer, l, readPos);
}

void ADCSocket::onTimer(int) throw()
{
	if(!disconnected) {
		// Do a silent disconnect. We don't want to show our protocol to an unknown peer.
		//(we could try sending an NMDC-style message here)
		disconnect("send timeout");
	}
	//last thing before we return control to event-system: the right thing to do! :)
	realDisconnect();
}

void ADCSocket::onRead(int) throw()
{
	try {
		handleOnRead();
	} catch(const command_error& e) {
		conn->doError(e.what(), e.code(), e.param());
		disconnect(e.what());
	} catch(const parse_error& e) {
		// stuff that's not even valid ADC -> silent disconnect
		while(!queue.empty())
			queue.pop();
		disconnect(e.what());
	} catch(const socket_error& e) {
		while(!queue.empty())
			queue.pop();
		disconnect(e.what());
	}
	// do this as the last thing before we return, see notes in realDisconnect
	if(disconnected && queue.empty()){
		realDisconnect();
	}
}

void ADCSocket::onWrite(int) throw()
{
	partialWrite();
	if(queue.empty()) {
		// possibly move this part to before partialWrite,
		// as a read on another socket will most likely
		// cause us to need to write again
		// (remove overhead from event_add/del calls)
		EventManager::instance()->disableWrite(getFd());
		writeEnabled = false;

		// Nothing left to write.. kill us
		// dont do anything after realDisconnect, see notes there
		if(disconnected)
			realDisconnect();
	}
}

void ADCSocket::disconnect(string const& msg)
{
	Socket::disconnect(msg);
	conn->onDisconnected(msg);
}

void ADCSocket::realDisconnect()
{
	//this will "delete this", meaning: we CANNOT access anything via this-> anymore, and
	//we cannot dispatch any more member calls, and we cannot even access "this" anymore

	//this should not be necessary: the assumptions above forbid it
	assert(fd != -1 && "Assumptions of 'delete this' usage was probably broken!");

	Logs::stat << format("Real Disconnect %d %p\n") % fd % this;
	if(writeEnabled) {
		EventManager::instance()->disableWrite(getFd());
	}
	close(fd);
	fd = -1;

	delete this;
}
