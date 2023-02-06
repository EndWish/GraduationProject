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

		// ������ �������� ���߿� ������ ��� ����(�ӽ÷� ������ �ִ�)�ϰ� �ִ� Ŭ���̾�Ʈ�� ������ �����Ѵ�.
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
	participants = pRoom->GetParticipants();	// �����ڵ��� ID�� �����´�.

	const vector<XMFLOAT3>& shuffledStudentStartPos = serverFramework.GetShuffledStudentStartPositions();

	// �ʱ� ������Ʈ���� �����ؼ� �����´�.
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

	// clientID�� ���� �÷��̾ ���Ѵ�.
	uniform_int_distribution<int> uid(0, (int)participants.size() - 1);
	int professorClientID = participants[uid(rd)];

	for (int studentStartPosIndex = 0; UINT participant : participants) {
		// ��� �÷��̾��� �ε����¸� false�� �ʱ�ȭ �Ѵ�.
		loadingCompletes[participant] = false;

		// Ŭ���̾�Ʈ�� ���¸� ���ӽ������� �ٲ��ش�.
		Client* client = serverFramework.GetClient(participant);
		client->SetClientState(ClientState::roomPlay);
		client->SetCurrentPlayInfo(this);

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
		SendContents(client->GetSocket(), client->GetRemainBuffer(), sendPacket);
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
	for (auto clientID : participants) {
		Client* pClient = serverFramework.GetClient(clientID);
		if (!pClient->IsDisconnected())
			SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
	}
}

