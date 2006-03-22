#ifndef QHUB_ID_H
#define QHUB_ID_H

#include <string>

namespace qhub {

typedef uint32_t sid_type;
typedef std::string cid_type;
typedef std::string pid_type;

#define INVALID_SID (sid_type(0))
#define HUB_SID (sid_type(0x41414141)) // "AAAA" as an integer

} // namespace qhub

#endif // QHUB_ID_H
