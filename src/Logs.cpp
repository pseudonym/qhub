#include "Logs.h"
#include "error.h"
#include <iostream>
#include <fstream>

using namespace std;
using namespace qhub;

ostream Logs::err(cerr.rdbuf());
ostream Logs::stat(cout.rdbuf());
#ifdef DEBUG
ostream Logs::line(0);
#endif

void Logs::setErr(const string& fn)
{
	auto_ptr<filebuf> tmp(new filebuf);
	tmp->open(fn.c_str(), ios::app | ios::out);
	if(!tmp->is_open()) {
		throw logs_error("could not reassign error stream to \"" + fn + '"');
	}
	err.rdbuf(tmp.release());
}

void Logs::setStat(const string& fn)
{
	auto_ptr<filebuf> tmp(new filebuf);
	tmp->open(fn.c_str(), ios::app | ios::out);
	if(!tmp->is_open()) {
		throw logs_error("could not reassign status stream to \"" + fn + '"');
	}
	stat.rdbuf(tmp.release());
}

#ifdef DEBUG
void Logs::setLine(const string& fn)
{
	auto_ptr<filebuf> tmp(new filebuf);
	tmp->open(fn.c_str(), ios::app | ios::out);
	if(!tmp->is_open()) {
		throw logs_error("could not reassign protocol line stream to \"" + fn + '"');
	}
	line.rdbuf(tmp.release());
}
#endif

void Logs::set(std::ostream& s, const std::string& filename)
{
	auto_ptr<filebuf> tmp(new filebuf);
	tmp->open(filename.c_str(), ios::app | ios::out);
	if(!tmp->is_open()) {
		throw logs_error("Logs::set(): could not reassign stream to \""
				+ filename + '"');
	}
	s.rdbuf(tmp.release());
}

void Logs::copy(const ostream& src, ostream& dest)
{
	dest.rdbuf(src.rdbuf());
}
