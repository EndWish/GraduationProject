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
	if (it != participants.end()) {	// ������Ͽ� ������ ���
		bool leaveHost = (*it == host);	// ȣ��Ʈ�� �������� Ȯ��
		participants.erase(it);	// �ش� ���� �����ɷ� ó��

		if (leaveHost && 0 < participants.size()) {	// ���� ȣ��Ʈ�� �Ӹ��Ѵ�.
			host = participants[0];
		}
	}

}
