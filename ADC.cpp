#include "ADC.h"
#include "qhub.h"
#include "Hub.h"

#define START_BUFFER 1024
#define READ_SIZE 512

using namespace std;
using namespace qhub;

ADC::ADC(int fd, Hub* parent) : hub(parent), state(START),
		readBuffer(new unsigned char[START_BUFFER]),
		readBufferSize(START_BUFFER), rbCur(0), added(false)
{
	this->fd = fd;
	struct linger so_linger;
	// Set linger to false
	so_linger.l_onoff = false;
	so_linger.l_linger = 0;
	int itmp = setsockopt(fd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));
	if(itmp != 0){
		fprintf(stderr, "Error setting SO_LINGER\n");
	}

	enable(fd, OOP_READ, this);
}

ADC::~ADC()
{
	realDisconnect();
	delete[] readBuffer;
}

void ADC::growBuffer()
{
	unsigned char* tmp = new unsigned char[readBufferSize*2];
	memmove(tmp, readBuffer, readBufferSize);
	readBufferSize *= 2;

	delete[] readBuffer;
	readBuffer = tmp;

	fprintf(stderr, "Growing buffer to %d\n", readBufferSize);
}

void ADC::checkParms()
{
	//check so that user isnt DOSing us with INF-data.
	if(INF.size() > 256){
		state = PROTOCOL_ERROR;
		disconnect();
	}
}

//XXX: putting them ALL in a hashmap could be worth it, simpler
void ADC::getParms(int length, int positionalParms)
{
	posParms.clear();
	namedParms.clear();
	//No parameters possible (this catches one case: length==5, which is an odd one)
	if(length<6){
		return;
	}
	//Retrieve all parameters in command

	string current;

	int index=5;
	int state=0;//0==space, 1==parameter
	bool done=false;
	if(positionalParms>0){
		while(index<length-1 && !done){
			switch(state){
			case 0:
				if(readBuffer[index] != ' '){
					state = 1;
					//this is start of new parameter
					current.clear();
					positionalParms--;
					current += readBuffer[index];
				}
				break;
			case 1:
				switch(readBuffer[index]){
				case ' ':
					state = 0;
					//fprintf(stderr, "Adding positional parm >%s<\n", current.c_str());
					posParms.push_back(current);
					if(positionalParms==0){
						//fprintf(stderr, "Aborting posparms %d\n", index);
						done=true;
					}
					break;
				case '\\':
					current += readBuffer[index];
					index++;
					if(index<length){
						current += readBuffer[index];
					}
					break;
				default:
					current += readBuffer[index];
					break;
				}
				break;
			}
			index++;
		}
	}

	if(state == 1){
		//fprintf(stderr, "Adding positional parm >%s<\n", current.c_str());
		posParms.push_back(current);
	}

	//Named parameters
	//while(index<length && readBuffer[index] && readBuffer[index++]!=' ');

	state = 0;//0==space, 1==parm-name1, 2==parm-name2, 3==parm-value

	//unsigned isnt liked by hash_map
	string name = "";

	fprintf(stderr, "index %d\n", index);

	while(index<length-1){
		switch(state){
		case 0:
			if(readBuffer[index] != ' '){
				state = 1;
				name.clear();
				name += readBuffer[index];
			} else {
				//parse error
			}
			break;
		case 1:
			if(readBuffer[index] != ' '){
				state = 2;
				name += readBuffer[index];
			} else {
				//parse error
			}
			break;
		case 2:
			current.clear();
			if(readBuffer[index] != ' '){
				state = 3;
			} else {
				state = 0;
				//empty parm
				namedParms[name] = current;
				//fprintf(stderr, "Adding empty named parm %c%c\n", name[0], name[1]);
			}
		case 3:
			switch(readBuffer[index]){
			case ' ':
				state = 0;
				//fprintf(stderr, "Adding named parm %s %s\n", name.c_str(), current.c_str());
				namedParms[name] = current;
				break;
			case '\\':
				current += readBuffer[index];
				index++;
				if(index<length){
					current += readBuffer[index];
				}
				break;
			default:
				current += readBuffer[index];
				break;
			}
			break;
		}
		index++;
	}
	if(state == 3){
		//fprintf(stderr, "Adding named parm %s %s\n", name.c_str(), current.c_str());
		namedParms[name] = current;
	}
}

