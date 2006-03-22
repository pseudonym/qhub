// vim:ts=4:sw=4:noet

#include "ConnectionBase.h"

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
