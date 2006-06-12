#ifndef _token_h_
#define _token_h_

namespace qhub {

class work
{
public:
	work(int work, int priority) : consume(work), prio(priority) {};
	virtual void doWork(void) = 0;
	virtual bool operator<(const work& a) { return prio<a.prio;};
protected:
	int consume;
	int prio;
};

class TokenBucket
{
public:
	TokenBucket();

	void setTokensPerSecond(uint64_t tokens);
private:
	//which frequency to add with, measured in nanoseconds
	//0 means nothing gets through
	uint64_t frequency;
	//how much to add each time
	uint64_t step;
};

}
#endif
