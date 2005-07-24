#include "Logs.h"
#include "error.h"
#include <iostream>
#include <fstream>

using namespace std;
using namespace qhub;

ostream& Logs::err = cerr;
ostream& Logs::stat = cout;
ostream& Logs::line = (clog.rdbuf(0), clog);

void Logs::setErr(const string& fn)
{
	auto_ptr<filebuf> tmp(new filebuf);
	tmp->open(fn.c_str(), ios::app | ios::out);
	if(!tmp->is_open()) {
		throw Exception("could not reassign error stream to \"" + fn + '"');
	}
	// can't delete original iostream buffers..
	err.rdbuf(tmp.release());
}

void Logs::setStat(const string& fn)
{
	auto_ptr<filebuf> tmp(new filebuf);
	tmp->open(fn.c_str(), ios::app | ios::out);
	if(!tmp->is_open()) {
		throw Exception("could not reassign status stream to \"" + fn + '"');
	}
	stat.rdbuf(tmp.release());
}

void Logs::setLine(const string& fn)
{
	auto_ptr<filebuf> tmp(new filebuf);
	tmp->open(fn.c_str(), ios::app | ios::out);
	if(!tmp->is_open()) {
		throw Exception("could not reassign protocol line stream to \"" + fn + '"');
	}
	line.rdbuf(tmp.release());
}

void Logs::set(std::ostream& s, const std::string& filename)
{
	auto_ptr<filebuf> tmp(new filebuf);
	tmp->open(filename.c_str(), ios::app | ios::out);
	if(!tmp->is_open()) {
		throw Exception("Logs::set(): could not reassign stream to \""
				+ filename + '"');
	}
	std::streambuf* t = s.rdbuf(tmp.release());
	if(dynamic_cast<std::filebuf*>(t))
		// ok, we allocated it, delete it
		delete t;
}

void Logs::copy(const ostream& src, ostream& dest)
{
	std::streambuf* t = dest.rdbuf(src.rdbuf());
	if(dynamic_cast<std::filebuf*>(t))
		// ok, we allocated it, delete it
		delete t;
}
