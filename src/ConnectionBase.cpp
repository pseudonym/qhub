// vim:ts=4:sw=4:noet

#include "ConnectionBase.h"
#include "ClientManager.h"
#include "ServerManager.h"
#include "Command.h"

#include <boost/utility.hpp>

using namespace qhub;
using namespace std;

ConnectionBase::ConnectionBase(ADCSocket* s) throw()
		: state(PROTOCOL), sock(s)
{
	if(!sock)
		sock = new ADCSocket;
	sock->setConnection(this);
}

ConnectionBase::~ConnectionBase() throw()
{
	sock->setConnection(NULL);
}

void ConnectionBase::updateSupports(const Command& cmd) throw()
{
	typedef Command::ConstParamIter CPI;
	using boost::next;
	for(CPI i = cmd.find("AD"); i != cmd.end(); i = cmd.find("AD", next(i)))
		supp.insert(i->substr(2));
	for(CPI i = cmd.find("RM"); i != cmd.end(); i = cmd.find("RM", next(i)))
		supp.erase(i->substr(2));
}

void ConnectionBase::dispatch(const Command& cmd) throw()
{
	switch(cmd.getAction()) {
	case 'I':
	case 'B':
		ClientManager::instance()->broadcast(cmd);
	case 'S':
		ServerManager::instance()->broadcast(cmd, this);
		break;
	case 'E':
		if(!hasSupport("IHUB")) // ghetto reflection :)
			send(cmd);
	case 'D':
		ClientManager::instance()->direct(cmd);
		break;
	case 'F':
		ServerManager::instance()->broadcast(cmd, this);
		ClientManager::instance()->broadcastFeature(cmd);
		break;
	}
}

