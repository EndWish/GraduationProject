#include "stdafx.h"
#include "Room.h"
#include "ServerFramework.h"

Room::Room(int _id) : id{ _id } {
	host = NULL;
	participants.reserve(maxParticipant);
}
Room::~Room() {

}

bool Room::EnterUser(SOCKET _socket) {
	if (participants.size() == maxParticipant)
		return false;

	if (participants.size() == 0)
		host = _socket;
	participants.push_back(_socket);
	return true;
}

void Room::LeaveUser(SOCKET _socket) {
	auto it = ranges::find(participants, _socket);
	if (it != participants.end()) {	// 참가목록에 존재할 경우
		bool leaveHost = (*it == host);	// 호스트가 나갔는지 확인
		participants.erase(it);	// 해당 유저 나간걸로 처리

		if (leaveHost && 0 < participants.size()) {	// 새로 호스트를 임명한다.
			host = participants[0];
		}
	}

}
