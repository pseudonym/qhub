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
#include "Timer.h"

namespace qhub {

using namespace std;

class Hub;
class ADCInf;
class UserData;

class ADCClient : public ADCSocket {
public:
	/*
	 * Pseudo-FOURCC stuff
	 */
	static u_int32_t stringToFourCC(string const& c) {
		return ((u_int32_t)c[0])|((u_int32_t)c[1]<<8)|((u_int32_t)c[2]<<16)|((u_int32_t)c[3]<<24);
	}
	enum {
		CTM, DSC, GET, GFI, GPA, INF, MSG, NTD, PAS, QUI, RCM, RES, SCH, SND, STA, SUP, CMD_LAST
	};
	static u_int32_t Commands[CMD_LAST];

	/*
	 * Client states
	 */
	enum State {
		PROTOCOL,	// HSUP
		IDENTIFY,	// BINF
		VERIFY,		// HPAS
		NORMAL		// everything except HPAS
	};

	/*
	 * Constructor / Destructor
	 */
	ADCClient(int fd, Domain fd, Hub* parent) throw();
	virtual ~ADCClient() throw();

	virtual void onRead() throw() { ADCSocket::onRead(); };
	virtual void onWrite() throw() { ADCSocket::onWrite(); };
	virtual void onAlarm() throw();

	/*
	 * ADC protocol
	 */
	string const& getInf() const throw();

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
	void handle(StringList& sl, u_int32_t const cmd, string const* full) throw();
	void handleSupports(StringList& sl) throw();
	void handleLogin(StringList& sl) throw();
	void handlePassword(StringList& sl) throw();
	void handleDisconnect(StringList& sl) throw();
	void handleInfo(StringList& sl) throw();
	void handleMessage(StringList& sl, u_int32_t const cmd, string const* full) throw();

	void login() throw();
	void logout() throw();
	bool added;

	bool isUdpActive() const throw();

	ADCInf* attributes;	
	UserData* userData;

	State state;
	string guid;
	string password;
	string8 salt;

	string const& assemble(StringList const& sl) throw() {
		StringList::const_iterator i = sl.begin();
		full = esc(*i);
		for(++i; i != sl.end(); ++i) {
			full += ' ' + esc(*i);
		}
		full += '\n';
		return full;
	}	
	string full;

	// Invalid
	ADCClient() : ADCSocket(-1, Socket::IP4, NULL), attributes(0) {};
};

}

#endif //_INCLUDED_ADCCLIENT_H_
