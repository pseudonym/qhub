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

class ADCSocket : public Socket {
public:
	/*
	 * ADC escaping
	 */
	static string esc(string const& in);
	static string cse(string const& in);

	/*
	 * Normal
	 */
	ADCSocket(int fd);
	virtual ~ADCSocket();
	void send(string const& msg) { write(msg, 0); };

	/*
	 * fd_demux calls
	 */
	virtual void on_read();
	virtual void on_write();

	/*
	 * Do protocol stuff / Handle events
	 */
	virtual void doError(string const& msg) throw() = 0; // send FatalError message
	virtual void onLine(StringList const& sl, string const& full) throw() = 0;
	virtual void onConnected() throw() = 0;
	virtual void onDisconnected(string const& clue) throw() = 0;

protected:
	void disconnect() { Socket::disconnect(); onDisconnected(Util::emptyString); };
	void disconnect(string const& msg) { Socket::disconnect(); onDisconnected(msg); };

private:
	StringList data;
	string raw;

	void realDisconnect();
	
	int readBufferSize;
	unsigned char* readBuffer;
	enum ReadState { NORMAL, PARTIAL } state;
	bool escaped;

	// Invalid
	ADCSocket() {};
};

} //namespace qhub

#endif //_INCLUDED_ADCSOCKET_H_
