// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_ADCSOCKET_H_
#define _INCLUDED_ADCSOCKET_H_

#include "Socket.h"
#include "compat_hash_map.h"
#include <vector>
#include <string>
#include <queue>

#include <boost/shared_ptr.hpp>

#include "Buffer.h"
#include "Util.h"

using namespace std;

namespace qhub {

class Hub;

class ADCSocket : public Socket {
public:
	/*
	 * Normal
	 */
	ADCSocket(int fd, Domain domain, Hub* parent) throw();
	virtual ~ADCSocket() throw();
	void send(string const& msg) { write(msg, 0); };
	Hub* getHub() throw() { return hub; };

	/*
	 * fd_demux calls
	 */
	void ADCSocket::handleOnRead() throw();
	virtual void onRead() throw();
	virtual void onWrite() throw();

	/*
	 * Do protocol stuff / Handle events
	 */
	virtual void doError(string const& msg) throw() = 0; // send FatalError message
	
protected:
	/*
	 * Do protocol stuff / Handle events
	 */
	virtual void onLine(StringList& sl, string const& full) throw() = 0;
	virtual void onConnected() throw() = 0;
	virtual void onDisconnected(string const& clue) throw() = 0;

	void disconnect(string const& msg = Util::emptyString);
	void realDisconnect();

private:
	StringList data;
	string raw;

	int readBufferSize;
	unsigned char* readBuffer;
	enum ReadState { NORMAL, PARTIAL } state;
	bool escaped;

	Hub* hub;

	// Invalid
	ADCSocket() {};
};

} //namespace qhub

#endif //_INCLUDED_ADCSOCKET_H_
