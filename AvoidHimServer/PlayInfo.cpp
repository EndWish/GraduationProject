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

	//	// 게임이 끝났으니 도중에 접속을 끊어서 보류(임시로 가지고 있던)하고 있던 클라이언트를 완전히 삭제한다.
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

	// 참가자들의 clientID와 client* 를 가져온다.
	for (UINT participant : pRoom->GetParticipants()) {
		participants.emplace_back(participant, serverFramework.GetClient(participant));
	}

	const vector<XMFLOAT3>& shuffledStudentStartPos = serverFramework.GetShuffledStudentStartPositions();
	vector<UINT> computersObjectID;

	// 초기 오브젝트들을 복사해서 가져온다.
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

	// clientID로 교수 플레이어를 정한다.
	uniform_int_distribution<int> uid(0, (int)participants.size() - 1);
	professorClientID = participants[uid(rd)].first;

	for (int studentStartPosIndex = 0; auto[participant, pClient] : participants) {
		// 모든 플레이어의 로딩상태를 false로 초기화 한다.
		loadingCompletes[participant] = false;

		// 클라이언트의 상태를 게임시작으로 바꿔준다.
		pClient->SetClientState(ClientState::roomWait);
		pClient->SetCurrentPlayInfo(this);

		// 플레이어들을 생성하고 위치를 초기화 해준다.
		Player* pPlayer = new Player();
		pPlayer->SetID(objectIDCount++);
		pPlayer->SetClientID(participant);
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
		SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
	}

	// 활성화될 컴퓨터와 레버를 정한다 => 플레이어 수(학생수 + 1) 만큼 활성화 시킨다.
	ranges::shuffle(computersObjectID, rd);
	for (UINT computerObjectID : computersObjectID | views::take(pPlayers.size())) {
		Computer* pNewObject = new Computer();
		pNewObject->SetType(ObjectType::computer);
		pNewObject->SetID(computerObjectID);
		pComputers.emplace(pNewObject->GetID(), pNewObject);
	}

	// 참가자들에게 게임이 시작되었다는 패킷을 전송한다.

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
	// activeComputerObjectID 의 내용을 채운다.
	for (int index = 0; auto [objectID, pObject] : pComputers) {
		sendPacket.activeComputerObjectID[index++] = objectID;
	}

	for (int i = 0; i < participants.size(); ++i) {
		memcpy(sendPacket.nickname[i], participants[i].second->GetNickname().data(), 20);
		wcout << sendPacket.nickname[i];
	}

	// 내용을 전부다 채운 buf를 각 플레이어들에게 전송한다.
	for (auto [participant, pClient] : participants) {
		if (pClient) {
			SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
		}
	}
	// 스폰 로케이션 갯수만큼 할당한다.
	isExistItem.assign(itemSpawnLocationCount, false);
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
	for (auto [participant, pClient] : participants) {
		if (pClient)
			SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
	}
}

