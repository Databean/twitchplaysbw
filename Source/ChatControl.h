#ifndef CHAT_CONTROL
#define CHAT_CONTROL

#include "Mutex.h"
#include "IRCClient.h"

class ChatControl {
private:
	Mutex& gameMutex;
	
	ChatControl& operator=(const ChatControl&) {} //deleted
public:
	ChatControl(Mutex& gameMutex);
	ChatControl(ChatControl& o);
	ChatControl(ChatControl&&);
	~ChatControl();

	void operator()(const IRCMessage& message, IRCClient& client);
};

#endif