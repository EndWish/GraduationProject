#include "stdafx.h"
#include "PlayInfo.h"
#include "ServerFramework.h"

#include "Client.h"

PlayInfo::PlayInfo(UINT _playInfoID) : playInfoID{ _playInfoID } {
	allPlayerLoadingComplete = false;
	professorObjectID = 0;
	objectIDCount = objectIDStart;

	hackingComplete = false;
	allLeverPowerOn = false;
	itemCount = 0;

	endGame = false;

	itemCreateCoolTime = 0.f;
	prisonDoorObjectID = 0;

	professorClientID = 0;
}
PlayInfo::~PlayInfo() {
	ServerFramework& serverFramework = ServerFramework::Instance();
	//for (auto [participant, pClient] : participants) {
	//	pClient->SetCurrentPlayInfo(NULL);

	//	// ������ �������� ���߿� ������ ��� ����(�ӽ÷� ������ �ִ�)�ϰ� �ִ� Ŭ���̾�Ʈ�� ������ �����Ѵ�.
	//	if (pClient->IsDisconnected()) {
	//		serverFramework.RemoveClient(participant);
	//	}
	//}

	
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

	for (auto [objectID, pComputer] : pComputers)
		delete pComputer;
	pComputers.clear();

}


void PlayInfo::Init(UINT _roomID) {
	ServerFramework& serverFramework = ServerFramework::Instance();
    Room* pRoom = ServerFramework::Instance().GetRoom(_roomID);

	// �����ڵ��� clientID�� client* �� �����´�.
	for (UINT participant : pRoom->GetParticipants()) {
		participants.emplace_back(participant, serverFramework.GetClient(participant));
	}

	const vector<XMFLOAT3>& shuffledStudentStartPos = serverFramework.GetShuffledStudentStartPositions();
	vector<UINT> computersObjectID;

	// �ʱ� ������Ʈ���� �����ؼ� �����´�.
	const auto& initObjects = serverFramework.GetinitialObjects();
	for (auto& [objectID, pObject] : initObjects) {
		GameObject* pNewObject = nullptr;// = new GameObject(*pObject);
		switch (pObject->GetType()) {
		case ObjectType::Rdoor:
		case ObjectType::Ldoor: 
		case ObjectType::exitLDoor: 
		case ObjectType::exitRDoor: {
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
		case ObjectType::computer: {
			computersObjectID.push_back(objectID);
			break;
		}
		case ObjectType::prisonDoor: {
			prisonDoorObjectID = objectID;
			break;
		}
		default:
			break;
		}
	}

	// clientID�� ���� �÷��̾ ���Ѵ�.
	uniform_int_distribution<int> uid(0, (int)participants.size() - 1);
	professorClientID = participants[uid(rd)].first;

	for (int studentStartPosIndex = 0; auto[participant, pClient] : participants) {
		// ��� �÷��̾��� �ε����¸� false�� �ʱ�ȭ �Ѵ�.
		loadingCompletes[participant] = false;

		// Ŭ���̾�Ʈ�� ���¸� ���ӽ������� �ٲ��ش�.
		pClient->SetClientState(ClientState::roomWait);
		pClient->SetCurrentPlayInfo(this);

		// �÷��̾���� �����ϰ� ��ġ�� �ʱ�ȭ ���ش�.
		Player* pPlayer = new Player();
		pPlayer->SetID(objectIDCount++);
		pPlayer->SetClientID(participant);
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

	// Ȱ��ȭ�� ��ǻ�Ϳ� ������ ���Ѵ� => �÷��̾� ��(�л��� + 1) ��ŭ Ȱ��ȭ ��Ų��.
	ranges::shuffle(computersObjectID, rd);
	for (UINT computerObjectID : computersObjectID | views::take(pPlayers.size())) {
		Computer* pNewObject = new Computer();
		pNewObject->SetType(ObjectType::computer);
		pNewObject->SetID(computerObjectID);
		pComputers.emplace(pNewObject->GetID(), pNewObject);
	}

	// �����ڵ鿡�� ������ ���۵Ǿ��ٴ� ��Ŷ�� �����Ѵ�.

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
	// activeComputerObjectID �� ������ ä���.
	for (int index = 0; auto [objectID, pObject] : pComputers) {
		sendPacket.activeComputerObjectID[index++] = objectID;
	}

	for (int i = 0; i < participants.size(); ++i) {
		memcpy(sendPacket.nickname[i], participants[i].second->GetNickname().data(), 20);
		wcout << sendPacket.nickname[i];
	}

	// ������ ���δ� ä�� buf�� �� �÷��̾�鿡�� �����Ѵ�.
	for (auto [participant, pClient] : participants) {
		if (pClient) {
			SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
		}
	}
	// ���� �����̼� ������ŭ �Ҵ��Ѵ�.
	isExistItem.assign(itemSpawnLocationCount, false);
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
		if (pClient)
			SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
	}
}

void PlayInfo::FrameAdvance(float _timeElapsed) {
	if (!allPlayerLoadingComplete)
		return;

	ServerFramework& serverFramework = ServerFramework::Instance();

	// �÷��̾��� ��ġ�� �����ش�.
	SC_PLAYERS_INFO sendPacket;
	sendPacket.nPlayer = (UINT)pPlayers.size();
	for (int index = 0; auto [objectID, pPlayer] : pPlayers) {
		// Ż���� �÷��̾��� ��� ������ ���� �ʴ´�.
		if (pPlayer->GetExit()) {
			--sendPacket.nPlayer;
			continue;
		}

		// ��Ŷ�� ������.
		SC_PLAYER_INFO& playerInfo = sendPacket.playersInfo[index];

		playerInfo.aniTime = 0.f;
		playerInfo.objectID = objectID;
		playerInfo.position = pPlayer->GetPosition();
		playerInfo.rotation = pPlayer->GetRotation();
		playerInfo.scale = pPlayer->GetScale();
		++index;
	}
	
	for (auto [participant, pClient] : participants) {
		if (pClient)
			SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
	}

	// �������� ��Ÿ���� ���δ�.
	for (auto [objectID, pObject] : pWaterDispensers) {
		pObject->SubtractCoolTime(_timeElapsed);
	}

	if (itemCount < 4) {
		itemCreateCoolTime -= _timeElapsed;
		if (itemCreateCoolTime < 0.f) {
			itemCreateCoolTime = CREATE_ITEM_PERIOD;
			AddItem();
		}
	}
}

void PlayInfo::ProcessRecv(CS_PACKET_TYPE _packetType) {
	switch (_packetType) {
	case CS_PACKET_TYPE::playerInfo: {
		CS_PLAYER_INFO& recvPacket = GetPacket<CS_PLAYER_INFO>();
		//cout << format("CS_PLAYER_INFO : cid - {}, objectID - {}, pid - {} \n", recvPacket.cid, recvPacket.objectID, recvPacket.pid);

		GameObject* pPlayer = pPlayers[recvPacket.objectID];
		//[����] aniTime�� ������� �ش�.
		pPlayer->SetPosition(recvPacket.position);
		pPlayer->SetRotation(recvPacket.rotation);
		pPlayer->SetScale(recvPacket.scale);

		break;
	}
	case CS_PACKET_TYPE::toggleDoor: {
		CS_TOGGLE_DOOR& recvPacket = GetPacket<CS_TOGGLE_DOOR>();
		cout << format("CS_TOGGLE_DOOR : cid - {}, objectID - {}, pid - {} \n", recvPacket.cid, recvPacket.objectID, recvPacket.pid);
		// ���� ���� ���� ��ȣ�ۿ��� ���
		cout << prisonDoorObjectID << " , " << recvPacket.objectID << "\n";
		if (prisonDoorObjectID == recvPacket.objectID) {
			if (pPlayers[recvPacket.playerObjectID]->GetItem() != ObjectType::prisonKeyItem) break;
			// �ش� �÷��̾ �������� �Ǿ��ٰ� ǥ���Ѵ�.
			for (auto [objectID, pPlayer] : pPlayers) {
				if (pPlayer->GetImprisoned()) {
					pPlayer->SetImprisoned(false);
					pPlayer->SetPosition(prisonExitPosition);
				}
			}

			// ������ ���� �����ٰ� ��Ŷ�� ������.
			SC_OPEN_PRISON_DOOR sendPacket;
			sendPacket.openPlayerObjectID = recvPacket.playerObjectID;

			for (auto [participant, pClient] : participants) {
				SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
			}
			// �ش� �÷��̾ ���� ���踦 �����Ѵ�.
			pPlayers[recvPacket.playerObjectID]->SetItem(ObjectType::none);
		}
		else {
			auto it = pDoors.find(recvPacket.objectID);
			if (it != pDoors.end()) {
				Door* pDoor = it->second;

				// �Ϲݹ��̰ų� Ż�⹮������ ��ŷ�� �Ϸ�Ǿ��� ��� ��ȣ�ۿ��Ѵ�.
				if (!pDoor->IsExitDoor() || (pDoor->IsExitDoor() && hackingComplete)) {
					pDoor->SetOpen(!pDoor->IsOpen());

					SC_TOGGLE_DOOR sendPacket;
					sendPacket.objectID = recvPacket.objectID;

					for (auto [participant, pClient] : participants) {
						SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
					}
				}
			}
			else {
				cout << "\b�ش��ϴ� ID�� ���� �����ϴ�.\n";
			}
		}

		break;
	}
	case CS_PACKET_TYPE::useWaterDispenser: {
		CS_USE_WATER_DISPENSER& recvPacket = GetPacket<CS_USE_WATER_DISPENSER>();
		cout << format("CS_USE_WATER_DISPENSER : cid - {}, objectID - {}, pid - {} \n", recvPacket.cid, recvPacket.objectID, recvPacket.pid);

		WaterDispenser* pWaterDispenser = pWaterDispensers[recvPacket.objectID];

		if (pWaterDispenser->GetCoolTime() <= 0) {	// �����⸦ ����� �� ���� ���
			pWaterDispenser->SetCoolTime(WATER_DISPENSER_COOLTIME);
			SC_USE_WATER_DISPENSER sendPacket;
			sendPacket.playerObjectID = recvPacket.playerObjectID;
			sendPacket.waterDispenserObjectID = recvPacket.objectID;

			for (auto [participant, pClient] : participants) {
				SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
			}
		}
		break;
	}
	case CS_PACKET_TYPE::queryUseComputer: {
		CS_QUERY_USE_COMPUTER& recvPacket = GetPacket<CS_QUERY_USE_COMPUTER>();
		cout << format("CS_QUERY_USE_COMPUTER : cid - {}, computerObjectID - {}, playerObjectID - {}, pid - {} \n", recvPacket.cid, recvPacket.computerObjectID, recvPacket.playerObjectID, recvPacket.pid);

		Computer* pComputer = pComputers[recvPacket.computerObjectID];
		if (allLeverPowerOn && !pComputer->GetUse() && pComputer->GetHackingRate() < 100.f) {
			pComputer->SetUse(true);
			SC_USE_COMPUTER sendPacket;
			sendPacket.computerObjectID = recvPacket.computerObjectID;
			sendPacket.playerObjectID = recvPacket.playerObjectID;
			for (auto [participant, pClient] : participants) {
				SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
			}
		}
		
		break;
	}
	case CS_PACKET_TYPE::hackingRate: {
		CS_HACKING_RATE& recvPacket = GetPacket<CS_HACKING_RATE>();
		cout << format("CS_HACKING_RATE : cid - {}, computerObjectID - {}, rate - {}, pid - {} \n", recvPacket.cid, recvPacket.computerObjectID, recvPacket.rate, recvPacket.pid);
		
		// ��ŷ���� �����Ѵ�.
		Computer* pComputer = pComputers[recvPacket.computerObjectID];
		pComputer->SetHackingRate(recvPacket.rate);
		pComputer->SetUse(false);

		// �ٸ� �÷��̾�� ��ŷ���� �˷��ش�.
		SC_HACKING_RATE sendPacket;
		sendPacket.computerObjectID = pComputer->GetID();
		sendPacket.rate = pComputer->GetHackingRate();
		for (auto [participant, pClient] : participants) {
			SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
		}

		// ��� ��ǻ�Ͱ� ��ŷ�� �Ϸ� �Ǿ����� Ȯ���Ѵ�.
		hackingComplete = true;
		for (auto [objectID, pComputer] : pComputers) {
			if (pComputer->GetHackingRate() < 100.f) {
				// ��ŷ�� ������ ���
				hackingComplete = false;
				break;
			}
		}
		cout << "��ŷ�Ϸ�? : " << hackingComplete << "\n";

		break;
	}
	case CS_PACKET_TYPE::attack: {
		CS_ATTACK& recvPacket = GetPacket<CS_ATTACK>();
		cout << format("CS_ATTACK : cid - {}, attackType - {}, pid - {} \n", recvPacket.cid, (int)recvPacket.attackType, recvPacket.pid);

		SC_ATTACK sendPacket;
		sendPacket.attackType = recvPacket.attackType;
		sendPacket.playerObjectID = recvPacket.playerObjectID;	// ������
		sendPacket.attackObjectID = objectIDCount++;	// ����ID �ο�
		for (auto [participant, pClient] : participants) {
			SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
		}
		break;
	}
	case CS_PACKET_TYPE::hit: {
		CS_ATTACK_HIT& recvPacket = GetPacket<CS_ATTACK_HIT>();
		cout << format("CS_ATTACK_HIT : cid - {}, attackType - {}, attackObjectID - {}, hitPlayerObjectID - {}, pid - {} \n", recvPacket.cid, (int)recvPacket.attackType, recvPacket.attackObjectID, recvPacket.hitPlayerObjectID, recvPacket.pid);

		SC_ATTACK_HIT sendPacket;
		sendPacket.attackObjectID = recvPacket.attackObjectID;
		sendPacket.hitPlayerObjectID = recvPacket.hitPlayerObjectID;
		sendPacket.attackType = recvPacket.attackType;

		for (auto [participant, pClient] : participants) {
			if (recvPacket.cid == participant)
				continue;
			SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
		}
		break;
	}
	case CS_PACKET_TYPE::goPrison: {
		CS_GO_PRISON& recvPacket = GetPacket<CS_GO_PRISON>();
		cout << format("CS_GO_PRISON : cid - {}, attackObjectID - {}, pid - {} \n", recvPacket.cid, (int)recvPacket.playerObjectID, recvPacket.pid);
		
		// �ش� �÷��̾ �����Ǿ��ٰ� ǥ���Ѵ�.
		pPlayers[recvPacket.playerObjectID]->SetImprisoned(true);

		// �̵���Ų��.
		pPlayers[recvPacket.playerObjectID]->SetPosition(prisonPosition);

		// ��Ŷ�� �����Ѵ�.
		SC_GO_PRISON sendPacket;
		sendPacket.playerObjectID = recvPacket.playerObjectID;
		for (auto [participant, pClient] : participants) {
			if (recvPacket.cid == participant)
				continue;
			SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
		}

		// ��� �л��� ������ ��� ������ �¸��� ������ ������.
		bool allStudentsInPrison = true;
		for (auto [objectID, pPlayer] : pPlayers) {
			if (objectID == professorObjectID)
				continue;
			if (!pPlayer->GetImprisoned()) {
				allStudentsInPrison = false;
				break;
			}
		}

		if (!endGame && allStudentsInPrison) {
			endGame = true;
			SC_PROFESSOR_WIN sendPacket;
			for (auto [participant, pClient] : participants) {
				SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
			}
		}

		break;
	}
	case CS_PACKET_TYPE::useItem: {
		// �ٸ� �÷��̾�� �ش� ����� �˸��� �÷��̾ ���� �������� �����Ѵ�.
		CS_USE_ITEM& recvPacket = GetPacket<CS_USE_ITEM>();
		pPlayers[recvPacket.playerObjectID]->SetItem(ObjectType::none);

		// ��Ŷ�� �����Ѵ�.
		SC_USE_ITEM sendPacket;
		if (recvPacket.itemType == ObjectType::trapItem) sendPacket.itemObjectID = objectIDCount++;
		sendPacket.objectType = recvPacket.itemType;
		sendPacket.playerObjectID = recvPacket.playerObjectID;
		for (auto [participant, pClient] : participants) {
			SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
		}
		break;
	}
	case CS_PACKET_TYPE::removeItem: {
		// �ٸ� �÷��̾�� �ش� ����� �˸��� �÷��̾ ���� �������� �����Ѵ�.
		CS_REMOVE_ITEM& recvPacket = GetPacket<CS_REMOVE_ITEM>();
		pPlayers[recvPacket.playerObjectID]->SetItem(recvPacket.itemType);
		// �ش� ��ġ �������� ���ٴ°��� ����
		cout << recvPacket.itemLocationIndex << " ��ġ ������ ����\n";
		isExistItem[recvPacket.itemLocationIndex] = false;
		--itemCount;
		SC_REMOVE_ITEM sendPacket;
		sendPacket.itemObjectID = recvPacket.itemObjectID;
		for (auto [participant, pClient] : participants) {
			if (recvPacket.cid == participant)
				continue;
			SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
		}
		break;
	}
	case CS_PACKET_TYPE::toggleLever: {
		CS_LEVER_TOGGLE& recvPacket = GetPacket<CS_LEVER_TOGGLE>();
		cout << format("SC_LEVER_TOGGLE : cid - {}, leverObjectID - {}, setPower - {}, pid - {} \n", recvPacket.cid, recvPacket.leverObjectID, recvPacket.setPower, recvPacket.pid);

		// ������ �Ŀ��� �����Ѵ�.
		pLevers[recvPacket.leverObjectID]->SetPower(recvPacket.setPower);

		// ��� ������ �����ִ��� Ȯ���Ѵ�.
		allLeverPowerOn = true;
		for (auto [objectID, pLever] : pLevers) {
			if (!pLever->GetPower()) {
				allLeverPowerOn = false;
				break;
			}
		}

		// ��Ŷ�� �����Ѵ�.
		SC_LEVER_TOGGLE sendPacket;
		sendPacket.setPower = recvPacket.setPower;
		sendPacket.leverObjectID = recvPacket.leverObjectID;
		sendPacket.allLeverPowerOn = allLeverPowerOn;

		// ������ ���� �����ٰ� ��Ŷ�� ������.
		for (auto [participant, pClient] : participants) {
			SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
		}
		break;
	}
	case CS_PACKET_TYPE::removeTrap: {
		CS_REMOVE_TRAP& recvPacket = GetPacket<CS_REMOVE_TRAP>();
		
		SC_REMOVE_TRAP sendPacket;
		sendPacket.trapObjectID = recvPacket.trapObjectID;
		for (auto [participant, pClient] : participants) {
			if (recvPacket.cid == participant)
				continue;
			SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
		}
		break;
	}
	case CS_PACKET_TYPE::exitPlayer: {
		CS_EXIT_PLAYER& recvPacket = GetPacket<CS_EXIT_PLAYER>();
		cout << format("CS_EXIT_PLAYER : cid - {}, playerObjectID - {}, pid - {} \n", recvPacket.cid, recvPacket.playerObjectID, recvPacket.pid);

		pPlayers[recvPacket.playerObjectID]->SetExit(true);
		if (!endGame) {
			endGame = true;

			// ��Ŷ�� �����Ѵ�.
			SC_EXIT_PLAYER sendPacket;
			sendPacket.playerObjectID = recvPacket.playerObjectID;
			for (auto [participant, pClient] : participants) {
				if (participant == recvPacket.cid)
					continue;
				SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
			}
		}
		break;
	}
	case CS_PACKET_TYPE::exitGame: {
		CS_EXIT_GAME& recvPacket = GetPacket<CS_EXIT_GAME>();
		cout << format("CS_EXIT_GAME : cid - {}, pid - {} \n", recvPacket.cid, recvPacket.pid);
		// �ش� Ŭ���̾�Ʈ�� ���ӿ��� ����.
		auto it = ranges::find(participants, recvPacket.cid, &pair<UINT, Client*>::first);
		if (it != participants.end()) {
			it->second->SetCurrentPlayInfo(nullptr);
			participants.erase(it);
		}
		// ��� Ŭ���̾�Ʈ�� ���������� ��� playInfo�� ���� ��Ų��.
		if (participants.empty()) {
			ServerFramework::Instance().GetRoom(playInfoID)->SetGameRunning(false);
			ServerFramework::Instance().RemovePlayInfo(playInfoID);
		}
		break;
	}
	default:
		READ_CID_IN_PACKET& readFrontPart = GetPacket<READ_CID_IN_PACKET>();
		cout << format("�߸��� ��Ŷ ��ȣ : {}, cid - {}\n", (int)readFrontPart.packetType, readFrontPart.cid);
		break;
	}
}

void PlayInfo::AddItem() {
	SC_ADD_ITEM packet;
	// ������ ��ġ�� ������ �������� �߰��Ѵ�.
	uniform_int_distribution<int> randomIndex(0, itemSpawnLocationCount-1);
	uniform_int_distribution<int> itemChance(1, 100);

	int index = randomIndex(rd);	// ������ ��������� �ε���
	while (isExistItem[index]) {
		cout << "�ߺ��Դϴ�. reroll..";
		index = randomIndex(rd);
	}
	int chance = itemChance(rd);	

	int keyChance = 5;
	int trapChance = 30;
	int energyDrinkChance = 40;	// �������� Ȯ��

	if (chance <= keyChance) {
		packet.objectType = ObjectType::prisonKeyItem;
	}
	else if (chance <= keyChance + trapChance) {
		packet.objectType = ObjectType::trapItem;
	}
	else if (chance <= keyChance + trapChance + energyDrinkChance) {
		packet.objectType = ObjectType::energyDrinkItem;
	}
	else {
		packet.objectType = ObjectType::medicalKitItem;
	}
	packet.itemLocationIndex = index;
	packet.itemObjectID = objectIDCount++;
	for (auto [participant, pClient] : participants) {
		SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), packet);
	}
	++itemCount;
	isExistItem[index] = true;
	cout << index << " ��ġ ������ �߰�\n";
}