void PlayInfo::FrameAdvance(float _timeElapsed) {
	if (!allPlayerLoadingComplete)
		return;

	ServerFramework& serverFramework = ServerFramework::Instance();

	// 플레이어의 위치를 보내준다.
	SC_PLAYERS_INFO sendPacket;
	sendPacket.nPlayer = (UINT)pPlayers.size();
	for (int index = 0; auto [objectID, pPlayer] : pPlayers) {
		// 탈출한 플레이어의 경우 정보를 담지 않는다.
		if (pPlayer->GetExit()) {
			--sendPacket.nPlayer;
			continue;
		}

		// 패킷을 보낸다.
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

	// 정수기의 쿨타임을 줄인다.
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
		//[수정] aniTime을 적용시켜 준다.
		pPlayer->SetPosition(recvPacket.position);
		pPlayer->SetRotation(recvPacket.rotation);
		pPlayer->SetScale(recvPacket.scale);

		break;
	}
	case CS_PACKET_TYPE::toggleDoor: {
		CS_TOGGLE_DOOR& recvPacket = GetPacket<CS_TOGGLE_DOOR>();
		cout << format("CS_TOGGLE_DOOR : cid - {}, objectID - {}, pid - {} \n", recvPacket.cid, recvPacket.objectID, recvPacket.pid);
		// 감옥 문에 대한 상호작용일 경우
		cout << prisonDoorObjectID << " , " << recvPacket.objectID << "\n";
		if (prisonDoorObjectID == recvPacket.objectID) {
			if (pPlayers[recvPacket.playerObjectID]->GetItem() != ObjectType::prisonKeyItem) break;
			// 해당 플레이어가 수감해제 되었다고 표시한다.
			for (auto [objectID, pPlayer] : pPlayers) {
				if (pPlayer->GetImprisoned()) {
					pPlayer->SetImprisoned(false);
					pPlayer->SetPosition(prisonExitPosition);
				}
			}

			// 감옥의 문을 열었다고 패킷을 보낸다.
			SC_OPEN_PRISON_DOOR sendPacket;
			sendPacket.openPlayerObjectID = recvPacket.playerObjectID;

			for (auto [participant, pClient] : participants) {
				SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
			}
			// 해당 플레이어가 가진 열쇠를 삭제한다.
			pPlayers[recvPacket.playerObjectID]->SetItem(ObjectType::none);
		}
		else {
			auto it = pDoors.find(recvPacket.objectID);
			if (it != pDoors.end()) {
				Door* pDoor = it->second;

				// 일반문이거나 탈출문이지만 해킹이 완료되었을 경우 상호작용한다.
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
				cout << "\b해당하는 ID의 문이 없습니다.\n";
			}
		}

		break;
	}
	case CS_PACKET_TYPE::useWaterDispenser: {
		CS_USE_WATER_DISPENSER& recvPacket = GetPacket<CS_USE_WATER_DISPENSER>();
		cout << format("CS_USE_WATER_DISPENSER : cid - {}, objectID - {}, pid - {} \n", recvPacket.cid, recvPacket.objectID, recvPacket.pid);

		WaterDispenser* pWaterDispenser = pWaterDispensers[recvPacket.objectID];

		if (pWaterDispenser->GetCoolTime() <= 0) {	// 정수기를 사용할 수 있을 경우
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
		
		// 해킹률을 적용한다.
		Computer* pComputer = pComputers[recvPacket.computerObjectID];
		pComputer->SetHackingRate(recvPacket.rate);
		pComputer->SetUse(false);

		// 다른 플레이어에게 해킹률을 알려준다.
		SC_HACKING_RATE sendPacket;
		sendPacket.computerObjectID = pComputer->GetID();
		sendPacket.rate = pComputer->GetHackingRate();
		for (auto [participant, pClient] : participants) {
			SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
		}

		// 모든 컴퓨터가 해킹이 완료 되었는지 확인한다.
		hackingComplete = true;
		for (auto [objectID, pComputer] : pComputers) {
			if (pComputer->GetHackingRate() < 100.f) {
				// 해킹이 덜됬을 경우
				hackingComplete = false;
				break;
			}
		}
		cout << "해킹완료? : " << hackingComplete << "\n";

		break;
	}
	case CS_PACKET_TYPE::attack: {
		CS_ATTACK& recvPacket = GetPacket<CS_ATTACK>();
		cout << format("CS_ATTACK : cid - {}, attackType - {}, pid - {} \n", recvPacket.cid, (int)recvPacket.attackType, recvPacket.pid);

		SC_ATTACK sendPacket;
		sendPacket.attackType = recvPacket.attackType;
		sendPacket.playerObjectID = recvPacket.playerObjectID;	// 시전자
		sendPacket.attackObjectID = objectIDCount++;	// 공격ID 부여
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
		
		// 해당 플레이어가 수감되었다고 표시한다.
		pPlayers[recvPacket.playerObjectID]->SetImprisoned(true);

		// 이동시킨다.
		pPlayers[recvPacket.playerObjectID]->SetPosition(prisonPosition);

		// 패킷을 전송한다.
		SC_GO_PRISON sendPacket;
		sendPacket.playerObjectID = recvPacket.playerObjectID;
		for (auto [participant, pClient] : participants) {
			if (recvPacket.cid == participant)
				continue;
			SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
		}

		// 모든 학생이 갇혔을 경우 교수의 승리로 게임을 끝낸다.
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
		// 다른 플레이어에게 해당 사실을 알리며 플레이어가 가진 아이템을 삭제한다.
		CS_USE_ITEM& recvPacket = GetPacket<CS_USE_ITEM>();
		pPlayers[recvPacket.playerObjectID]->SetItem(ObjectType::none);

		// 패킷을 전송한다.
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
		// 다른 플레이어에게 해당 사실을 알리며 플레이어가 가진 아이템을 갱신한다.
		CS_REMOVE_ITEM& recvPacket = GetPacket<CS_REMOVE_ITEM>();
		pPlayers[recvPacket.playerObjectID]->SetItem(recvPacket.itemType);
		// 해당 위치 아이템이 없다는것을 갱신
		cout << recvPacket.itemLocationIndex << " 위치 아이템 삭제\n";
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

		// 레버의 파워를 변경한다.
		pLevers[recvPacket.leverObjectID]->SetPower(recvPacket.setPower);

		// 모든 레버가 켜져있는지 확인한다.
		allLeverPowerOn = true;
		for (auto [objectID, pLever] : pLevers) {
			if (!pLever->GetPower()) {
				allLeverPowerOn = false;
				break;
			}
		}

		// 패킷을 전송한다.
		SC_LEVER_TOGGLE sendPacket;
		sendPacket.setPower = recvPacket.setPower;
		sendPacket.leverObjectID = recvPacket.leverObjectID;
		sendPacket.allLeverPowerOn = allLeverPowerOn;

		// 감옥의 문을 열었다고 패킷을 보낸다.
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

			// 패킷을 전송한다.
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
		// 해당 클라이언트를 게임에서 뺀다.
		auto it = ranges::find(participants, recvPacket.cid, &pair<UINT, Client*>::first);
		if (it != participants.end()) {
			it->second->SetCurrentPlayInfo(nullptr);
			participants.erase(it);
		}
		// 모든 클라이언트가 빠져나갔을 경우 playInfo를 삭제 시킨다.
		if (participants.empty()) {
			ServerFramework::Instance().GetRoom(playInfoID)->SetGameRunning(false);
			ServerFramework::Instance().RemovePlayInfo(playInfoID);
		}
		break;
	}
	default:
		READ_CID_IN_PACKET& readFrontPart = GetPacket<READ_CID_IN_PACKET>();
		cout << format("잘못된 패킷 번호 : {}, cid - {}\n", (int)readFrontPart.packetType, readFrontPart.cid);
		break;
	}
}

