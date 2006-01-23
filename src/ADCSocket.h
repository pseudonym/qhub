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
#include "Timer.h"
#include "Util.h"

using namespace std;

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
	 * fd_demux calls
	 */
	virtual bool onRead() throw();
	virtual void onWrite() throw();
	virtual void onTimeout() throw();

	virtual void disconnect(string const& msg = Util::emptyString);

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
