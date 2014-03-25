#include "Mutex.h"

#include <stdexcept>
#include <string>

#include <windows.h>

Mutex::Mutex() {
	raw_mutex = CreateMutex( 
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex
	if (raw_mutex == NULL) {
		throw std::runtime_error("CreateMutex error: " + std::string(itoa(GetLastError(), NULL, 10)) + "\n");
	}
}

Mutex::~Mutex() {
	if(raw_mutex != nullptr) {
		CloseHandle(raw_mutex);
	}
}

void Mutex::lock() {
	WaitForSingleObject( 
		raw_mutex,    // handle to mutex
		INFINITE);  // no time-out interval
}

void Mutex::unlock() {
	ReleaseMutex(raw_mutex);
}

LockGuard::LockGuard(Mutex& mutex) : mutex(mutex) {
	mutex.lock();
}

LockGuard::~LockGuard() {
	mutex.unlock();
}

