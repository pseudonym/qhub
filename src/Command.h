#ifndef QHUB_COMMAND_H
#define QHUB_COMMAND_H

#include <string>
#include <map>
#include <vector>
#include <cassert>

#include "id.h"
#include "error.h"
#include "Util.h"

namespace qhub {

class ConnectionBase;

class Command {
public:
	static u_int32_t stringToFourCC(std::string const& c) {
		assert(c.size() == 4);
		return ((u_int32_t)c[0])|((u_int32_t)c[1]<<8)|((u_int32_t)c[2]<<16)|((u_int32_t)c[3]<<24);
	}

	static std::map<uint32_t,int> numPosParams;
	static void initNumPosParams() throw();

#define MAKE_CMD(n, a, b, c) n = (((u_int32_t)a<<8) | (((u_int32_t)b)<<16) | (((u_int32_t)c)<<24))
	enum CmdInt {
		MAKE_CMD(CTM, 'C','T','M'),
		MAKE_CMD(DSC, 'D','S','C'),
		MAKE_CMD(GET, 'G','E','T'),
		MAKE_CMD(GFI, 'G','F','I'),
		MAKE_CMD(GPA, 'G','P','A'),
		MAKE_CMD(INF, 'I','N','F'),
		MAKE_CMD(MSG, 'M','S','G'),
		MAKE_CMD(PAS, 'P','A','S'),
		MAKE_CMD(QUI, 'Q','U','I'),
		MAKE_CMD(RCM, 'R','C','M'),
		MAKE_CMD(RES, 'R','E','S'),
		MAKE_CMD(SCH, 'S','C','H'),
		MAKE_CMD(SID, 'S','I','D'),
		MAKE_CMD(SND, 'S','N','D'),
		MAKE_CMD(STA, 'S','T','A'),
		MAKE_CMD(SUP, 'S','U','P'),
		// ZLIF extension
		MAKE_CMD(ZON, 'Z','O','N'),
		MAKE_CMD(ZOF, 'Z','O','F'),
		// user command extension
		MAKE_CMD(CMD, 'C','M','D')
	};
#undef MAKE_CMD

	typedef std::pair<std::string,std::string> NamedParam;
	typedef std::vector<std::string> Params;

	typedef Params::iterator ParamIter;
	typedef Params::const_iterator ConstParamIter;

	Command(const Command&) throw();
	Command(const char* first, const char* last) throw(parse_error);
	Command(char a, CmdInt c, sid_type f = INVALID_SID,
			const std::string& feat = Util::emptyString) throw();
	Command(CmdInt c, sid_type f, sid_type t) throw();

	void swap(Command&) throw();

	Command& operator<<(const std::string&) throw();
	Command& operator<<(const NamedParam&) throw(parse_error);

	Command& operator=(const Command&) throw();
	bool operator==(uint32_t rhs) const { return uint32_t(cmd | action) == rhs; };
	bool operator!=(uint32_t rhs) const { return !(*this == rhs); };

	std::string& operator[](int pos);
	const std::string& operator[](int pos) const;

	ParamIter begin() throw() { return setDirty(), params.begin(); }
	ConstParamIter begin() const throw() { return params.begin(); }
	ParamIter end() throw() { return setDirty(), params.end(); }
	ConstParamIter end() const throw() { return params.end(); }

	ParamIter find(const std::string& k, ParamIter start) throw();
	ConstParamIter find(const std::string& k, ConstParamIter start) const throw();
	ParamIter find(const std::string& k) throw() { return find(k, begin() + getOffset()); }
	ConstParamIter find(const std::string& k) const throw() { return find(k, begin() + getOffset()); }

	const std::string& toString() const throw();

	sid_type getSource() const throw() { return from; };
	sid_type getDest() const throw() { return to; };
	CmdInt getCmd() const throw() { return cmd; };
	char getAction() const throw() { return action; };
	int getOffset() const throw() { return numPosParams[getCmd()]; }
	const std::string& getFeatures() const throw() { return features; }

private:
	void setDirty() throw() { dirty = true; };
	void checkFeatures() const throw(parse_error);

	Params params;

	char action;
	CmdInt cmd;

	sid_type from;
	sid_type to;

	std::string features;

	bool dirty;
	mutable std::string full;

	Command() throw() {}
};

typedef Command::NamedParam CmdParam;

inline void swap(Command& l, Command& r) throw() { l.swap(r); }

} // namespace qhub

#endif // QHUB_COMMAND_H
