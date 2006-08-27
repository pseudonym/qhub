#ifndef QHUB_SINGLETON_H
#define QHUB_SINGLETON_H

#include <boost/noncopyable.hpp>

namespace qhub {

template<typename T>
class Singleton : protected boost::noncopyable {
public:
	static T* instance()
	{
		if(!inst)
			inst = new T();
		return inst;
	}

protected:
	static T* inst;
};

template<typename T> T* Singleton<T>::inst = 0;

} // namespace qhub

#endif // QHUB_SINGLETON_H
