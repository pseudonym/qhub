// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_ADCSOCKET_H_
#define _INCLUDED_ADCSOCKET_H_

#include "Socket.h"

#include "Util.h"

namespace qhub {

class ConnectionBase;

class ADCSocket : public Socket {
public:
	/*
	 * Normal
	 */
	ADCSocket(int fd, Domain domain) throw();
	ADCSocket() throw();
	virtual ~ADCSocket() throw();

	ConnectionBase* getConnection() throw() { return conn; };
	void setConnection(ConnectionBase* c) throw() { conn = c; };

	/*
	 * EventManager calls
	 */
	virtual void onRead(int) throw();
	virtual void onWrite(int) throw();
	virtual void onTimer(int) throw();

	virtual void disconnect(std::string const& msg = Util::emptyString);

protected:
	/*
	 * Do protocol stuff / Handle events
	 */
	void realDisconnect();

private:
	void handleOnRead();

	char* readBuffer;
	size_t readPos;

	ConnectionBase* conn;
};

} //namespace qhub

#endif //_INCLUDED_ADCSOCKET_H_
