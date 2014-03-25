#ifndef MUTEX_H
#define MUTEX_H

#include <Windows.h>

class Mutex {
private:
	HANDLE raw_mutex;

	Mutex(const Mutex&) {} //deleted
	Mutex& operator=(const Mutex&) { return *this; } //deleted
	Mutex& operator=(Mutex&&) { return *this; } //deleted
public:
	Mutex();
	
	~Mutex();

	void lock();
	void unlock();
};

class LockGuard {
private:
	Mutex& mutex;

	LockGuard(const LockGuard& o) : mutex(o.mutex) {} //deleted
	LockGuard& operator=(const LockGuard&) { return *this; } //deleted
	LockGuard& operator=(LockGuard&&) { return *this; } //deleted
public:
	LockGuard(Mutex&);
	~LockGuard();
};

#endif