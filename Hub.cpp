#include "Hub.h"

#include "qhub.h"
#include "ServerSocket.h"

using namespace qhub;

Hub::Hub()
{
	//Inter-hub
	ServerSocket* tmp = new ServerSocket(9000, ServerSocket::INTER_HUB);
	enable(tmp->getFd(), OOP_READ, tmp);

	//Leaf-handler
	tmp = new ServerSocket(9001, ServerSocket::LEAF_HANDLER);
	enable(tmp->getFd(), OOP_READ, tmp);
}



