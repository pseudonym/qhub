// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_ADCCLIENT_H_
#define _INCLUDED_ADCCLIENT_H_

#include "ADCSocket.h"
#include "compat_hash_map.h"
#include <vector>
#include <string>
#include <queue>

#include <boost/shared_ptr.hpp>

#include "Buffer.h"
#include "string8.h"

namespace qhub {

using namespace std;

class Hub;
class ADCInf;
class UserData;

class ADCClient : public ADCSocket {
public:
	enum State {
		START,		// HSUP
		IDENTIFY,	// BINF
		VERIFY,		// HPAS
		NORMAL,
		DISCONNECTED	// signals that one shouldn't use this anymore
	};
	
	ADCClient(int fd, Domain fd, Hub* parent) throw();
	virtual ~ADCClient() throw();

	virtual void on_read() { ADCSocket::on_read(); };
	virtual void on_write() { ADCSocket::on_write(); };

	/*
	 * ADC protocol
	 */
	string const& getInf() const;

	/*
	 * Object information
	 */
	State getState() const throw() { return state; };
	UserData* getData() throw() { return userData; };
	string const& getCID32() const { return guid; };
	ADCInf* getAttr() { return attributes; };

	/*
	 * Various calls (don't send in bad states!)
	 */
	virtual void doAskPassword(string const& pwd) throw(); // send at LOGIN only!
	virtual void doWarning(string const& msg) throw();
	virtual void doError(string const& msg) throw();
	virtual void doDisconnect(string const& msg = Util::emptyString) throw();
	virtual void doHubMessage(string const& msg) throw();
	virtual void doPrivateMessage(string const& msg) throw();

protected:
	/*
	 * Calls from ADCSocket
	 */
	virtual void onLine(StringList const& sl, string const& full) throw();
	virtual void onConnected() throw();
	virtual void onDisconnected(string const& clue) throw();
	
private:
	/*
	 * Data handlers	
	 */
	void handleA(StringList const& sl, string const& full);
	void handleB(StringList const& sl, string const& full);
	void handleBINF(StringList const& sl, string const& full);
	void handleBMSG(StringList const& sl, string const& full);
	void handleD(StringList const& sl, string const& full);
	void handleH(StringList const& sl, string const& full);
	void handleHDSC(StringList const& sl, string const& full);
	void handleHPAS(StringList const& sl, string const& full);
	void handleHSUP(StringList const& sl, string const& full);
	void handleP(StringList const& sl, string const& full);

	void login();
	void logout();
	bool added;

	ADCInf* attributes;	
	Hub* hub;
	UserData* userData;

	State state;
	string guid;
	string password;
	string8 salt;

	// Invalid
	ADCClient() : ADCSocket(-1, Socket::IP4), attributes(0) {};
};

}

#endif //_INCLUDED_ADCCLIENT_H_
