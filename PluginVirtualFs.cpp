// vim:ts=4:sw=4:noet
#include "PluginVirtualFs.h"

#include "ADCClient.h"
#include "UserData.h"
#include "PluginVirtualFsDir.h"

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

UserData::Key VirtualFs::idVirtualFs = UserData::toKey("virtualfs");
UserData::Key VirtualFs::idVirtualPath = UserData::toKey("virtualpath");

void VirtualFs::Message::reply(string const& msg) throw()
{
	client->doPrivateMessage(msg);
}

void VirtualFs::init() throw()
{
	Plugin::data.setVoidPtr(idVirtualFs, this);
	root = new Dir;
	root->setPartialMatch(true);
	root->setData(this);
}

void VirtualFs::deinit() throw()
{
	delete root;
	Plugin::data.setVoidPtr(idVirtualFs, NULL);
}

void VirtualFs::on(PluginStarted, Plugin* p) throw()
{
	if(p == this) {
		init();
		fprintf(stderr, "success: Plugin VirtualFs: Started.\n");
	}
}

void VirtualFs::on(PluginStopped, Plugin* p) throw()
{
	if(p == this) {
		deinit();
		fprintf(stderr, "success: Plugin VirtualFs: Stopped.\n");
	}
}

void VirtualFs::on(PluginMessage, Plugin* p, void* d) throw()
{
	if(p == this) {
		Message* m = (Message*)d;
		assert(d);
		if(m->cwd == "" && m->type == Message::HELP) {
			m->client->doPrivateMessage("You are at the root of qhub::VirtualFs. Available commands: cd, help, ls, pwd");
		}
	}
}

bool VirtualFs::mkdir(string const& path, Plugin* p) throw()
{
	Dir* d = root->md(path);
	if(d) {
		d->setData(p);
		return true;
	} else {
		return false;
	}
}

bool VirtualFs::mknod(string const& path, Plugin* p) throw()
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

void VirtualFs::on(ClientCommand, ADCClient* client, string& msg) throw()
{
	UserData* data = client->getData();
	StringList sl = Util::lazyQuotedStringTokenize(msg);
	if(sl.empty())
		return;
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
				client->doPrivateMessage("ACK: " + d->toPath() + "\r\n" + d->ls());
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
			Plugin* p = (Plugin*)d->getData();
			if(p) {
				Message m(Message::CHDIR, data->getString(idVirtualPath), client);
				p->on(PluginMessage(), this, &m);
			}
		}
	} else if(sl[0] == "help") {
		// add path arg support
		Dir* d = root->cd(pwd);
		if(!d || !d->getData()) {
			client->doPrivateMessage("NAK: Path invalid, ambiguous or no help available.");
		} else {
			Plugin* p = (Plugin*)d->getData();
			Message m(Message::HELP, pwd, client);
			p->on(PluginMessage(), this, &m);
		}
	} else {
		Dir* d = root->cd(pwd);
		if(d && (d = d->splitPath(sl[0]))) {
			Plugin* p = (Plugin*)d->getNode(sl[0]);
			if(p) {
				Message m(Message::EXEC, sl[0], client, sl);
				p->on(PluginMessage(), this, &m);
			} else {
				client->doPrivateMessage("NAK: Command ambiguous or not found.");
			}
		} else {
			client->doPrivateMessage("NAK: Path not found.");
		}
	}		
}
