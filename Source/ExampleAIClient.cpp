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
#include "WorkerControl.h"
#include "CameraControl.h"

#include <windows.h>

#include <string>

using namespace BWAPI;

void initialMining();
void handleMessage(const IRCMessage& message, IRCClient& client);

Mutex mutex;

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

	IRCClient client;
	client.HookIRCCommand("PRIVMSG", handleMessage);

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

void handleMessage(const IRCMessage& message, IRCClient& client) {
	std::string text = message.parameters.at(message.parameters.size() - 1);
	LockGuard guard(mutex);
	if(text == "build worker") {
		Unitset units = Broodwar->self()->getUnits();
		units.remove_if([](Unit i) { return !i->getType().isResourceDepot(); });
		if(units.size() > 0) {
			Unit u = units.rand();
			u->train(Broodwar->self()->getRace().getWorker());
			centerCameraOnUnit(u);
		}
	} else if(text == "send idle workers to mine") {
		Unitset units = Broodwar->self()->getUnits();
		for(auto it = units.begin(); it != units.end(); it++) {
			if(it->getType().isWorker() && it->isIdle()) {
				it->gather(it->getClosestUnit(Filter::IsMineralField));
				centerCameraOnUnit(*it);
			}
		}
	} else if(text == "build supply depot") {
		Unitset units = Broodwar->self()->getUnits();
		// Get all our workers
		units.remove_if([](Unit i) { return !i->getType().isWorker(); });
		units.remove_if([](Unit i) { return !i->canBuild(); });
		units.remove_if([](Unit i) { return !i->canBuild(UnitTypes::Terran_Supply_Depot); });
		if(units.size() > 0) {
			// Choose a random one
			Unit worker = units.rand();
			//Find the closest place to build one
			buildAtClosestLocation(worker, UnitTypes::Terran_Supply_Depot);
			centerCameraOnUnit(worker);
		}
	} else if(text == "build barracks") {
		Unitset units = Broodwar->self()->getUnits();
		// Get all our workers
		units.remove_if([](Unit i) { return !i->getType().isWorker(); });
		units.remove_if([](Unit i) { return !i->canBuild(); });
		units.remove_if([](Unit i) { return !i->canBuild(UnitTypes::Terran_Barracks); });
		if(units.size() > 0) {
			// Choose a random one
			Unit worker = units.rand();
			//Find the closest place to build one
			buildAtClosestLocation(worker, UnitTypes::Terran_Barracks);
			centerCameraOnUnit(worker);
		}
	} else if(text == "build marine") {
		Unitset units = Broodwar->self()->getUnits();
		units.remove_if([](Unit i) { return i->getType().getID() != UnitTypes::Terran_Barracks; });
		if(units.size() > 0) {
			Unit barracks = units.rand();
			if(barracks->train(UnitTypes::Terran_Marine)) {
				centerCameraOnUnit(barracks);
			}
		}
	} else if(text == "yolo marines") {
		Unitset units = Broodwar->self()->getUnits();
		units.remove_if([](Unit i) { return i->getType().getID() != UnitTypes::Terran_Marine; });
		
		Unitset enemyUnits = Broodwar->enemies()[0]->getUnits();
		enemyUnits.remove_if([](Unit i) { return !i->getType().isResourceDepot(); });
		if(enemyUnits.size() > 0) {
			
			for(auto it = units.begin(); it != units.end(); it++) {
				if(!it->attack(PositionOrUnit(enemyUnits.getPosition()))) {
					std::cout << "unit failed to attack" << std::endl;
				} else {
					std::cout << "successful attack!" << std::endl;
				}
				centerCameraOnUnit(*it);
			}
		} else {
			std::cout << "nothing to attack" << std::endl;
		}
	}
}

