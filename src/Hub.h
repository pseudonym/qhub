// vim:ts=4:sw=4:noet
#ifndef QHUB_HUB_H
#define QHUB_HUB_H

#include "qhub.h"
#include "Command.h"
#include "Singleton.h"

#include <string>

namespace qhub {

class Hub : public Singleton<Hub> {
public:
	sid_type getSid() const { return sid; }
	// safe to add one, because file descriptors below ~3 will always be used
	sid_type getBotSid() const { return getSid() | 1; }

	void setName(const std::string& n) { name = n; }
	const std::string& getName() const { return name; }

	void setDescription(const std::string& d) { description = d; }
	const std::string& getDescription() const { return description; }

	void setInterPass(const std::string& p) { interPass = p; }
	const std::string& getInterPass() const { return interPass; }

	void motd(Client* c) throw();
	Command getAdcInf() const throw();

private:
	friend class Singleton<Hub>;

	sid_type sid;

	std::string name;
	std::string description;
	std::string interPass;

	Hub() throw();
	~Hub() throw() {}
};

} // namespace qhub

#endif // QHUB_HUB_H
