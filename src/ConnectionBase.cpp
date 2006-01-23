// vim:ts=4:sw=4:noet

#include "ConnectionBase.h"

using namespace qhub;
using namespace std;

ConnectionBase::ConnectionBase(Hub* h, ADCSocket* s) throw()
		: state(PROTOCOL), hub(h), sock(s)
{
	if(!sock)
		sock = new ADCSocket;
	sock->setConnection(this);
}

ConnectionBase::~ConnectionBase() throw()
{
	sock->setConnection(NULL);
}
