#define _WINSOCKAPI_    // stops windows.h including winsock.h
#define NOMINMAX

#include <cstdio>

#include <BWAPI.h>
#include <BWAPI/Client.h>
#include <iostream>
#include <algorithm>

#include "IRCClient.h"
#include "Thread.h"
#include "Mutex.h"
#include "ChatControl.h"
#include "WorkerControl.h"

#include <windows.h>

#include <string>

using namespace BWAPI;

void reconnect()
{
	while(!BWAPIClient.connect())
	{
		Sleep(1000);
	}
}

void clientThread(void* v_client) {
	IRCClient* client = (IRCClient*)v_client;
	while(client->Connected()) {
		std::cout << "receving data." << std::endl;
		client->ReceiveData();
	}
}

int main(int argc, const char* argv[]) {

	std::string host = "irc.freenode.net";
	int port = 8000;

	std::string nickname = "twitchplaysbw";
	std::string username = "twitchplaysbw";

	Mutex mutex;

	IRCClient client;
	client.HookIRCCommand("PRIVMSG", ChatControl(mutex));

	if(!client.InitSocket()) {
		std::cerr << "Unable to init socket" << std::endl;
		return -1;
	}
	if(!client.Connect(host.c_str(), port)) {
		std::cerr << "Unable to connect" << std::endl;
		return -1;
	}
	if(!client.Login(nickname, username)) {
		std::cerr << "Unable to login" << std::endl;
		return -1;
	}

	client.SendIRC("JOIN #twitchplaysbw");
	
	Thread thread;
	thread.Start(&clientThread, &client);

	std::cout << "Connecting..." << std::endl;;
	reconnect();
	while(true)
	{
		mutex.lock();

		std::cout << "waiting to enter match" << std::endl;
		while ( !Broodwar->isInGame() )
		{
			BWAPI::BWAPIClient.update();
			if (!BWAPI::BWAPIClient.isConnected())
			{
				std::cout << "Reconnecting..." << std::endl;;
				reconnect();
			}
		}
		std::cout << "starting match!" << std::endl;
		Broodwar->sendText("Hello world!");
		Broodwar << "The map is " << Broodwar->mapName() << ", a " << Broodwar->getStartLocations().size() << " player map" << std::endl;
		// Enable some cheat flags
		Broodwar->enableFlag(Flag::UserInput);
		// Uncomment to enable complete map information
		//Broodwar->enableFlag(Flag::CompleteMapInformation);

		initialMining();

		Broodwar << "The match up is " << Broodwar->self()->getRace() << " vs " << Broodwar->enemy()->getRace() << std::endl;

		bool running = Broodwar->isInGame();

		mutex.unlock();

		while(running) {
			LockGuard guard(mutex);
			
			running = Broodwar->isInGame();

			BWAPI::BWAPIClient.update();
			if (!BWAPI::BWAPIClient.isConnected())
			{
				std::cout << "Reconnecting..." << std::endl;
				reconnect();
			}
		}
		std::cout << "Game ended" << std::endl;
	}
	std::cout << "Press ENTER to continue..." << std::endl;
	std::cin.ignore();
	return 0;
}

