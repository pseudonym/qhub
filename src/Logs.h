#ifndef QHUB_LOGS_H
#define QHUB_LOGS_H

#include "qhub.h"

#include <ostream>
#include <string>

#include <boost/format.hpp>

namespace qhub {

class Logs {
public:
	static std::ostream err;
	static std::ostream stat;
#ifdef DEBUG
	static std::ostream line;
#endif

	static void setErr(const std::string& filename);
	static void setStat(const std::string& filename);
#ifdef DEBUG
	static void setLine(const std::string& filename);
#endif
	static void set(std::ostream& s, const std::string& filename);
	static void copy(const std::ostream& src, std::ostream& dest);
};

using boost::format;

}

#endif // QHUB_LOGS_H
