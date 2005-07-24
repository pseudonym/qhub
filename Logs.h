#ifndef LOGS_H_INCLUDED
#define LOGS_H_INCLUDED

#include <ostream>
#include <string>
#include <boost/format.hpp>

namespace qhub {

class Logs {
public:
	// these are just aliases for standard streams to save on memory
	static std::ostream& err;
	static std::ostream& stat;
	static std::ostream& line;

	static void setErr(const std::string& filename);
	static void setStat(const std::string& filename);
	static void setLine(const std::string& filename);
	static void set(std::ostream& s, const std::string& filename);
	static void copy(const std::ostream& src, std::ostream& dest);
};

using boost::format;

}

#endif // LOGS_H_INCLUDED
