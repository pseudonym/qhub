// vim:ts=4:sw=4:noet

#include "ConnectionBase.h"

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
	for(CPI i = cmd.find("AD"); i != cmd.end(); i = cmd.find("AD", boost::next(i)))
		supp.insert(i->substr(2));
	for(CPI i = cmd.find("RM"); i != cmd.end(); i = cmd.find("RM", boost::next(i)))
		supp.erase(i->substr(2));
}
