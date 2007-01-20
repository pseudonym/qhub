#include "Command.h"
#include "Util.h"
#include "ADC.h"
#include "Logs.h"

using namespace std;
using namespace qhub;

map<uint32_t,int> Command::numPosParams;

void Command::initNumPosParams() throw()
{
	if(!numPosParams.empty())
		return;	// already initialized

	numPosParams[CTM] = 2;
	numPosParams[DSC] = 1;
	numPosParams[GET] = 4;
	numPosParams[GFI] = 2;
	numPosParams[GPA] = 1;
	numPosParams[INF] = 0;
	numPosParams[MSG] = 1;
	numPosParams[PAS] = 1;
	numPosParams[QUI] = 1;
	numPosParams[RCM] = 1;
	numPosParams[RES] = 0;
	numPosParams[SCH] = 0;
	numPosParams[SID] = 1;
	numPosParams[SND] = 4;
	numPosParams[STA] = 2;
	numPosParams[SUP] = 0;
	// ZLIF extension
	numPosParams[ZON] = 0;
	numPosParams[ZOF] = 0;
	// user command extension
	numPosParams[CMD] = 1;
}

Command::Command(const Command& rhs) throw()
		: params(rhs.params), action(rhs.action), cmd(rhs.cmd), from(rhs.from), to(rhs.to),
		  features(rhs.features), dirty(true)
{
}

Command::Command(const char* first, const char* last) throw(parse_error)
		: from(INVALID_SID), to(INVALID_SID), dirty(false), full(first, last)
{
	initNumPosParams();
	// optimize later
	// this is lazy due to a DC++ bug that sends empty parameters...
	// ideally, we should not be using it
	StringList sl = Util::lazyStringTokenize(full);
	full += '\n';
	if(full.size() < 5 || sl[0].size() != 4)
		throw parse_error("invalid message type");
	action = sl[0][0];
	cmd = (CmdInt)(stringToFourCC(sl[0]) & 0xFFFFFF00);
	StringList::size_type loc;	// message parameters start at this index

	switch(action) {
	case 'F':
		from = ADC::toSid(sl[1]);
		features = sl[2];
		checkFeatures();
		loc = 3;
		break;
	case 'D':
	case 'E':
		from = ADC::toSid(sl[1]);
		to = ADC::toSid(sl[2]);
		loc = 3;
		break;
	case 'B':
	case 'S':
		from = ADC::toSid(sl[1]);
		loc = 2;
		break;
	case 'I':
	case 'H':
	case 'L':
		loc = 1;
		break;
	default:
		throw parse_error(string("invalid action type '") + action + '\'');
	}

	if(numPosParams.count(cmd)) {
		// known command
		if(loc + numPosParams[cmd] > sl.size())
			throw parse_error("missing parameters");
		params.resize(numPosParams[cmd]);
		copy(sl.begin() + loc, sl.begin() + loc + numPosParams[cmd], params.begin());
		for(StringList::iterator i = sl.begin() + loc + numPosParams[cmd]; i != sl.end(); ++i) {
			// param name parts can't be escaped chars
			if(i->size() < 2 || ADC::CSE(i->substr(0, 2)).size() != 2)
				throw parse_error("invalid named parameter: " + *i);
			else
				*this << *i;
		}
		transform(params.begin(), params.end(), params.begin(), &ADC::CSE);
	} else {
		// unknown command
		// assume all are positional
		params.resize(sl.size() - loc);
		transform(sl.begin() + loc, sl.end(), params.begin(), &ADC::CSE);
	}
}
//}}}

//{{{
Command::Command(char a, CmdInt c, sid_type f /*= INVALID_SID*/, const string& feat /*= Util::emptyString*/) throw()
		: action(a), cmd(c), from(f), to(INVALID_SID), features(feat), dirty(true)
{
	// sanity checks
	switch(action) {
	case 'F':
		checkFeatures();
		break;
	case 'B':
	case 'S':
		assert(from != INVALID_SID);
		break;
	case 'I':
	case 'L':
		assert(from == INVALID_SID);
		break;
	default:
		assert(0);
	}
}

// only makes D-type... but will we ever need to make an
// E-type one?
Command::Command(CmdInt c, sid_type f, sid_type t) throw()
		: action('D'), cmd(c), from(f), to(t), dirty(true)
{
}

void Command::swap(Command& rhs) throw()
{
	using std::swap;

	params.swap(rhs.params);
	swap(cmd, rhs.cmd);
	swap(action, rhs.action);
	swap(to, rhs.to);
	swap(from, rhs.from);
	features.swap(rhs.features);
	swap(dirty, rhs.dirty);
	swap(full, rhs.full);
}

Command& Command::operator<<(const string& val) throw()
{
	params.push_back(val);
	setDirty();
	return *this;
}

Command& Command::operator<<(const NamedParam& param) throw(parse_error)
{
	if(param.first.size() != 2)
		throw parse_error("named param key not of length 2");
	params.push_back(param.first);
	params.back() += param.second;
	setDirty();
	return *this;
}

Command& Command::operator=(const Command& rhs) throw()
{
	Command(rhs).swap(*this);
	return *this;
}

string& Command::operator[](int pos)
{
	setDirty();
	return params.at(pos);
}

const string& Command::operator[](int pos) const
{
	return params.at(pos);
}

Command::ParamIter Command::find(const string& k, ParamIter start) throw()
{
	assert(k.size() == 2);
	setDirty();
	for( ; start != end(); ++start)
		if(start->compare(0, 2, k) == 0)
			break;
	return start;
}

Command::ConstParamIter Command::find(const string& k, ConstParamIter start) const throw()
{
	assert(k.size() == 2);
	for( ; start != end(); ++start)
		if(start->compare(0, 2, k) == 0)
			break;
	return start;
}

const string& Command::toString() const throw()
{
	if(dirty) {
		full.clear();
		full += action;
		full += char((cmd >>  8) & 0xFF);
		full += char((cmd >> 16) & 0xFF);
		full += char((cmd >> 24) & 0xFF);
		full += ' ';

		switch(action) {
		case 'S':
		case 'B':
			full += ADC::fromSid(from);
			full += ' ';
			break;
		case 'D':
		case 'E':
			full += ADC::fromSid(from);
			full += ' ';
			full += ADC::fromSid(to);
			full += ' ';
			break;
		case 'F':
			full += ADC::fromSid(from);
			full += ' ';
			full += features;
			full += ' ';
			break;
		case 'I':
		case 'L':
		case 'H':
			break;
		default:
			// we should not be seeing any of the other types
			assert(0);
		}

		for(Params::const_iterator i = params.begin(); i != params.end(); ++i) {
			full += ADC::ESC(*i);
			full += ' ';
		}
		full[full.size()-1] = '\n';
	}
	return full;
}

void Command::checkFeatures() const throw(parse_error)
{
	if(features.size() % 5)
		throw parse_error("feature string invalid");
	for(string::size_type i = 0; i != features.size(); i += 5) {
		if(features[i] == '+' || features[i] == '-')
			continue;
		else
			throw parse_error("bad feature operator");
	}
}
