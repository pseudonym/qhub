// vim:ts=4:sw=4:noet
#include "VirtualFs.h"

#include "Client.h"
#include "UserData.h"
#include "Logs.h"

#include "VirtualFsDir.h"

using namespace qhub;

/*
 * Plugin loader
 */

extern "C" {
	void* getPlugin() { return new VirtualFs(); }
} //extern "C"



/*
 * Plugin details
 */

UserData::key_type VirtualFs::idVirtualFs = "virtualfs";
UserData::key_type VirtualFs::idVirtualPath = "virtualpath";

/*void VirtualFs::Message::reply(string const& msg) throw()
{
	client->doPrivateMessage(msg);
}*/

void VirtualFs::init() throw()
{
	Util::data.setVoidPtr(idVirtualFs, this);
	root = new Dir;
	root->setPartialMatch(true);
	root->setData(this);
}

void VirtualFs::deinit() throw()
{
	delete root;
	Util::data.setVoidPtr(idVirtualFs, NULL);
}

void VirtualFs::on(PluginStarted&, Plugin* p) throw()
{
	if(p == this) {
		init();
		Logs::stat << "success: Plugin VirtualFs: Started.\n";
	}
}

void VirtualFs::on(PluginStopped&, Plugin* p) throw()
{
	if(p == this) {
		deinit();
		Logs::stat << "success: Plugin VirtualFs: Stopped.\n";
	}
}

/*void VirtualFs::on(PluginMessage&, Plugin* p, void* d) throw()
{
	if(p == this) {
		Message* m = static_cast<Message*>(d);
		assert(m);
		if(m->cwd == "" && m->type == Message::HELP) {
			m->reply("You are at the root of qhub::VirtualFs. Available commands: cd, help, ls, pwd");
		}
	}
}*/

void VirtualFs::on(Help, const string& cwd, Client* c) throw()
{
	assert(cwd.empty());
	c->doPrivateMessage("You are at the root of qhub::VirtualFs. Available commands: cd, help, ls, pwd");
}

bool VirtualFs::mkdir(string const& path, VirtualFsListener* p) throw()
{
	Dir* d = root->md(path);
	if(d) {
		d->setData(p);
		return true;
	} else {
		return false;
	}
}

bool VirtualFs::mknod(string const& path, VirtualFsListener* p) throw()
{
	return root->mkNode(path, p);
}

bool VirtualFs::rmdir(string const& path) throw()
{
	return root->rd(path);
}

bool VirtualFs::rmnod(string const& path) throw()
{
	return root->rmNode(path);
}

void VirtualFs::on(UserCommand& a, Client* client, const string& msg) throw()
{
	UserData* data = client->getUserData();
	StringList sl = Util::lazyQuotedStringTokenize(msg);
	if(sl.empty())
		return;

	a.setState(Plugin::STOP);

	string const& pwd = data->getString(idVirtualPath);
	size_t siz = sl.size();
	if(siz == 1 && sl[0] == "pwd") {
		Dir* d = root->cd(pwd);
		if(!d) {
			client->doPrivateMessage("NAK: Current working directory is invalid. Type \"cd /\".");
		} else {
			client->doPrivateMessage("ACK: " + d->toPath());
		}
	} else if(sl[0] == "ls" && (siz == 2 || siz == 1)) {
		Dir* d = root->cd(pwd);
		if(d) {
			if(siz == 1 || (d = d->cd(sl[1]))) {
				client->doPrivateMessage("ACK: " + d->toPath() + "\n" + d->ls());
			} else {
				client->doPrivateMessage("NAK: Path ambiguous or not found.");
			}
		} else {
			client->doPrivateMessage("NAK: Current working directory is invalid. Type \"cd /\".");
		}
	} else if(sl[0] == "cd" && (siz == 2 || siz == 1)) {
		Dir* d;
		if(siz == 1) {
			d = root;
		} else {
			d = root->cd(pwd);
			if(!d)
				d = root;
			d = d->cd(sl[1]);
		}
		if(!d) {
			client->doPrivateMessage("NAK: Path ambiguous or not found.");
		} else {
			client->doPrivateMessage("ACK: " + d->toPath());
			data->setString(idVirtualPath, d->toPath() == "/" ? Util::emptyString : d->toPath());
			VirtualFsListener* p = d->getData();
			if(p) {
				//Message m(Message::CHDIR, data->getString(idVirtualPath), client);
				ChDir action;
				p->on(action, data->getString(idVirtualPath), client);
			}
		}
	} else if(sl[0] == "help") {
		// add path arg support
		Dir* d = root->cd(pwd);
		if(!d || !d->getData()) {
			client->doPrivateMessage("NAK: Path invalid, ambiguous or no help available.");
		} else {
			VirtualFsListener* p = d->getData();
			//Message m(Message::HELP, pwd, client);
			Help action;
			p->on(action, pwd, client);
		}
	} else {
		Dir* d = root->cd(pwd);
		if(d && (d = d->splitPath(sl[0]))) {
			VirtualFsListener* p = d->getNode(sl[0]);
			if(p) {
				//Message m(Message::EXEC, sl[0], client, sl);
				Exec action;
				p->on(action, sl[0], client, sl);
			} else {
				client->doPrivateMessage("NAK: Command ambiguous or not found.");
			}
		} else {
			client->doPrivateMessage("NAK: Path not found.");
		}
	}
}
