// vim:ts=4:sw=4:noet
#ifndef QHUB_ADC_H
#define QHUB_ADC_H

#include "qhub.h"
#include "error.h"
#include "Util.h"

#include <string>

namespace qhub {

class ADC {
public:
	static std::string ESC(std::string const& in) throw();
	static std::string CSE(std::string const& in) throw(parse_error);
	static sid_type toSid(const std::string&) throw(parse_error);
	static std::string fromSid(sid_type) throw();
	static std::string& toString(StringList const& sl, std::string& out) throw();
};

} //namespace qhub

#endif //QHUB_ADC_H
