// vim:ts=4:sw=4:noet
#ifndef QHUB_PLUGIN_VIRTUALFSDIR_H
#define QHUB_PLUGIN_VIRTUALFSDIR_H

#include "Util.h"

#include <cassert>
#include <map>

namespace qhub {

class Dir {
public:
	Dir(std::string const& n = Util::emptyString, Dir* parent = NULL) throw() : name(n), partial(false) {
		if(parent != NULL) {
			subDirs[".."] = parent;
			partial = parent->partial;
		}
	}
	~Dir() throw() {
		for(SubDirs::iterator i = subDirs.begin(); i != subDirs.end(); ++i) {
			if(i->first != "..")
				delete i->second;
		}
	}
	std::string toString(int indent = 0) const throw() {
		std::string ret;
		for(SubDirs::const_iterator i = subDirs.begin(); i != subDirs.end(); ++i) {
			ret += std::string(indent, ' ') + i->first + "/\n";
			if(i->first != "..")
				ret += i->second->toString(indent + 1);
		}
		for(Nodes::const_iterator i = nodes.begin(); i != nodes.end(); ++i) {
			ret += std::string(indent, ' ') + i->first + "\n";
		}
		ret.erase(ret.end()-1);
		return ret;
	}
	std::string ls() const throw() {
		std::string ret;
		for(SubDirs::const_iterator i = subDirs.begin(); i != subDirs.end(); ++i) {
			ret += i->first + "/\n";
		}
		for(Nodes::const_iterator i = nodes.begin(); i != nodes.end(); ++i) {
			ret += i->first + "\n";
		}
		ret.erase(ret.end()-1);
		return ret;
	}	
	std::string toPath() const throw() {
		SubDirs::const_iterator i = subDirs.find("..");
		if(i == subDirs.end())
			return "/";
		return i->second->toPath() + name + '/';
	}
	Dir* cd() throw() {
		Dir* next = this;
		SubDirs::iterator i;
		while((i = next->subDirs.find("..")) != next->subDirs.end())
			next = i->second;
		return next;
	}
	Dir* cd(std::string const& path) throw() {
		Dir* next = NULL;
		// go to root
		if(path.empty() || path[0] == '/')
			next = cd();
		StringList sl = Util::lazyStringTokenize(path, '/');
		if(sl.empty())
			return next;
		// iterate through dirs from `next'
		if(!next)
			next = this;
		for(StringList::const_iterator i = sl.begin(); i != sl.end(); ++i) {
			// find complete match
			SubDirs::iterator j = next->subDirs.find(*i);
			if(j != next->subDirs.end()) {
				next = j->second;
			// partial matches work if not ambiguous
			} else if(next->partial) {
				SubDirs::iterator k = next->subDirs.end();
				for(j = next->subDirs.begin(); j != next->subDirs.end(); ++j) {
					if(j->first.length() >= i->length() && j->first.substr(0, i->length()) == *i) {
						if(k != next->subDirs.end())
							return NULL; // ambiguous
						else
							k = j;
					}
				}
				if(k == next->subDirs.end())
					return NULL;
				next = k->second;
			} else {
				return NULL;
			}
		}
		return next;
	}
	Dir* md(std::string const& path) throw() {
		if(path.empty() || path == "/")
			return NULL;
		std::string::size_type i = path.rfind('/');
		if(i == std::string::npos) { // no preceding path
			return (subDirs[path] = new Dir(path, this));
		} else {
			Dir* d = cd(path.substr(0, i));
			if(!d)
				return NULL;
			std::string l = path.substr(i + 1);
			return (d->subDirs[l] = new Dir(l, d));
		}
	}
	Dir* rd(std::string const& path) throw() {
		Dir* next = cd(path);
		if(!next || next->subDirs.size() != 1) // remove existing/empty dirs only
			return NULL;
		SubDirs::iterator i = next->subDirs.find("..");
		if(i == next->subDirs.end()) // trying to remove root
			return NULL;
		Dir* parent = i->second;
		assert((i = parent->subDirs.find(next->name)) != parent->subDirs.end()); // name must exist
		delete i->second;
		parent->subDirs.erase(i);
		return parent;
	}

	void setData(VirtualFsListener* dat) throw() {
		data = dat;
	}
	VirtualFsListener* getData() const throw() {
		return data;
	}
	
	Dir* splitPath(std::string& name) {
		std::string::size_type i = name.rfind('/');
		if(i == std::string::npos) { // no preceding path
			return this;
		} else {
			Dir* d = cd(name.substr(0, i));
			if(d) {
				name = name.substr(i + 1);
				return d;
			}
			return NULL;
		}
	}
	bool mkNode(std::string const& name, VirtualFsListener* data) throw() {
		std::string tmp = name;
		Dir* d = splitPath(tmp);
		if(d) {
			d->nodes[tmp] = data;
			return true;
		}
		return false;
	}
	bool rmNode(std::string const& name) throw() {
		std::string tmp = name;
		Dir* d = splitPath(tmp);
		if(d) {
			Nodes::iterator i = d->nodes.find(tmp);
			if(i != d->nodes.end()) {
				d->nodes.erase(i);
				return true;
			}
		}
		return false;	
	}
	VirtualFsListener* getNode(std::string& name) throw() {
		std::string tmp = name;
		Dir* d = splitPath(tmp);
		if(d) {
			// find complete match
			Nodes::const_iterator i = d->nodes.find(tmp);
			if(i != d->nodes.end())
				return i->second;
			// partial matches work if not ambiguous
			if(d->partial) {
				Nodes::const_iterator j = d->nodes.end();
				for(i = d->nodes.begin(); i != d->nodes.end(); ++i) {
					if(i->first.length() >= tmp.length() && i->first.substr(0, tmp.length()) == tmp) {
						if(j != d->nodes.end())
							return NULL; // ambiguous
						else
							j = i;
					}
				}
				if(j == d->nodes.end())
					return NULL;
				name = j->first; // tell caller what we found (cd has toPath())
				return j->second;
			} else {
				return NULL;
			}
		}
		return NULL;
	}

	void setPartialMatch(bool b) throw() { partial = b; };
	
private:
	std::string name;
	bool partial;
	typedef std::map<std::string, Dir*> SubDirs;
	SubDirs subDirs;
	VirtualFsListener* data;
	typedef std::map<std::string, VirtualFsListener*> Nodes;
	Nodes nodes;
};

} //namespace qhub

#endif //_INCLUDED_PLUGINS_DIR_H