void PlayInfo::AddItem() {
	SC_ADD_ITEM packet;
	// 랜덤한 위치에 랜덤한 아이템을 추가한다.
	uniform_int_distribution<int> randomIndex(0, itemSpawnLocationCount-1);
	uniform_int_distribution<int> itemChance(1, 100);

	int index = randomIndex(rd);	// 아이템 스폰장소의 인덱스
	while (isExistItem[index]) {
		cout << "중복입니다. reroll..";
		index = randomIndex(rd);
	}
	int chance = itemChance(rd);	

	int keyChance = 5;
	int trapChance = 30;
	int energyDrinkChance = 40;	// 아이템의 확률

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
	cout << index << " 위치 아이템 추가\n";
}

void PlayInfo::EscapeClient(UINT _clientID) {
	// 탈주한 클라이언트를 participants에서 찾는다.
	auto iterEscapeClient = ranges::find(participants, _clientID, &pair<UINT, Client*>::first);
	auto [escapeClientID, pEscapeClient] = *iterEscapeClient;

	// 게임이 아직 끝나지 않았을 때
	if (!endGame) {
		// 교수가 탈주했을 경우 = 학생 승
		if (escapeClientID == professorClientID) {
			endGame = true;
			SC_ESCAPE_PROFESSOR sendPacket;
			for (auto [participant, pClient] : participants) {
				if (participant == escapeClientID)
					continue;
				SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
			}
		}

		// 학생이 탈주했을 경우 (1. 게임이 끝난다. 2. 그 플레이어만 삭제하고 게임을 계속 진행한다.)
		else {
			// 1. 게임이 끝나는 경우
			if (participants.size() == 2) {
				endGame = true;
				SC_PROFESSOR_WIN sendPacket;
				for (auto [participant, pClient] : participants) {
					if (participant == escapeClientID)
						continue;
					SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
				}
			}
			// 2. 게임을 계속 진행할 수 있는 경우
			else if (2 < participants.size()) {
				SC_ESCAPE_STUDENT sendPacket;
				// 탈주하는 클라이언트가 조종하는 플레이어의 objectID를 찾아낸다.
				for (auto [objectID, pPlayer] : pPlayers) {	
					if (pPlayer->GetClientID() == escapeClientID) {
						sendPacket.escapeObjectID = objectID;
						break;
					}
				}
				// 패킷을 보낸다.
				for (auto [participant, pClient] : participants) {
					if (participant == escapeClientID)
						continue;
					SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
				}
				// pPlayers에서 삭제한다.
				pPlayers.erase(sendPacket.escapeObjectID);
			}
		}
	}

	// 탈주한 클라이언트를 participants에서 제거한다.
	participants.erase(iterEscapeClient);

	// 모든 클라이언트가 빠져나갔을 경우 playInfo를 삭제 시킨다.
	if (participants.empty()) {
		ServerFramework::Instance().GetRoom(playInfoID)->SetGameRunning(false);
		ServerFramework::Instance().RemovePlayInfo(playInfoID);
	}

}
