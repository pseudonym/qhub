// vim:ts=4:sw=4:noet
#include "ADC.h"

using namespace std;
using namespace qhub;

bool ADC::Attributes::setInf(StringList const& sl)
{
	bool ret = true;
	for(StringList::const_iterator sli = sl.begin() + 2; sli != sl.end(); ++sli) {
		if(sli->length() < 2) {
			parent->sendHubMessage("BINF takes only named parameters");
			return false;
		} else {
			ret = ret && setInf(sli->substr(0, 2), sli->substr(2));
		}
	}
	return ret;
}

bool ADC::Attributes::setInf(string const& key, string const& val)
{
	// set to changes buffer first
	// move to current buffer only on send (getInfs)
	if(key == "I4" && val == "0.0.0.0") {
		changes[key] = parent->getPeerName();
	} else {
		changes[key] = val;
	}
	// here we block illegal key/value combinations, like OP1
	return true;
}

void ADC::Attributes::updateInf()
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
		full += ' ' + esc(i->first + i->second);
	full += '\n';
}
	
string ADC::Attributes::getChangedInf()
{
	// iterate over current and changes to see differences
	string partial = "BINF ";
	partial += parent->getCID32();
	for(Inf::const_iterator i = changes.begin(); i != changes.end(); ++i) {
		Inf::const_iterator j = current.find(i->first);
		bool exists = j != current.end();
		if((!exists && !i->second.empty()) || (exists && i->second != j->second))
			partial += ' ' + esc(i->first + i->second);
	}
	// erase changes buffer and update current/full
	updateInf();
	return partial + '\n';
}
