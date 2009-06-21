// vim:ts=4:sw=4:noet
#include "TokenBucket.h"

#include <algorithm>
#include <ctime>
#include <numeric>

using namespace std;
using namespace qhub;

TokenBucket::TokenBucket(int m, int inc, int in)
		: max(m), increment(inc), interval(in)
{
	count = max;
	gettimeofday(&last, NULL);
}

bool TokenBucket::use(int amt)
{
	const int million = 1000*1000;
	timeval now_tv;
	gettimeofday(&now_tv, NULL);
	int64_t now = now_tv.tv_sec * million + now_tv.tv_usec;
	int64_t l = last.tv_sec * million + last.tv_usec;
	int64_t diff = now - l;
	int64_t add = diff / interval;
	l = l + add * interval;
	last.tv_sec = l / million;
	last.tv_usec = l % million;

	count = min<int64_t>(this->max, count + add * increment);
	if(count < amt)
		return false;

	count =- amt;
	return true;
}

