#include "InterHub.h"

#include "qhub.h"

using namespace qhub;

InterHub::InterHub(int fd) 
{
	this->fd = fd;
	enable(fd, OOP_READ, this);
}

void InterHub::on_read()
{

}

void InterHub::on_write()
{
}
