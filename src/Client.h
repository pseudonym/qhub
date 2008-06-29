// vim:ts=4:sw=4:noet
#ifndef QHUB_CLIENT_H
#define QHUB_CLIENT_H

#include <vector>
#include <string>

#include "ConnectionBase.h"
#include "Util.h"

namespace qhub {

class Hub;
class UserData;
class UserInfo;
class ADCSocket;

class Client : public ConnectionBase {
public:
	/*
	 * Constructor / Destructor
	 */
	Client(ADCSocket* s) throw();
	virtual ~Client() throw();

	/*
	 * ADC protocol
	 */
	Command const& getAdcInf() throw();

	/*
	 * Object information
	 */
	UserData* getUserData() throw();
	UserInfo* getUserInfo() throw() { return userInfo; };
	sid_type getSid() const throw() { return sid; };

	/*
	 * Various calls (don't send in bad states!)
	 */
	// Special login call
	void doAskPassword(std::string const& pwd) throw(); // send at LOGIN only!
	// Error types
	virtual void doWarning(std::string const& msg) throw();
	virtual void doError(std::string const& msg, int code, const std::string& flag) throw();
	virtual void doDisconnect(std::string const& msg = Util::emptyString) throw();
	// Message types
	void doHubMessage(std::string const& msg) throw();
	void doPrivateMessage(std::string const& msg) throw();
	// Disconnect
	void doDisconnectBy(sid_type kicker, std::string const& msg) throw();

	/*
	 * Calls from ADCSocket
	 */
	virtual void onLine(Command& cmd) throw(command_error);
	virtual void onConnected() throw();
	virtual void onDisconnected(std::string const& clue) throw();

private:
	/*
	 * Data handlers
	 */
	void handle(Command&) throw(command_error);
	void handleSupports(Command&) throw(command_error);
	void handleLogin(Command&) throw(command_error);
	void handlePassword(Command&) throw();
	void handleDisconnect(Command&) throw();
	void handleInfo(Command&) throw(command_error);
	void handleMessage(Command&) throw();
	void handleAddr(UserInfo&) throw(command_error);

	void login() throw();
	void logout() throw();
	bool added;

	UserData* userData;
	UserInfo* userInfo;

	sid_type sid;
	std::string password;
	std::vector<uint8_t> salt;
};

}

#endif //QHUB_CLIENT_H
