#include "WorkerControl.h"

#include <iostream>

#include <BWAPI.h>

using BWAPI::Unitset;
using BWAPI::Unit;
using BWAPI::Broodwar;
using BWAPI::UnitType;
using BWAPI::TilePosition;
using BWAPI::Color;

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

bool buildAtLocation(Unit worker, UnitType type, TilePosition tile) {
	if(worker->canBuild(type, tile)) {
		Broodwar->drawCircle(BWAPI::CoordinateType::Map, tile.x * 32, tile.y * 32, 1.2 * std::max(type.width(), type.height()), Color(255, 0 , 0));
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