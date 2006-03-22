// vim:ts=4:sw=4:noet
#ifndef CONNECTIONBASE_H
#define CONNECTIONBASE_H

#include <boost/utility.hpp>

#include "error.h"

#include "Util.h"
#include "ADCSocket.h"
#include "Command.h"

namespace qhub {

class Hub;

class ConnectionBase : public boost::noncopyable {
public:
	/*
	 * Client states
	 */
	enum State {
		PROTOCOL,	// HSUP
		IDENTIFY,	// BINF
		VERIFY,		// HPAS
		INFLIST,	// for Interhub connections, leaves on first non-INF
		NORMAL		// everything else
	};


	/*
	 * Normal
	 */
	explicit ConnectionBase(ADCSocket* s = NULL) throw();
	virtual ~ConnectionBase() throw();
	void send(const Command& cmd) { sock->write(cmd.toString(), 0); };

	State getState() const throw() { return state; };
	ADCSocket* getSocket() throw() { return sock; };
	void setSocket(ADCSocket* s) throw() { sock = s; };

	/*
	 * Do protocol stuff / Handle events
	 */
	virtual void doError(string const& msg) throw() = 0; // send FatalError message
	virtual void doWarning(string const& msg) throw() = 0;

	virtual void onLine(Command& cmd) throw(command_error) = 0;
	virtual void onConnected() throw() = 0;
	virtual void onDisconnected(string const& clue) throw() = 0;

	
protected:
	// much easier than changing all the assignments to setState() :)
	State state;

private:
	ADCSocket* sock;
};

} //namespace qhub

#endif // CONNECTIONBASE_H
