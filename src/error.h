// vim:ts=4:sw=4:noet
#ifndef QHUB_ERROR_H
#define QHUB_ERROR_H

#include <stdexcept>
#include <string>

#include "Util.h"

namespace qhub {

// used for parsing ex. bad escapes
struct parse_error : public std::runtime_error {
	parse_error() : runtime_error("parse_error: error parsing ADC string") {}
	explicit parse_error(const std::string& arg) : runtime_error(arg) {}
};

// used for invalid command types
struct command_error : public std::runtime_error {
	command_error() : runtime_error("command_error: invalid ADC command") {}
	explicit command_error(const std::string& arg, int c = 0,
			const std::string& p = Util::emptyString)
		: runtime_error(arg), errcode(c), par(p) {}
	~command_error() throw() {}
	const std::string& param() const { return par; }
	int code() const { return errcode; }
private:
	int errcode;
	const std::string par;
};

// used for low-level socket stuff
struct socket_error : public std::runtime_error {
	socket_error() : runtime_error("socket_error: low-level socket problem") {}
	explicit socket_error(const std::string& arg) : runtime_error(arg) {}
};

// used by Logs class for errors
struct logs_error : public std::runtime_error {
	logs_error() : runtime_error("logs_error: problem with logging streams") {}
	explicit logs_error(const std::string& arg) : runtime_error(arg) {}
};

// non-socket I/O errors, mainly for config files
struct io_error : public std::runtime_error {
	io_error() : runtime_error("io_error: problem reading/writing files") {}
	explicit io_error(const std::string& arg) : runtime_error(arg) {}
};

} // namespace qhub

#endif // QHUB_ERROR_H
