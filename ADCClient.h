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
#include "Util.h"

namespace qhub {

using namespace std;

class Hub;
class UserData;
class UserInfo;

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
	string const& getAdcInf() throw();

	/*
	 * Object information
	 */
	State getState() const throw() { return state; };
	UserData* getUserData() throw();
	UserInfo* getUserInfo() throw() { return userInfo; };
	string const& getCID32() const throw() { return guid; };

	/*
	 * Various calls (don't send in bad states!)
	 */
	// Special login call
	virtual void doAskPassword(string const& pwd) throw(); // send at LOGIN only!
	// Error types
	virtual void doWarning(string const& msg) throw();
	virtual void doError(string const& msg) throw();
	virtual void doDisconnect(string const& msg = Util::emptyString) throw();
	// Message types
	virtual void doHubMessage(string const& msg) throw();
	virtual void doPrivateMessage(string const& msg) throw();
	// The four 'ban' types
	virtual void doDisconnect(string const& kicker, string const& msg, bool silent) throw();
	virtual void doKick(string const& kicker, string const& msg, bool silent) throw();
	virtual void doBan(string const& kicker, u_int32_t seconds, string const& msg, bool silent) throw();
	virtual void doRedirect(string const& kicker, string const& address, string const& msg, bool silent) throw();

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
	void handleInfo(StringList& sl, u_int32_t const cmd, string const* full) throw();
	void handleMessage(StringList& sl, u_int32_t const cmd, string const* full) throw();

	void login() throw();
	void logout() throw();
	bool added;

	UserData* userData;
	UserInfo* userInfo;

	State state;
	string guid;
	string password;
	string8 salt;
	bool udpActive;

	string const& assemble(StringList const& sl) throw();
	string temp;

	// Invalid
	ADCClient() : ADCSocket(-1, Socket::IP4, NULL) {};
};

}

#endif //_INCLUDED_ADCCLIENT_H_
