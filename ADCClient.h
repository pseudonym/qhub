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
	 * Constructor / Destructor
	 */
	ADCClient(int fd, Domain domain, Hub* parent) throw();
	virtual ~ADCClient() throw();

	/*
	 * ADC protocol
	 */
	string const& getAdcInf() throw();

	/*
	 * Object information
	 */
	UserData* getUserData() throw();
	UserInfo* getUserInfo() throw() { return userInfo; };
	string const& getCID32() const throw() { return cid; };

	/*
	 * Various calls (don't send in bad states!)
	 */
	// Special login call
	void doAskPassword(string const& pwd) throw(); // send at LOGIN only!
	// Error types
	virtual void doWarning(string const& msg) throw();
	virtual void doError(string const& msg) throw();
	virtual void doDisconnect(string const& msg = Util::emptyString) throw();
	// Message types
	void doHubMessage(string const& msg) throw();
	void doPrivateMessage(string const& msg) throw();
	// Disconnect
	void doDisconnectBy(string const& kicker, string const& msg) throw();

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

	string cid;
	string password;
	string8 salt;
	bool active;

	string const& assemble(StringList const& sl) throw();
	string temp;

	// Invalid
	ADCClient() : ADCSocket(-1, Socket::IP4, NULL) {};
};

}

#endif //_INCLUDED_ADCCLIENT_H_