void PlayInfo::EscapeClient(UINT _clientID) {
	// Ż���� Ŭ���̾�Ʈ�� participants���� ã�´�.
	auto iterEscapeClient = ranges::find(participants, _clientID, &pair<UINT, Client*>::first);
	auto [escapeClientID, pEscapeClient] = *iterEscapeClient;

	// ������ ���� ������ �ʾ��� ��
	if (!endGame) {
		// ������ Ż������ ��� = �л� ��
		if (escapeClientID == professorClientID) {
			endGame = true;
			SC_ESCAPE_PROFESSOR sendPacket;
			for (auto [participant, pClient] : participants) {
				if (participant == escapeClientID)
					continue;
				SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
			}
		}

		// �л��� Ż������ ��� (1. ������ ������. 2. �� �÷��̾ �����ϰ� ������ ��� �����Ѵ�.)
		else {
			// 1. ������ ������ ���
			if (participants.size() == 2) {
				endGame = true;
				SC_PROFESSOR_WIN sendPacket;
				for (auto [participant, pClient] : participants) {
					if (participant == escapeClientID)
						continue;
					SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
				}
			}
			// 2. ������ ��� ������ �� �ִ� ���
			else if (2 < participants.size()) {
				SC_ESCAPE_STUDENT sendPacket;
				// Ż���ϴ� Ŭ���̾�Ʈ�� �����ϴ� �÷��̾��� objectID�� ã�Ƴ���.
				for (auto [objectID, pPlayer] : pPlayers) {	
					if (pPlayer->GetClientID() == escapeClientID) {
						sendPacket.escapeObjectID = objectID;
						break;
					}
				}
				// ��Ŷ�� ������.
				for (auto [participant, pClient] : participants) {
					if (participant == escapeClientID)
						continue;
					SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
				}
				// pPlayers���� �����Ѵ�.
				pPlayers.erase(sendPacket.escapeObjectID);
			}
		}
	}

	// Ż���� Ŭ���̾�Ʈ�� participants���� �����Ѵ�.
	participants.erase(iterEscapeClient);

	// ��� Ŭ���̾�Ʈ�� ���������� ��� playInfo�� ���� ��Ų��.
	if (participants.empty()) {
		ServerFramework::Instance().GetRoom(playInfoID)->SetGameRunning(false);
		ServerFramework::Instance().RemovePlayInfo(playInfoID);
	}

}
