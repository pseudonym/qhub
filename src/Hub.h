// vim:ts=4:sw=4:noet
#ifndef QHUB_HUB_H
#define QHUB_HUB_H

#include <string>

#include "Client.h"
#include "Singleton.h"

namespace qhub {

class Client;

class Hub : public Singleton<Hub> {
public:
	//these should never change during the life of the hub
	void setSidPrefix(const std::string& s) { assert(s.size() == 2); sidpre = s; }
	const std::string& getSidPrefix() const { return sidpre; }

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

	std::string name;
	std::string sidpre;
	std::string description;
	std::string interPass;
	
	Hub() throw();
	~Hub() throw() {}
};

} // namespace qhub

#endif // QHUB_HUB_H
