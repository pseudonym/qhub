// vim:ts=4:sw=4:noet
#ifndef QHUB_QHUB_H
#define QHUB_QHUB_H

// do this from one centralized place
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
// going to assume it defines all the ones we need
#else
// guess at what it should be; rest are defined by inet headers
#warning "stdint.h does not exist, guessing definitions for sized integer types"
typedef long long int int64_t;
typedef unsigned long long int uint64_t;
#endif

namespace qhub {

typedef uint32_t sid_type;

// const in anonymous namespace is better than #define
namespace {
const sid_type INVALID_SID = sid_type(-1);
}

// forward declarations for classes
class ADC;
class ADCSocket;
class Buffer;
class Client;
class ClientManager;
class Command;
class ConnectionBase;
class ConnectionManager;
class DnsManager;
class Encoder;
class EventManager;
class Hub;
class InterHub;
class Logs;
class Plugin;
class PluginManager;
class ServerManager;
class ServerSocket;
class Settings;
class Socket;
class TigerHash;
class TokenBucket;
class UserData;
class UserInfo;
class Util;
class XmlTok;
class ZBuffer;

} // namespace qhub

#endif // QHUB_QHUB_H
