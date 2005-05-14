#include <cstdio>
#include <string>

namespace qhub
{

std::FILE* qerr = stderr;
std::FILE* qstat = stdout;
std::FILE* qline = std::fopen("/dev/null", "w");

}
