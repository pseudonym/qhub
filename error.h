// vim:ts=4:sw=4:noet
#ifndef QHUB_ERROR_H
#define QHUB_ERROR_H

#include <stdexcept>
#include <string>

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

// used for parsing ex. bad escapes
struct parse_error : public std::runtime_error {
	parse_error() : runtime_error("parse_error: error parsing ADC string") {}
	explicit parse_error(const std::string& arg) : runtime_error(arg) {}
};

// used for invalid command types
struct command_error : public std::runtime_error {
	command_error() : runtime_error("command_error: invalid ADC command") {}
	explicit command_error(const std::string& arg) : runtime_error(arg) {}
};

} // namespace qhub

#endif // QHUB_ERROR_H
