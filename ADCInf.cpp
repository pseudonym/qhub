// vim:ts=4:sw=4:noet
#include "ADCInf.h"
#include "ADCClient.h"

using namespace std;
using namespace qhub;

void ADCInf::setInf(StringList const& sl) throw()
{
	for(StringList::const_iterator sli = sl.begin() + 2; sli != sl.end(); ++sli) {
		if(sli->length() >= 2) {
			setInf(sli->substr(0, 2), sli->substr(2));
		}
	}
}

void ADCInf::setInf(string const& key, string const& val) throw()
{
	// set to changes buffer first, move to current buffer only on send (getChanged..)
	// only modify protocol forced stuff, not hubs/slots/etc.. those should be in plugins
	// perhaps add a plugin that checks if the I4/I6 values are correct
	if(key == "I4" && val == "0.0.0.0") {
		if(parent->getDomain() == PF_INET)
			changes[key] = parent->getPeerName();
		else
			changes[key] = Util::emptyString;
	} else if(key == "I6" && (val == "[::]" || val == "[0:0:0:0:0:0:0:0]")) {
		if(parent->getDomain() == PF_INET6)
			changes[key] = parent->getPeerName();
		else
			changes[key] = Util::emptyString;
	} else {
		changes[key] = val;
	}
}

void ADCInf::updateInf() throw()
{
	// update current
	for(Inf::const_iterator i = changes.begin(); i != changes.end(); ++i) {
		Inf::iterator j = current.find(i->first);
		if(j != current.end() && i->second.empty())
			current.erase(j);
		else
			current[i->first] = i->second;
	}
	changes.clear();
	// update inf string
	full = "BINF ";
	full += parent->getCID32();
	for(Inf::const_iterator i = current.begin(); i != current.end(); ++i)
		full += ' ' + ADCSocket::esc(i->first + i->second);
	full += '\n';
}
	
string ADCInf::getChangedInf() throw()
{
	// iterate over current and changes to see differences
	string partial;
	for(Inf::const_iterator i = changes.begin(); i != changes.end(); ++i) {
		Inf::const_iterator j = current.find(i->first);
		bool exists = j != current.end();
		if((!exists && !i->second.empty()) || (exists && i->second != j->second))
			partial += ' ' + ADCSocket::esc(i->first + i->second);
	}
	// erase changes buffer and update current/full
	updateInf();
	if(partial.empty())
		return Util::emptyString;
	return "BINF " + parent->getCID32() + partial + '\n';
}

string const& ADCInf::getNewInf(string const& key) const throw()
{
	Inf::const_iterator i = changes.find(key);
	if(i != changes.end())
		return i->second;
	return Util::emptyString;
}

string const& ADCInf::getSetInf(string const& key) const throw()
{
	Inf::const_iterator i = current.find(key);
	if(i != current.end())
		return i->second;
	return Util::emptyString;
}

bool ADCInf::newInf(string const& key) const throw()
{
	Inf::const_iterator i = changes.find(key);
	Inf::const_iterator j = current.find(key);
	if(
			(i == current.end()) ||
			(j == current.end() && i->second.empty()) ||
			(j != current.end() && i->second == j->second)
	)
		return false;
	return true;
}
