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
		NORMAL		// everything except HPAS
	};

	enum Action {
		PROCEED = 0,	// proceed as normal
		NONE = 1,		// there is no action to be taken, modifying state or lastParsed will not help
		STOP = 2,		// don't do the usual stuff
		MODIFIED = 4	// data has been modified, use sl, not full
	};
	void initAction(Action a = PROCEED) throw() { action = PROCEED | a; };
	void setAction(Action c) throw() { assert(!getAction(NONE)); action |= c; };
	bool getAction(Action c) const throw() { return action & c == c; };
	
	ADCClient(int fd, Domain fd, Hub* parent) throw();
	virtual ~ADCClient() throw();

	virtual void on_read() { ADCSocket::on_read(); };
	virtual void on_write() { ADCSocket::on_write(); };

	/*
	 * ADC protocol
	 */
	string const& getInf() const throw();
	string const& getFullLine() const throw() { return *lastLine; };
	StringList& getFullInput() throw() { return *lastParsed; };
	bool isActive() const throw();

	/*
	 * Object information
	 */
	State getState() const throw() { return state; };
	UserData* getData() throw() { return userData; };
	string const& getCID32() const throw() { return guid; };
	ADCInf* getAttr() throw() { return attributes; };

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
	virtual void onLine(StringList& sl, string const& full) throw();
	virtual void onConnected() throw();
	virtual void onDisconnected(string const& clue) throw();
	
private:
	/*
	 * Data handlers	
	 */
	void handleA(StringList& sl, string const& full) throw();
	void handleB(StringList& sl, string const& full) throw();
	void handleBINF(StringList& sl, string const& full) throw();
	void handleBMSG(StringList& sl, string const& full) throw();
	void handleD(StringList& sl, string const& full) throw();
	void handleH(StringList& sl, string const& full) throw();
	void handleHDSC(StringList& sl, string const& full) throw();
	void handleHPAS(StringList& sl, string const& full) throw();
	void handleHSUP(StringList& sl, string const& full) throw();
	void handleP(StringList& sl, string const& full) throw();

	void login() throw();
	void logout() throw();
	bool added;

	ADCInf* attributes;	
	Hub* hub;
	UserData* userData;

	State state;
	string guid;
	string password;
	string8 salt;
	int action;

	string const* lastLine;
	StringList* lastParsed;

	// Invalid
	ADCClient() : ADCSocket(-1, Socket::IP4), attributes(0) {};
};

}

#endif //_INCLUDED_ADCCLIENT_H_
