#ifndef QHUB_INTERHUB_H
#define QHUB_INTERHUB_H

#include "qhub.h"
#include "ConnectionBase.h"
#include "DnsManager.h"
#include "EventManager.h"

#include <string>
#include <vector>

namespace qhub {

class InterHub : public ConnectionBase, public EventListener, public DnsListener {
public:
	InterHub(const std::string& hn, short p, const std::string& pa) throw();
	InterHub(ADCSocket* s) throw();
	virtual ~InterHub() throw() {};

	// from EventListener
	virtual void onTimer(int what) throw();

	// from DnsListener
	virtual void onResult(const std::string&, const std::vector<in_addr>&) throw();
	virtual void onResult(const std::string&, const std::vector<in6_addr>&) throw() {}
	virtual void onFailure() throw();

	short getPort() const { return port; }

	// from ConnectionBase
	virtual void doError(std::string const& msg, int code, std::string const& flag) throw();
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

#endif // QHUB_INTERHUB_H
