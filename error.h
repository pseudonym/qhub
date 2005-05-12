#ifndef QHUB_ERROR_H
#define QHUB_ERROR_H

#include <exception>
#include <string>
#include <boost/format.hpp>

namespace qhub {

struct Exception : public std::exception
{
	Exception(const std::string& m) : msg(m) {}
	~Exception() throw() {}
	
	virtual const char* what()
	{
		return msg.c_str();
	}

private:
	std::string msg;
};

extern std::FILE* qerr;
extern std::FILE* qstat;
extern std::FILE* qline;

template<typename CharT, typename Traits, typename Alloc>
inline void log(std::FILE* stream, const std::basic_string<CharT,Traits,Alloc>& msg)
{
	fwrite(msg.data(), msg.size(), sizeof(CharT), stream);
	fwrite("\n", 1, sizeof(char), stream);
}

template<typename CharT, typename Traits, typename Alloc>
inline void log(std::FILE* stream, const boost::basic_format<CharT,Traits,Alloc>& msg)
{
	log(stream, msg.str());
}

template<typename CharT>
inline void log(std::FILE* stream, const CharT* msg)
{
	log(stream, std::basic_string<CharT>(msg));
}

using boost::format;

} // namespace qhub

#endif // QHUB_ERROR_H
