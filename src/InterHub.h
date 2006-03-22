#ifndef __INTERHUB_H_
#define __INTERHUB_H_

#include "ConnectionBase.h"

#include <string>
#include <vector>

namespace qhub {

class Hub;

class InterHub : public ConnectionBase {
public:
	InterHub(const std::string& hn, short p, const std::string& pa) throw();
	InterHub(ADCSocket* s) throw();
	virtual ~InterHub() throw() {};

	//for DNSLookup callback
	virtual void onLookup(const std::string& ip);

	short getPort() const { return port; }
	//const string& getCID32() const { return cid; }

	// from ConnectionBase
	virtual void doError(const std::string& msg) throw();
	virtual void doWarning(const std::string& msg) throw();

protected:
	/*
	 * Calls from ConnectionBase
	 */
	virtual void onLine(Command&) throw(command_error);
	virtual void onConnected() throw();
	virtual void onDisconnected(std::string const& clue) throw();

private:
	void doSupports() throw();
	void doInf() throw();
	void doAskPassword() throw();
	void doPassword(const Command& cmd) throw();

	void handle(const Command& cmd) throw(command_error);
	void handlePassword(const Command& cmd) throw(command_error);

	std::string hostname;
	short port;
	std::string password;
	const bool outgoing;

	std::vector<uint8_t> salt;
};

} // namespace qhub


#endif // __INTERHUB_H
