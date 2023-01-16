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

    // 교수 플레이어를 정한다.
	uniform_int_distribution<int> uid(0, (int)participants.size() - 1);
	professorID = uid(rd);
	
	for (UINT participant : participants) {
	    Client* client = ServerFramework::Instance().GetClient(participant);
	    client->SetClientState(ClientState::roomPlay);	// 플레이어의 상태를 바꾼다.
		client->SetCurrentPlayInfo(this);
	}

	// 활성화될 컴퓨터와 레버를 정한다.

	// 참자가들에게 게임이 시작되었다는 패킷을 전송한다.
	for (UINT participant : participants) {
		SC_GAME_START sendPacket;
		sendPacket.professorClientID = professorID;
		send(ServerFramework::Instance().GetClient(participant)->GetSocket(), (char*)&sendPacket, sizeof(SC_GAME_START), 0);
	}

}

void PlayInfo::LoadingComplete(UINT _clientID) {
	if (loadingCompletes.contains(_clientID) && !allPlayerLoadingComplete) {
		loadingCompletes[_clientID] = true;

		// 만약 모든 플레이어가 로딩이 완료되었는지 확인한다.
		bool success = true;
		for (auto [clientID, loadingComplete] : loadingCompletes) {
			if (!loadingComplete)
				success = false;
		}

		// 모든 플레이어가 로딩이 완료되었다면
		if (success) {
			ProcessLoadingComplete();
		}
	}
}

void PlayInfo::ProcessLoadingComplete() {
	allPlayerLoadingComplete = true;

	// 모든 플레이어가 로딩이 완료되었다고 패킷을 보낸다.
	SC_All_PLAYER_LOADING_COMPLETE sendPacket;
	for (UINT participant : participants)
		send(ServerFramework::Instance().GetClient(participant)->GetSocket(), (char*)&sendPacket, sizeof(SC_All_PLAYER_LOADING_COMPLETE), 0);
}

