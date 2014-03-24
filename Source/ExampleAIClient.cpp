#define _WINSOCKAPI_    // stops windows.h including winsock.h
#define NOMINMAX

#include <cstdio>

#include <BWAPI.h>
#include <BWAPI/Client.h>
#include <iostream>
#include <algorithm>

#include "IRCClient.h"
#include "Thread.h"

#include <windows.h>

#include <string>

using namespace BWAPI;

void initialMining();
void handleMessage(IRCMessage message, IRCClient* client);

HANDLE ghMutex; 

void reconnect()
{
	while(!BWAPIClient.connect())
	{
		Sleep(1000);
	}
}

void lock() {
	WaitForSingleObject( 
				ghMutex,    // handle to mutex
				INFINITE);  // no time-out interval
}

void unlock() {
	ReleaseMutex(ghMutex);
}

void clientThread(void* v_client) {
	IRCClient* client = (IRCClient*)v_client;
	while(client->Connected()) {
		std::cout << "receving data." << std::endl;
		client->ReceiveData();
	}
}

int main(int argc, const char* argv[])
{
	ghMutex = CreateMutex( 
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex

	if (ghMutex == NULL) 
    {
        printf("CreateMutex error: %d\n", GetLastError());
        return 1;
    }

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
		lock();

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

		unlock();

		while(running) {
			lock();
			
			running = Broodwar->isInGame();

			BWAPI::BWAPIClient.update();
			if (!BWAPI::BWAPIClient.isConnected())
			{
				std::cout << "Reconnecting..." << std::endl;
				reconnect();
			}
			unlock();
		}
		std::cout << "Game ended" << std::endl;
	}
	std::cout << "Press ENTER to continue..." << std::endl;
	std::cin.ignore();
	return 0;
}

void centerCameraOnPoint(Position p) {
	if(p == Positions::Unknown) {
		return;
	}
	Broodwar->setScreenPosition(p - Position(300, 200)); //experimental testing
}

void centerCameraOnUnit(UnitInterface* unit) {
	if(!unit) {
		return;
	}
	Position p = unit->getPosition();
	if(p == Positions::Unknown) {
		return;
	}
	centerCameraOnPoint(p);
}

void initialMining() {
	//send each worker to the mineral field that is closest to it
	Unitset units    = Broodwar->self()->getUnits();
	Unitset minerals  = Broodwar->getMinerals();
	for ( Unitset::iterator i = units.begin(); i != units.end(); ++i )
	{
		if ( i->getType().isWorker() )
		{
			Unit closestMineral = NULL;

			for( Unitset::iterator m = minerals.begin(); m != minerals.end(); ++m )
			{
				if ( !closestMineral || i->getDistance(*m) < i->getDistance(closestMineral))
					closestMineral = *m;
			}
			if ( closestMineral )
				i->rightClick(closestMineral);
		}
		else if ( i->getType().isResourceDepot() )
		{
			//if this is a center, tell it to build the appropiate type of worker
			i->train(Broodwar->self()->getRace().getWorker());
		}
	}
}

bool strBeginsWith(const std::string& str, const std::string& begin) {
	return str.substr(0, begin.size()) == begin;
}

std::vector<std::string> strSplit(const std::string& str, char splitOn) {
	std::vector<std::string> ret;
	std::string current;
	for(std::size_t i = 0; i < str.size(); i++) {
		if(str[i] == splitOn) {
			ret.push_back(current);
			current = "";
		} else {
			current.push_back(str[i]);
		}
	}
	ret.push_back(current);
	return std::move(ret);
}

bool buildAtLocation(Unit worker, UnitType type, TilePosition tile) {
	if(worker->canBuild(type, tile)) {
		Broodwar->drawCircle(CoordinateType::Map, tile.x * 32, tile.y * 32, 1.2 * std::max(type.width(), type.height()), Color(255, 0 , 0));
		Unitset closestOtherUnits = Broodwar->getUnitsInRadius(tile.x * 32, tile.y * 32, 1.2 * std::max(type.width(), type.height()));
		closestOtherUnits.remove_if([worker](Unit u) { return u == worker; });
		if(closestOtherUnits.size() == 0) {
			return worker->build(type, tile);
		}
	}
	return false;
}

void buildAtClosestLocation(Unit worker, UnitType type) {
	TilePosition tile = worker->getTilePosition();
	if(buildAtLocation(worker, type, tile)) {
		std::cout << "built successfully!" << std::endl;
		return;
	}
	for(int radius = 1; radius < 200; radius++) {
		//Spiral out
		tile.x++;
		for(int j = 0; j < radius; j++) {
			tile.x--;
			tile.y--;
			if(buildAtLocation(worker, type, tile)) {
				std::cout << "built successfully!" << std::endl;
				return;
			}
		}
		for(int j = 0; j < radius; j++) {
			tile.x--;
			tile.y++;
			if(buildAtLocation(worker, type, tile)) {
				std::cout << "built successfully!" << std::endl;
				return;
			}
		}
		for(int j = 0; j < radius; j++) {
			tile.x++;
			tile.y++;
			if(buildAtLocation(worker, type, tile)) {
				std::cout << "built successfully!" << std::endl;
				return;
			}
		}
		for(int j = 0; j < radius; j++) {
			tile.x++;
			tile.y--;
			if(buildAtLocation(worker, type, tile)) {
				std::cout << "built successfully!" << std::endl;
				return;
			}
		}
	}
}

void handleMessage(IRCMessage message, IRCClient* client) {
	std::string text = message.parameters.at(message.parameters.size() - 1);
	if(text == "build worker") {
		lock();
		Unitset units = Broodwar->self()->getUnits();
		units.remove_if([](Unit i) { return !i->getType().isResourceDepot(); });
		if(units.size() > 0) {
			Unit u = units.rand();
			u->train(Broodwar->self()->getRace().getWorker());
			centerCameraOnUnit(u);
		}
		unlock();
	} else if(text == "send idle workers to mine") {
		lock();
		Unitset units = Broodwar->self()->getUnits();
		for(auto it = units.begin(); it != units.end(); it++) {
			if(it->getType().isWorker() && it->isIdle()) {
				it->gather(it->getClosestUnit(Filter::IsMineralField));
				centerCameraOnUnit(*it);
			}
		}
		unlock();
	} else if(text == "build supply depot") {
		lock();
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
		unlock();
	} else if(text == "build barracks") {
		lock();
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
		unlock();
	} else if(text == "build marine") {
		lock();
		Unitset units = Broodwar->self()->getUnits();
		units.remove_if([](Unit i) { return i->getType().getID() != UnitTypes::Terran_Barracks; });
		if(units.size() > 0) {
			Unit barracks = units.rand();
			if(barracks->train(UnitTypes::Terran_Marine)) {
				centerCameraOnUnit(barracks);
			}
		}
		unlock();
	} else if(text == "yolo marines") {
		lock();
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
		unlock();
	}
}

