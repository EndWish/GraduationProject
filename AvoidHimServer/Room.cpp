#include "stdafx.h"
#include "Room.h"
#include "ServerFramework.h"

Room::Room(UINT _id) : id{ _id } {
	hostID = 0;
	participants.reserve(maxParticipant);

	gameRunning = false;
}
Room::~Room() {

}

bool Room::EnterUser(UINT _clientID) {
	
	ServerFramework& serverFramework = ServerFramework::Instance();

	if (participants.size() == maxParticipant)
		return false;

	// ù��° �������� ��� �������� �Ӹ��Ѵ�.
	if (participants.size() == 0)
		hostID = _clientID;

	participants.push_back(_clientID); // �����ڸ� �����̳ʿ� �߰��Ѵ�.

	// �������� ���¿� ������ ���ȣ�� �����Ѵ�.
	Client* client = ServerFramework::Instance().GetClient(_clientID);
	client->SetClientState(ClientState::roomWait);
	client->SetCurrentRoom(this);

	// 2. �����ϴ� �÷��̾�� ������ �����Ѵ�.
	SC_ROOM_PLAYERS_INFO sendPacket1;
	sendPacket1.roomID = GetID();
	sendPacket1.hostID = GetHostID();
	sendPacket1.nParticipant = GetNumOfParticipants();
	for (UINT i = 0; UINT clientID : GetParticipants()) {
		sendPacket1.participantInfos[i].clientID = clientID;
		sendPacket1.participantInfos[i].ready = client->GetClientState() == ClientState::roomReady;
		++i;
	}
	send(client->GetSocket(), (char*)&sendPacket1, sizeof(SC_ROOM_PLAYERS_INFO), 0);
	cout << "�����ϴ� �÷��̾�� ������ �����Ѵ�.\n";

	return true;
}
void Room::LeaveUser(UINT _clientID) {
	auto it = ranges::find(participants, _clientID);
	if (it != participants.end()) {	// ������Ͽ� ������ ���

		// �������� ���¿� ������ ���ȣ�� �����Ѵ�.
		Client* client = ServerFramework::Instance().GetClient(_clientID);
		client->SetClientState(ClientState::lobby);	
		client->SetCurrentRoom(NULL);

		bool leaveHost = (_clientID == hostID);	// ȣ��Ʈ�� �������� Ȯ��
		participants.erase(it);	// �ش� ���� �����ɷ� ó��

		if (leaveHost && 0 < participants.size()) {	// ���� ȣ��Ʈ�� �Ӹ��Ѵ�.
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
	cout << "playinfo " << id << " ���� \n";
}
