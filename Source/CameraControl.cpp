#include "CameraControl.h"

#include <BWAPI.h>

using BWAPI::Broodwar;
using BWAPI::Position;
using BWAPI::Unit;

void centerCameraOnPoint(Position p) {
	if(p == BWAPI::Positions::Unknown) {
		return;
	}
	Broodwar->setScreenPosition(p - Position(300, 200)); //experimental testing
}

void centerCameraOnUnit(Unit unit) {
	if(!unit) {
		return;
	}
	Position p = unit->getPosition();
	if(p == BWAPI::Positions::Unknown) {
		return;
	}
	centerCameraOnPoint(p);
}