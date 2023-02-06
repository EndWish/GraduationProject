#include "stdafx.h"
#include "Room.h"
#include "ServerFramework.h"

Room::Room(UINT _id) : id{ _id } {
	hostID = 0;
	participants.reserve(MAX_PARTICIPANT);

	gameRunning = false;
}
Room::~Room() {

}

bool Room::EnterUser(UINT _clientID) {
	
	ServerFramework& serverFramework = ServerFramework::Instance();

	if (participants.size() == MAX_PARTICIPANT)
		return false;

	// 첫번째 참가자일 경우 방장으로 임명한다.
	if (participants.size() == 0)
		hostID = _clientID;

	participants.push_back(_clientID); // 참가자를 컨테이너에 추가한다.

	// 참가자의 상태와 접속한 룸번호를 변경한다.
	Client* pClient = serverFramework.GetClient(_clientID);
	pClient->SetClientState(ClientState::roomWait);
	pClient->SetCurrentRoom(this);

	// 2. 입장하는 플레이어에게 정보를 전송한다.
	SC_ROOM_PLAYERS_INFO sendPacket;
	sendPacket.roomID = GetID();
	sendPacket.hostID = GetHostID();
	sendPacket.nParticipant = GetNumOfParticipants();
	for (UINT i = 0; UINT participant : GetParticipants()) {
		Client* pParticipantClient = serverFramework.GetClient(participant);
		sendPacket.participantInfos[i].clientID = participant;
		sendPacket.participantInfos[i].ready = pParticipantClient->GetClientState() == ClientState::roomReady;
		++i;
	}
	SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);

	return true;
}
void Room::LeaveUser(UINT _clientID) {
	auto it = ranges::find(participants, _clientID);
	if (it != participants.end()) {	// 참가목록에 존재할 경우

		// 참가자의 상태와 접속한 룸번호를 변경한다.
		Client* client = ServerFramework::Instance().GetClient(_clientID);
		client->SetClientState(ClientState::lobby);	
		client->SetCurrentRoom(NULL);

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

void Room::GameStart() {
	gameRunning = true;
	ServerFramework::Instance().AddPlayInfo(id);
	cout << "playinfo " << id << " 생성 \n";
}
