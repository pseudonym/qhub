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
	static string esc(string const& in);
	static string cse(string const& in);
	
	typedef vector<string> StringList;
	
	ADCSocket(int fd);
	virtual ~ADCSocket();

	virtual void on_read();
	virtual void on_write();

	void send(string const& msg) { write(msg, 0); };
	void disconnect(string const& msg) { Socket::disconnect(); onDisconnected(msg); };

	/*
	 * Send a readable message
	 */
	virtual void notify(string const& msg) = 0;
	/*
	 * Events to be handled
	 */
	virtual void onLine(StringList const& sl, string const& full) = 0;
	virtual void onConnected() = 0;
	virtual void onDisconnected(string const& clue) = 0;

protected:
	void disconnect() { Socket::disconnect(); onDisconnected(Util::emptyString); };

private:
	StringList data;
	string raw;
	bool escaped;

	void realDisconnect();
	
	int readBufferSize;
	unsigned char* readBuffer;
	enum ReadState { NORMAL, PARTIAL } state;

	// Invalid
	ADCSocket() {};
};

} //namespace qhub

#endif //_INCLUDED_ADCSOCKET_H_