string ADC::getFullInf()
{
	string parms = "";
	for(INFIterator i = INF.begin(); i!=INF.end(); i++){
		parms += " " + i->first + i->second;
	}
	return "BINF " + guid + parms + "\n";
}

void ADC::sendFullInf()
{
	hub->broadcastSelf(this, getFullInf());
}

void ADC::realDisconnect()
{
	if(fd == -1){
		//already done
		return;
	}
	fprintf(stderr, "Disconnecting %d %p GUID: %s\n", fd, this, guid.c_str());
	if(added){
		//dont remove us if we werent added
		hub->removeClient(guid);

		//send a QUI aswell
		hub->broadcast(this, string("IQUI " + guid + " ND\n"));
	}
	close(fd);
	cancel(fd, OOP_READ);
	if(writeEnabled){
		cancel(fd, OOP_WRITE);
	}

	fd = -1;

	//what about deleting us?
	delete this;
}

void ADC::handleBCommand(int length)
{
	if(length<5){
		return;
	}

	switch(readBuffer[1]){
	case 'I':
		if(readBuffer[2] == 'N' && readBuffer[3] == 'F'){
			if(state == GOT_SUP){
				getParms(length, 1);
				if(posParms.size()<1){
					fprintf(stderr, "Malformed parms\n");
					state = PROTOCOL_ERROR;
					disconnect();
					return;
				} else {
					for(parmMapIterator i = namedParms.begin(); i!=namedParms.end(); i++){
						//fprintf(stderr, "Name: %s Val: %s\n", i->first.c_str(), i->second.c_str());
						if(i->first == "I4" && i->second == "0.0.0.0") {
							INF[i->first] = getPeerName();
						} else {
							INF[i->first] = i->second;
						}
					}
					//we know the guid, now register us in the hub.
					//fprintf(stderr, "Registering us with guid %s\n", guid.c_str());

					//XXX: when DC++ isnt buggy
					/*if(namedParms.find("LO") == namedParms.end() || namedParms["LO"] != "1"){
						//no LO1 login-tag
						state = PROTOCOL_ERROR;
						disconnect();
						return;
					}*/

					//send userlist, will buffer for us
					hub->getUsersList(this);

					guid = posParms[0];
					//add us later, dont want us two times
					if(!hub->addClient(this, posParms[0])){
						disconnect();
						return;
					}
					//only set this when we are sure that we are added, ie. here!
					added = true;

					//notify him that userlist is over
					sendFullInf();
					state = LOGGED_IN;
					hub->motd(this);
					//XXX: when DC++ isnt buggy
					//namedParms.erase(namedParms.find("LO"));
				}
			} else {
				//regular update
				for(parmMapIterator i = namedParms.begin(); i!=namedParms.end(); i++){
					//fprintf(stderr, "Name: %s Val: %s\n", i->first.c_str(), i->second.c_str());
					INF[string(i->first)] = i->second;
				}
				//broadcast it aswell, to self?
				string tmp((char*)readBuffer, length);
				hub->broadcastSelf(this, tmp);
			}
		}
		break;
	default:
		{
			string tmp((char*)readBuffer, length);
			hub->broadcastSelf(this, tmp);
		}
		break;
	}

}

void ADC::handleDCommand(int length)
{
	if(length<5 || state != LOGGED_IN){
		return;
	}

	//only care about guid
	getParms(length, 2);

	string tmp((char*)readBuffer, length);
	fprintf(stderr, "%d\n", posParms.size());
	fprintf(stderr, "%s vs. %s and %s\n", posParms[1].c_str(), tmp.c_str(), posParms[0].c_str());
	hub->direct(posParms[1], tmp);
}

