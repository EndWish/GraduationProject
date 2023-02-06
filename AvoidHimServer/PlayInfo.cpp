#include "stdafx.h"
#include "PlayInfo.h"
#include "ServerFramework.h"
#include "GameObject.h"

PlayInfo::PlayInfo(UINT _playInfoID) : playInfoID{ _playInfoID } {
	allPlayerLoadingComplete = false;
	professorObjectID = 0;
	objectIDCount = objectIDStart;

	timer = Timer();
}
PlayInfo::~PlayInfo() {
	ServerFramework& serverFramework = ServerFramework::Instance();
	for (auto clientID : participants) {
		Client* pClient = serverFramework.GetClient(clientID);
		pClient->SetCurrentPlayInfo(NULL);

		// 게임이 끝났으니 도중에 접속을 끊어서 보류(임시로 가지고 있던)하고 있던 클라이언트를 완전히 삭제한다.
		if (pClient->IsDisconnected()) {
			serverFramework.RemoveClient(clientID);
		}
	}

	for (auto [objectID, pPlayer] : pPlayers)
		delete pPlayer;
	pPlayers.clear();

	for (auto [objectID, pDoor] : pDoors)
		delete pDoor;
	pDoors.clear();

	for (auto [objectID, pLever] : pLevers)
		delete pLever;
	pLevers.clear();

	for (auto [objectID, pWaterDispenser] : pWaterDispensers)
		delete pWaterDispenser;
	pWaterDispensers.clear();

}

void PlayInfo::Init(UINT _roomID) {
	ServerFramework& serverFramework = ServerFramework::Instance();
    Room* pRoom = ServerFramework::Instance().GetRoom(_roomID);
	participants = pRoom->GetParticipants();	// 참가자들의 ID를 가져온다.

	const vector<XMFLOAT3>& shuffledStudentStartPos = serverFramework.GetShuffledStudentStartPositions();

	// 초기 오브젝트들을 복사해서 가져온다.
	const auto& initObjects = serverFramework.GetinitialObjects();
	for (auto& [objectID, pObject] : initObjects) {
		GameObject* pNewObject = new GameObject(*pObject);
		switch (pNewObject->GetType()) {
		case ObjectType::door: pDoors.emplace(pNewObject->GetID(), pNewObject); break;
		case ObjectType::lever: pLevers.emplace(pNewObject->GetID(), pNewObject); break;
		case ObjectType::waterDispenser: pWaterDispensers.emplace(pNewObject->GetID(), pNewObject); break;
		default:
			break;
		}
	}

	// clientID로 교수 플레이어를 정한다.
	uniform_int_distribution<int> uid(0, (int)participants.size() - 1);
	int professorClientID = participants[uid(rd)];

	for (int studentStartPosIndex = 0; UINT participant : participants) {
		// 모든 플레이어의 로딩상태를 false로 초기화 한다.
		loadingCompletes[participant] = false;

		// 클라이언트의 상태를 게임시작으로 바꿔준다.
		Client* client = serverFramework.GetClient(participant);
		client->SetClientState(ClientState::roomPlay);
		client->SetCurrentPlayInfo(this);

		// 플레이어들을 생성하고 위치를 초기화 해준다.
		GameObject* pPlayer = new GameObject();
		pPlayer->SetID(objectIDCount++);
		if (participant == professorClientID) {	// 교수일 경우
			pPlayer->SetPosition(serverFramework.GetProfessorStartPosition());
			professorObjectID = pPlayer->GetID();
		}
		else {
			pPlayer->SetPosition(shuffledStudentStartPos[studentStartPosIndex++]);
		}

		pPlayers.emplace(pPlayer->GetID(), pPlayer);

		// 자신이 조종할 캐릭터의 오브젝트 아이디를 알려준다.
		SC_YOUR_PLAYER_OBJECTID sendPacket;
		sendPacket.objectID = pPlayer->GetID();
		SendContents(client->GetSocket(), client->GetRemainBuffer(), sendPacket);
	}
	// 활성화될 컴퓨터와 레버를 정한다.

	// 참가자들에게 게임이 시작되었다는 패킷을 전송한다.
	char* bufWriter = buffer.data();

	//	 SC_GAME_START의 내용을 buf에 채운다.
	SC_GAME_START sendPacket;
	sendPacket.nPlayer = (UINT)participants.size();
	sendPacket.professorObjectID = professorObjectID;

	//	 이어서 SC_PLAYER_INFO 의 내용들을 채운다.
	for (int index = 0; auto [objectID, pPlayer] : pPlayers) {
		SC_PLAYER_INFO& playerInfo = sendPacket.playerInfo[index];

		playerInfo.aniTime = 0.f;
		playerInfo.objectID = objectID;
		playerInfo.position = pPlayer->GetPosition();
		playerInfo.rotation = pPlayer->GetRotation();
		playerInfo.scale = pPlayer->GetScale();

		++index;
	}

	// 내용을 전부다 채운 buf를 각 플레이어들에게 전송한다.
	for (UINT participant : participants) {
		Client* pClient = serverFramework.GetClient(participant);
		if (pClient) {
			SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
		}
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
	ServerFramework& serverFramework = ServerFramework::Instance();
	allPlayerLoadingComplete = true;

	// loadingCompletes가 차지하던 메모리를 없애준다.
	loadingCompletes.clear();
	loadingCompletes.rehash(0);

	// 모든 플레이어가 로딩이 완료되었다고 패킷을 보낸다.
	SC_All_PLAYER_LOADING_COMPLETE sendPacket;
	for (auto clientID : participants) {
		Client* pClient = serverFramework.GetClient(clientID);
		if (!pClient->IsDisconnected())
			SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
	}
}

