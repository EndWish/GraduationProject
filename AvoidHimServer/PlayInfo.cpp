#include "stdafx.h"
#include "PlayInfo.h"
#include "ServerFramework.h"

PlayInfo::PlayInfo(UINT _playInfoID) : playInfoID{ _playInfoID } {
	allPlayerLoadingComplete = false;
	professorID = 0;

	zone = Zone(XMFLOAT3(100, 100, 100), XMINT3(10, 10, 10));
	timer = Timer();
	professorID = 0;
}
PlayInfo::~PlayInfo() {
	for (UINT participant : participants) {
		Client* pClient = ServerFramework::Instance().GetClient(participant);
		pClient->SetCurrentPlayInfo(NULL);
		if (pClient->IsDisconnected()) {
			ServerFramework::Instance().RemoveClient(participant);
		}
	}
}

void PlayInfo::Init(UINT _roomID) {
    Room* pRoom = ServerFramework::Instance().GetRoom(_roomID);

	participants = pRoom->GetParticipants();
	for (UINT participant : participants)
		loadingCompletes[participant] = false;

    // ���� �÷��̾ ���Ѵ�.
	uniform_int_distribution<int> uid(0, (int)participants.size() - 1);
	professorID = uid(rd);
	
	for (UINT participant : participants) {
	    Client* client = ServerFramework::Instance().GetClient(participant);
	    client->SetClientState(ClientState::roomPlay);	// �÷��̾��� ���¸� �ٲ۴�.
		client->SetCurrentPlayInfo(this);
	}

	// Ȱ��ȭ�� ��ǻ�Ϳ� ������ ���Ѵ�.

	// ���ڰ��鿡�� ������ ���۵Ǿ��ٴ� ��Ŷ�� �����Ѵ�.
	for (UINT participant : participants) {
		SC_GAME_START sendPacket;
		sendPacket.professorClientID = professorID;
		send(ServerFramework::Instance().GetClient(participant)->GetSocket(), (char*)&sendPacket, sizeof(SC_GAME_START), 0);
	}

}

void PlayInfo::LoadingComplete(UINT _clientID) {
	if (loadingCompletes.contains(_clientID) && !allPlayerLoadingComplete) {
		loadingCompletes[_clientID] = true;

		// ���� ��� �÷��̾ �ε��� �Ϸ�Ǿ����� Ȯ���Ѵ�.
		bool success = true;
		for (auto [clientID, loadingComplete] : loadingCompletes) {
			if (!loadingComplete)
				success = false;
		}

		// ��� �÷��̾ �ε��� �Ϸ�Ǿ��ٸ�
		if (success) {
			ProcessLoadingComplete();
		}
	}
}

void PlayInfo::ProcessLoadingComplete() {
	allPlayerLoadingComplete = true;

	// ��� �÷��̾ �ε��� �Ϸ�Ǿ��ٰ� ��Ŷ�� ������.
	SC_All_PLAYER_LOADING_COMPLETE sendPacket;
	for (UINT participant : participants)
		send(ServerFramework::Instance().GetClient(participant)->GetSocket(), (char*)&sendPacket, sizeof(SC_All_PLAYER_LOADING_COMPLETE), 0);
}

