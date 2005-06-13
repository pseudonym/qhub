#ifndef QHUB_ERROR_H
#define QHUB_ERROR_H

#include <exception>
#include <string>
#include "Logs.h"

namespace qhub {

struct Exception : public std::exception
{
	Exception(const std::string& m) : msg(m) {}
	~Exception() throw() {}
	
	virtual const char* what() const throw()
	{
		return msg.c_str();
	}

private:
	std::string msg;
};

} // namespace qhub

#endif // QHUB_ERROR_H
