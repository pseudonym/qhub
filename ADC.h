// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_ADC_H_
#define _INCLUDED_ADC_H_

#include "ADCSocket.h"
#include "compat_hash_map.h"
#include <vector>
#include <string>
#include <queue>

#include <boost/shared_ptr.hpp>

#include "Buffer.h"

namespace qhub {

using namespace std;

class Hub;

class ADC : public ADCSocket {
public:
	ADC(int fd, Hub* parent);
	virtual ~ADC();

	virtual void on_read() { ADCSocket::on_read(); };
	virtual void on_write() { ADCSocket::on_write(); };

	// Other stuff
	void sendFullInf();
	string getFullInf();

	// Send-to functions
	void sendHubMessage(string const& msg);

	// Calls from ADCSocket
	virtual void onLine(StringList const& sl, string const& full);
	virtual void onLineError(string const& msg);
	virtual void onDisconnect();
private:
	Hub* hub;

	enum State {
		START,
		PROTOCOL_ERROR, //has sent message to client, at next data, disconnect
		GOT_SUP,
		LOGGED_IN
	};
	int state;

	string guid;
	bool added;
	hash_map<string, string> INF;
	typedef hash_map<string, string>::iterator INFIterator;

	// Invalid
	ADC() : ADCSocket(-1) {};
};


}

#endif //_INCLUDED_ADC_H_
