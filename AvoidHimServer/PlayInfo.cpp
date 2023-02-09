#include "stdafx.h"
#include "PlayInfo.h"
#include "ServerFramework.h"

#include "Client.h"

PlayInfo::PlayInfo(UINT _playInfoID) : playInfoID{ _playInfoID } {
	allPlayerLoadingComplete = false;
	professorObjectID = 0;
	objectIDCount = objectIDStart;
}
PlayInfo::~PlayInfo() {
	ServerFramework& serverFramework = ServerFramework::Instance();
	for (auto [participant, pClient] : participants) {
		pClient->SetCurrentPlayInfo(NULL);

		// ������ �������� ���߿� ������ ��� ����(�ӽ÷� ������ �ִ�)�ϰ� �ִ� Ŭ���̾�Ʈ�� ������ �����Ѵ�.
		if (pClient->IsDisconnected()) {
			serverFramework.RemoveClient(participant);
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

	// �����ڵ��� clientID�� client* �� �����´�.
	for (UINT participant : pRoom->GetParticipants()) {
		participants.emplace_back(participant, serverFramework.GetClient(participant));
	}

	const vector<XMFLOAT3>& shuffledStudentStartPos = serverFramework.GetShuffledStudentStartPositions();

	// �ʱ� ������Ʈ���� �����ؼ� �����´�.
	const auto& initObjects = serverFramework.GetinitialObjects();
	for (auto& [objectID, pObject] : initObjects) {
		GameObject* pNewObject = nullptr;// = new GameObject(*pObject);
		switch (pObject->GetType()) {
		case ObjectType::door: { 
			pNewObject = new Door(*static_cast<Door*>(pObject));
			pDoors.emplace(pNewObject->GetID(), static_cast<Door*>(pNewObject));
			break; 
		}
		case ObjectType::lever: {
			pNewObject = new Lever(*static_cast<Lever*>(pObject));
			pLevers.emplace(pNewObject->GetID(), static_cast<Lever*>(pNewObject));
			break;
		}
		case ObjectType::waterDispenser: {
			pNewObject = new WaterDispenser(*static_cast<WaterDispenser*>(pObject));
			pWaterDispensers.emplace(pNewObject->GetID(), static_cast<WaterDispenser*>(pNewObject));
			break;
		}
		default:
			break;
		}
	}

	// clientID�� ���� �÷��̾ ���Ѵ�.
	uniform_int_distribution<int> uid(0, (int)participants.size() - 1);
	int professorClientID = participants[uid(rd)].first;

	for (int studentStartPosIndex = 0; auto[participant, pClient] : participants) {
		// ��� �÷��̾��� �ε����¸� false�� �ʱ�ȭ �Ѵ�.
		loadingCompletes[participant] = false;

		// Ŭ���̾�Ʈ�� ���¸� ���ӽ������� �ٲ��ش�.
		pClient->SetClientState(ClientState::roomPlay);
		pClient->SetCurrentPlayInfo(this);

		// �÷��̾���� �����ϰ� ��ġ�� �ʱ�ȭ ���ش�.
		GameObject* pPlayer = new GameObject();
		pPlayer->SetID(objectIDCount++);
		if (participant == professorClientID) {	// ������ ���
			pPlayer->SetPosition(serverFramework.GetProfessorStartPosition());
			professorObjectID = pPlayer->GetID();
		}
		else {
			pPlayer->SetPosition(shuffledStudentStartPos[studentStartPosIndex++]);
		}

		pPlayers.emplace(pPlayer->GetID(), pPlayer);

		// �ڽ��� ������ ĳ������ ������Ʈ ���̵� �˷��ش�.
		SC_YOUR_PLAYER_OBJECTID sendPacket;
		sendPacket.objectID = pPlayer->GetID();
		SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
	}
	// Ȱ��ȭ�� ��ǻ�Ϳ� ������ ���Ѵ�.

	// �����ڵ鿡�� ������ ���۵Ǿ��ٴ� ��Ŷ�� �����Ѵ�.
	char* bufWriter = buffer.data();

	//	 SC_GAME_START�� ������ buf�� ä���.
	SC_GAME_START sendPacket;
	sendPacket.nPlayer = (UINT)participants.size();
	sendPacket.professorObjectID = professorObjectID;

	//	 �̾ SC_PLAYER_INFO �� ������� ä���.
	for (int index = 0; auto [objectID, pPlayer] : pPlayers) {
		SC_PLAYER_INFO& playerInfo = sendPacket.playerInfo[index];

		playerInfo.aniTime = 0.f;
		playerInfo.objectID = objectID;
		playerInfo.position = pPlayer->GetPosition();
		playerInfo.rotation = pPlayer->GetRotation();
		playerInfo.scale = pPlayer->GetScale();

		++index;
	}

	// ������ ���δ� ä�� buf�� �� �÷��̾�鿡�� �����Ѵ�.
	for (auto [participant, pClient] : participants) {
		if (pClient) {
			SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
		}
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
	ServerFramework& serverFramework = ServerFramework::Instance();
	allPlayerLoadingComplete = true;

	// loadingCompletes�� �����ϴ� �޸𸮸� �����ش�.
	loadingCompletes.clear();
	loadingCompletes.rehash(0);

	// ��� �÷��̾ �ε��� �Ϸ�Ǿ��ٰ� ��Ŷ�� ������.
	SC_All_PLAYER_LOADING_COMPLETE sendPacket;
	for (auto [participant, pClient] : participants) {
		if (pClient && !pClient->IsDisconnected())
			SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
	}
}

void PlayInfo::FrameAdvance() {
	if (!allPlayerLoadingComplete)
		return;

	ServerFramework& serverFramework = ServerFramework::Instance();

	// �÷��̾��� ��ġ�� �����ش�.
	SC_PLAYERS_INFO sendPacket;
	sendPacket.nPlayer = pPlayers.size();
	for (int index = 0; auto [objectID, pPlayer] : pPlayers) {
		SC_PLAYER_INFO& playerInfo = sendPacket.playersInfo[index];

		playerInfo.aniTime = 0.f;
		playerInfo.objectID = objectID;
		playerInfo.position = pPlayer->GetPosition();
		playerInfo.rotation = pPlayer->GetRotation();
		playerInfo.scale = pPlayer->GetScale();
		++index;
	}
	
	for (auto [participant, pClient] : participants) {
		if (pClient && !pClient->IsDisconnected())
			SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
	}

}

bool PlayInfo::ApplyCSPlayerInfo(CS_PLAYER_INFO& _packet) {
	GameObject* pPlayer = pPlayers[_packet.objectID];
	//[����] aniTime�� ������� �ش�.
	pPlayer->SetPosition(_packet.position);
	pPlayer->SetRotation(_packet.rotation);
	pPlayer->SetScale(_packet.scale);

	return false;
}

void PlayInfo::ApplyToggleDoor(UINT _objectID) {
	auto it = pDoors.find(_objectID);
	if (it != pDoors.end()) {
		Door* pDoor = it->second;
		pDoor->SetOpen(!pDoor->IsOpen());

		SC_TOGGLE_DOOR sendPacket;
		sendPacket.objectID = _objectID;

		for (auto [participant, pClient] : participants) {
			SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
		}
	}
	else {
		cout << "\b�ش��ϴ� ID�� ���� �����ϴ�.\n";
	}

}

