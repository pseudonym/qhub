#include "Hub.h"

#include "qhub.h"
#include "ServerSocket.h"
#include "ADC.h"
#include "InterHub.h"

using namespace qhub;

Hub::Hub()
{
	//Inter-hub
	ServerSocket* tmp = new ServerSocket(9000, ServerSocket::INTER_HUB, this);
	enable(tmp->getFd(), OOP_READ, tmp);

	//Leaf-handler
	tmp = new ServerSocket(9001, ServerSocket::LEAF_HANDLER, this);
	enable(tmp->getFd(), OOP_READ, tmp);
}

void Hub::acceptLeaf(int fd)
{
	Socket* tmp = new ADC(fd, this);
}

void Hub::addClient(ADC* client, string nick)
{
	users[nick] = client;
}

void Hub::acceptInterHub(int fd)
{
	Socket* tmp = new InterHub(fd);
}


