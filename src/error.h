// vim:ts=4:sw=4:noet
#ifndef QHUB_ERROR_H
#define QHUB_ERROR_H

#include <stdexcept>
#include <string>

namespace qhub {

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

} // namespace qhub

#endif // QHUB_ERROR_H
