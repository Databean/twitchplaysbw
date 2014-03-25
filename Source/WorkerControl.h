#ifndef WORKER_CONTROL_H
#define WORKER_CONTROL_H

#include <BWAPI.h>

void initialMining();
bool buildAtLocation(BWAPI::Unit worker, BWAPI::UnitType type, BWAPI::TilePosition tile);
void buildAtClosestLocation(BWAPI::Unit worker, BWAPI::UnitType type);

#endif