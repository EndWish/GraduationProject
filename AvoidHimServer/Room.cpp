#include "stdafx.h"
#include "Room.h"
#include "ServerFramework.h"

Room::Room(UINT _id) : id{ _id } {
	hostID = 0;
	participants.reserve(maxParticipant);
}
Room::~Room() {

}

bool Room::EnterUser(UINT _clientID) {
	if (participants.size() == maxParticipant)
		return false;

	// 첫번째 참가자일 경우 반장으로 임명한다.
	if (participants.size() == 0)
		hostID = _clientID;

	participants.push_back(_clientID); // 참가자를 컨테이너에 추가한다.

	// 참가자의 상태와 접속한 룸번호를 변경한다.
	Client& client = ServerFramework::Instance().GetClient(_clientID);
	client.SetClientState(ClientState::roomWait);
	client.SetCurrentRoom(this);

	return true;
}
void Room::LeaveUser(UINT _clientID) {
	auto it = ranges::find(participants, _clientID);
	if (it != participants.end()) {	// 참가목록에 존재할 경우

		// 참가자의 상태와 접속한 룸번호를 변경한다.
		Client& client = ServerFramework::Instance().GetClient(_clientID);
		client.SetClientState(ClientState::lobby);	
		client.SetCurrentRoom(NULL);

		bool leaveHost = (_clientID == hostID);	// 호스트가 나갔는지 확인
		participants.erase(it);	// 해당 유저 나간걸로 처리

		if (leaveHost && 0 < participants.size()) {	// 새로 호스트를 임명한다.
			hostID = participants[0];
		}
	}

	if (participants.size() == 0) {
		ServerFramework::Instance().RemoveRoom(id);
	}
}
