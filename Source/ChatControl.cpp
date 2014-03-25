#define _WINSOCKAPI_    // stops windows.h including winsock.h
#define NOMINMAX

#include "ChatControl.h"

#include <BWAPI.h>

#include "IRCClient.h"
#include "CameraControl.h"
#include "WorkerControl.h"
#include "Mutex.h"

using namespace BWAPI;

ChatControl::ChatControl(Mutex& gameMutex) : gameMutex(gameMutex) {

}

ChatControl::ChatControl(ChatControl& other) : gameMutex(other.gameMutex) {

}

ChatControl::ChatControl(ChatControl&& control) : gameMutex(control.gameMutex) {

}

ChatControl::~ChatControl() {

}

void ChatControl::operator()(const IRCMessage& message, IRCClient& client) {
	std::string text = message.parameters.at(message.parameters.size() - 1);
	LockGuard guard(gameMutex);
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
				it->gather(it->getClosestUnit(BWAPI::Filter::IsMineralField));
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