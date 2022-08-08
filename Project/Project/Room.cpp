#include "stdafx.h"
#include "Room.h"


Room::Room() {

}

Room::~Room() {

}

void Room::AnimateObjects(double _timeElapsed) {

	
}

const BoundingOrientedBox& Room::GetBoundingBox() const {
	return boundingBox;
}

const vector<weak_ptr<Room>>& Room::GetSideRooms() const {
	return pSideRooms;
}