void ADC::sendHubMsg(string msg)
{
	Buffer::writeBuffer tmp(new Buffer(string("BMSG FQI2LLF4K5W3Y " + escape(msg) + "\n"), 0));
	w(tmp);
}

string ADC::escape(string in)
{
	string tmp;
	tmp.reserve(255);
	for(int i=0; i<in.size(); i++){
		switch(in[i]){
case ' ': case 0x0a: case '\\':
			tmp += '\\'; tmp += in[i];
			break;
		default:
			tmp += in[i];
		}
	}
	return tmp;
}

void ADC::handleHCommand(int length)
{
	if(length<5){
		return;
	}

	switch(readBuffer[1]){
	case 'S':
		if(readBuffer[2] == 'U' && readBuffer[3] == 'P'){
			if(state == START){
				Buffer::writeBuffer tmp(new Buffer(string("ISUP FQI2LLF4K5W3Y +BASE\nIINF FQI2LLF4K5W3Y NI"
				                                   + escape(hub->getHubName()) + " HU1 HI1 DEmajs VEqhub0.02\n"), 0));
				w(tmp);
				state = GOT_SUP;
			} else {
				state = PROTOCOL_ERROR;
				disconnect();
			}
		} else if(readBuffer[2] == 'N' && readBuffer[3] == 'D'){
		}
		break;
	case 'P':
		//PAS
		if(state == GOT_SUP){
			//check password
		} else {
			state = PROTOCOL_ERROR;
			disconnect();
		}
		break;
	case 'D':
		//DSC
		fprintf(stderr, "DSC gotten\n");
		disconnect();
		return;
		break;
	case 'G':
		//GET
		break;
	}
}

void ADC::handleCommand(int length)
{
	//there is a command in readBuffer, starting at index 0.
	//fprintf(stderr, "Command: %d ", length);
	/*for(int i=0; i<length-1; i++){
		fprintf(stderr, "%c", readBuffer[i]);
	}
	fprintf(stderr, ">\n");*/

	switch(readBuffer[0]){
	case 'H':
		handleHCommand(length);
		break;
	case 'B':
		handleBCommand(length);
		break;
	case 'D':
		handleDCommand(length);
		break;
	}

	//now, this command is handled. Remove it.
	//XXX: memmove could be eliminated by using a rotating buffer
	memmove(readBuffer, readBuffer+length, readBufferSize-length);
	rbCur -= length;
}

void ADC::on_read()
{
	fprintf(stderr, "ON_READ\n");
	while(rbCur+READ_SIZE >= readBufferSize){
		growBuffer();
	}
	int r = read(fd, readBuffer+rbCur, READ_SIZE);
	if(r > 0){
		fprintf(stderr, "Got data %d %d %d\n", rbCur, r, readBufferSize);
		rbCur += r;

		//look through data until no more left?
		//XXX: could be optimised, start from rbCur instead
		bool done=false;
		while(rbCur>0 && !done){
			for(int i=0; i<rbCur; i++){
				if(i>hub->getMaxPacketSize()){
					state = PROTOCOL_ERROR;
					disconnect();
					break;
				}
				if(readBuffer[i] == '\\'){
					i++;
				} else if(readBuffer[i] == 0x0a){
					handleCommand(i+1);
					break;
				}
				if(i>=rbCur-1){
					done=true;
				}
			}
		}
	} else if(r < 1){
		fprintf(stderr, "Got -1 from read\n");
		disconnect();
	}

	if(disconnected){
		realDisconnect();
	}
}

void ADC::on_write()
{
	fprintf(stderr, "On_write\n");

	//in Socket
	partialWrite();

	if(queue.empty()){
		cancel(fd, OOP_WRITE);
		writeEnabled=false;
	}

	if(disconnected){
		realDisconnect();
	}
}
