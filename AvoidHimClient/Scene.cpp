#include "stdafx.h"
#include "Scene.h"
#include "Timer.h"
#include "GameFramework.h"


Scene::Scene()	{
}

Scene::~Scene()	 {
}


void Scene::ProcessMouseInput(UINT _type, XMFLOAT2 _pos)
{
	switch (_type) {
	case WM_LBUTTONDOWN:

		for (auto [name, pButton] : pButtons) {
			if (pButton->CheckEnable() && pButton->CheckClick(_pos)) { // 버튼이 클릭되었을 경우
				pButton->Press(true, _pos);

				break;
			}
		}
		break;
	case WM_LBUTTONUP:
		for (auto [name, pButton] : pButtons) {
			if (pButton->CheckEnable()) {
				// 떼지면서 버튼이 눌린것인지 확인. 아니라면 pressed를 false로 바꿔줌
				if (pButton->Press(false, _pos)) {
					ReActButton(pButton);	// 플레이어가 버튼을 눌렀다면 버튼에 대한 동작 수행
				}
			}
		}
		break;
	}
}

void Scene::ProcessCursorMove(XMFLOAT2 _delta) {
}

char Scene::CheckCollision() {
	return 0;
}

void Scene::PostRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList)
{
}

void Scene::NoticeCloseToServer() {
}

//////////////////////////

LobbyScene::LobbyScene()
{
	roomPage = 1;
	currState = LobbyState::title;
	viewPort = { 0,0, C_WIDTH, C_HEIGHT, 0, 1 };
	scissorRect = { 0,0, C_WIDTH, C_HEIGHT };
}

LobbyScene::~LobbyScene()
{
}
void LobbyScene::Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();

	gameFramework.GetTextureManager().GetTexture("2DUI_readyButton", _pDevice,_pCommandList);
	gameFramework.GetTextureManager().GetTexture("2DUI_readyCancelButton", _pDevice, _pCommandList);
	gameFramework.GetTextureManager().GetTexture("2DUI_ready", _pDevice, _pCommandList);
	gameFramework.GetTextureManager().GetTexture("2DUI_host", _pDevice, _pCommandList);
	gameFramework.GetTextureManager().GetTexture("2DUI_roomInfo", _pDevice, _pCommandList);
	
	string name = "2DUI_title";
	pBackGround = make_shared<Image2D>(name, XMFLOAT2(2.f, 2.f), XMFLOAT2(0.f,0.f), XMFLOAT2(1.f,1.f), _pDevice, _pCommandList);


	shared_ptr<Image2D> pImg;
	name = "2DUI_ready";
	pImg = make_shared<Image2D>(name, XMFLOAT2(0.285f, 0.142f), XMFLOAT2(0.08f, 1.58f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
	pUIs["2DUI_ready_1"] = pImg;
	pImg = make_shared<Image2D>(name, XMFLOAT2(0.285f, 0.142f), XMFLOAT2(0.47f, 1.58f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
	pUIs["2DUI_ready_2"] = pImg;
	pImg = make_shared<Image2D>(name, XMFLOAT2(0.285f, 0.142f), XMFLOAT2(0.856f, 1.58f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
	pUIs["2DUI_ready_3"] = pImg;
	pImg = make_shared<Image2D>(name, XMFLOAT2(0.285f, 0.142f), XMFLOAT2(1.244f, 1.58f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
	pUIs["2DUI_ready_4"] = pImg;
	pImg = make_shared<Image2D>(name, XMFLOAT2(0.285f, 0.142f), XMFLOAT2(1.632f, 1.58f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
	pUIs["2DUI_ready_5"] = pImg;

	name = "2DUI_startButton";
	shared_ptr<Button> pButton = make_shared<Button>(name, XMFLOAT2(0.3f, 0.2f), XMFLOAT2(1.1f, 1.2f), ButtonType::start, _pDevice, _pCommandList);
	pButtons["startButton"] = pButton;

	name = "2DUI_optionButton";
	pButton = make_shared<Button>(name, XMFLOAT2(0.3f, 0.2f), XMFLOAT2(1.1f, 1.4f), ButtonType::option, _pDevice, _pCommandList);
	pButtons["optionButton"] = pButton;

	name = "2DUI_exitButton";
	pButton = make_shared<Button>(name, XMFLOAT2(0.3f, 0.2f), XMFLOAT2(1.1f, 1.6f), ButtonType::exit, _pDevice, _pCommandList);
	pButtons["exitButton"] = pButton;

	/////////////////////////////////


	name = "2DUI_title";
	pButton = make_shared<RoomButton>(name, XMFLOAT2(0.4f, 0.2f), XMFLOAT2(0.5f, 0.4f), ButtonType::room, _pDevice, _pCommandList, false);
	pButtons["RoomButton_1"] = pButton;
	pButton = make_shared<RoomButton>(name, XMFLOAT2(0.4f, 0.2f), XMFLOAT2(0.5f, 0.7f), ButtonType::room, _pDevice, _pCommandList, false);
	pButtons["RoomButton_2"] = pButton;
	pButton = make_shared<RoomButton>(name, XMFLOAT2(0.4f, 0.2f), XMFLOAT2(0.5f, 1.0f), ButtonType::room, _pDevice, _pCommandList, false);
	pButtons["RoomButton_3"] = pButton;
	pButton = make_shared<RoomButton>(name, XMFLOAT2(0.4f, 0.2f), XMFLOAT2(1.1f, 0.4f), ButtonType::room, _pDevice, _pCommandList, false);
	pButtons["RoomButton_4"] = pButton;
	pButton = make_shared<RoomButton>(name, XMFLOAT2(0.4f, 0.2f), XMFLOAT2(1.1f, 0.7f), ButtonType::room, _pDevice, _pCommandList, false);
	pButtons["RoomButton_5"] = pButton;
	pButton = make_shared<RoomButton>(name, XMFLOAT2(0.4f, 0.2f), XMFLOAT2(1.1f, 1.0f), ButtonType::room, _pDevice, _pCommandList, false);
	pButtons["RoomButton_6"] = pButton;

	name = "2DUI_refreshRoomButton";
	pButton = make_shared<Button>(name, XMFLOAT2(0.2f, 0.2f), XMFLOAT2(1.7f, 0.1f), ButtonType::refreshRoomList, _pDevice, _pCommandList, false);
	pButtons["refreshRoomButton"] = pButton;

	name = "2DUI_leftArrowButton";
	pButton = make_shared<Button>(name, XMFLOAT2(0.2f, 0.1f), XMFLOAT2(0.7f, 1.3f), ButtonType::prevRoomPage, _pDevice, _pCommandList, false);
	pButtons["prevRoomButton"] = pButton;

	name = "2DUI_rightArrowButton";
	pButton = make_shared<Button>(name, XMFLOAT2(0.2f, 0.1f), XMFLOAT2(1.1f, 1.3f), ButtonType::nextRoomPage, _pDevice, _pCommandList, false);
	pButtons["nextRoomButton"] = pButton;

	name = "2DUI_titleButton";
	pButton = make_shared<Button>(name, XMFLOAT2(0.3f, 0.2f), XMFLOAT2(1.6f, 1.7f), ButtonType::title, _pDevice, _pCommandList, false);
	pButtons["titleButton"] = pButton;

	name = "2DUI_makeRoom";
	pButton = make_shared<Button>(name, XMFLOAT2(0.3f, 0.2f), XMFLOAT2(1.25f, 1.7f), ButtonType::makeRoom, _pDevice, _pCommandList, false);
	pButtons["makeRoom"] = pButton;

	shared_ptr<TextBox> pText = make_shared<TextBox>((WCHAR*)L"휴먼돋움체", D2D1::ColorF(1, 1, 1, 1), XMFLOAT2(0.9f, 1.3f), XMFLOAT2(0.2f, 0.2f), C_WIDTH / 40.0f, false);
	pTexts["pageNum"] = pText;

	////////////////////////////////////


	name = "2DUI_startButton";
	pButton = make_shared<Button>(name, XMFLOAT2(0.3f, 0.2f), XMFLOAT2(1.25f, 1.7f), ButtonType::gameStart, _pDevice, _pCommandList, false);
	pButtons["gameStartButton"] = pButton;

	name = "2DUI_readyButton";
	pButton = make_shared<Button>(name, XMFLOAT2(0.3f, 0.2f), XMFLOAT2(1.25f, 1.7f), ButtonType::ready, _pDevice, _pCommandList, false);
	pButtons["readyButton"] = pButton;

	name = "2DUI_readyCancelButton";
	pButton = make_shared<Button>(name, XMFLOAT2(0.3f, 0.2f), XMFLOAT2(1.25f, 1.7f), ButtonType::readyCancel, _pDevice, _pCommandList, false);
	pButtons["readyCancelButton"] = pButton;

	name = "2DUI_quitRoomButton";
	pButton = make_shared<Button>(name, XMFLOAT2(0.3f, 0.2f), XMFLOAT2(1.6f, 1.7f), ButtonType::quitRoom, _pDevice, _pCommandList, false);
	pButtons["quitRoomButton"] = pButton;


}

void LobbyScene::ReleaseUploadBuffers() {

}

void LobbyScene::ProcessKeyboardInput(const array<UCHAR, 256>& _keysBuffers, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

}

void LobbyScene::AnimateObjects(char _collideCheck, double _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList)  {

}

void LobbyScene::ProcessSocketMessage() {
	GameFramework& gameFramework = GameFramework::Instance();

	SC_PACKET_TYPE packetType;
	recv(server_sock, (char*)&packetType, sizeof(SC_PACKET_TYPE), 0);
	switch (packetType) {
	case SC_PACKET_TYPE::giveClientID: { // 처음 접속시 플레이어 cid를 부여받는 패킷
		SC_GIVE_CLIENT_ID packet;
		RecvContents(packet);
		cid = packet.clientID;
		break;
	}
	case SC_PACKET_TYPE::roomListInfo: {
		SC_ROOMLIST_INFO packet;
		RecvContents(packet);

		// Roomlist 내 nRoom의 수 만큼 SC_SUB_ROOMLIST_INFO 패킷을 추가로 한꺼번에 받는다.
		roomList.clear();
		roomList.resize(packet.nRoom);
		recv(server_sock, (char*)roomList.data(), sizeof(SC_SUB_ROOMLIST_INFO) * packet.nRoom, 0);
		UpdateRoomText();
		break;
	}	// 현재 방의 리스트를 받는 패킷
	case SC_PACKET_TYPE::roomPlayersInfo: { // 입장 질의를 보내고 난 후 입장이 가능하다고 받음
		SC_ROOM_PLAYERS_INFO packet;
		RecvContents(packet);
		roomInfo.id = packet.roomID;
		roomInfo.players.clear();
		roomInfo.host = packet.hostID;
		roomInfo.nParticipant = packet.nParticipant;

		for (UINT i = 0; i < roomInfo.nParticipant; ++i) {
			Player_Info pi{ packet.participantInfos[i].clientID, packet.participantInfos[i].ready };
			roomInfo.players.push_back(pi);
		}

		changeUI(LobbyState::roomList, false);
		changeUI(LobbyState::inRoom, true);
		UpdateReadyState();
		break;
	} 
	case SC_PACKET_TYPE::fail: { 	// 방이 시작했거나, 꽉차거나 삭제되어 방 입장 실패한 경우
		// 실패
		SC_FAIL packet;
		RecvContents(packet);
		
		break;
	}
	case SC_PACKET_TYPE::ready: {	// 누군가 준비를 눌렀을 경우 
		SC_READY packet;
		RecvContents(packet);

		// 해당 cid를 가진 플레이어를 찾아 레디상태를 반대로 바꾸어준다.
		auto pindex = roomInfo.findPlayerIndex(packet.readyClientID);
		pindex->ready = !pindex->ready;
		UpdateReadyState();
		break;
	}
	case SC_PACKET_TYPE::roomVisitPlayerInfo: {	// 누가 방에 들어온 경우
		SC_ROOM_VISIT_PLAYER_INFO packet;
		RecvContents(packet);

		// 방 정보에 해당 플레이어의 정보를 추가한다.
		Player_Info pi{ packet.visitClientID, false };
		roomInfo.players.push_back(pi);
		roomInfo.nParticipant++;
		UpdateReadyState();
		break;
	}
	case SC_PACKET_TYPE::roomOutPlayerInfo: { // 누가 방에서 나간 경우
		SC_ROOM_OUT_PLAYER_INFO packet;
		RecvContents(packet);

		// 해당 클라이언트를 찾아 지우고 방장이 바뀌었다면 새로 임명한다.
		auto pindex = roomInfo.findPlayerIndex(packet.outClientID);
		roomInfo.players.erase(pindex);
		roomInfo.nParticipant--;
		roomInfo.host = packet.newHostID;
		UpdateReadyState();
		break;
	}
	case SC_PACKET_TYPE::gameStart: {
		SC_GAME_START recvPacket;
		RecvContents(recvPacket);

		loadingScene = make_shared<PlayScene>();

		// 게임이 시작된 경우 먼저 게임에서 사용될 인스턴스 정보, 메쉬, 애니메이션, 텍스처 등의 정보를 로드한다.
		gameFramework.LoadingScene(loadingScene);

		// 플레이 씬 로딩이 모두 완료된 경우 서버에게 로딩 완료 패킷을 보낸다. 
		CS_LOADING_COMPLETE sendPacket;
		sendPacket.roomID = roomInfo.id;

		sendPacket.cid = cid;
		send(server_sock, (char*)&sendPacket, sizeof(CS_LOADING_COMPLETE), 0);
		
		break;
	}
	case SC_PACKET_TYPE::allPlayerLoadingComplete: {

		SetCapture(hWnd);
		gameFramework.InitOldCursor();
		gameFramework.PushScene(loadingScene);

		break;

	}
	default:

		cout << "나머지 패킷. 타입 = " << (int)packetType << "\n";
	}
}

void LobbyScene::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed) {
	GameFramework& gameFramework = GameFramework::Instance();

	_pCommandList->RSSetViewports(1, &viewPort);
	_pCommandList->RSSetScissorRects(1, &scissorRect);

	// 텍스처가 들어있는 디스크립터 힙을 연결한다.
	Shader::SetDescriptorHeap(_pCommandList);

	gameFramework.GetShader("UIShader")->PrepareRender(_pCommandList);
	pBackGround->Render(_pCommandList);

	for (auto [name, pUI] : pUIs) {
		pUI->Render(_pCommandList);
	}
	for (auto [name, pButton] : pButtons) {
		pButton->Render(_pCommandList);
	}
}

void LobbyScene::PostRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	for (auto [name, pButton] : pButtons) {
		pButton->PostRender();
	}
	// 글자는 Render과정이 전부 끝난 후에 가능
	for (auto [name, pText] : pTexts) {
		pText->Render();
	}
}

void LobbyScene::ReActButton(shared_ptr<Button> _pButton) { // 시작 버튼을 누른 경우
	ButtonType type = _pButton->GetType();
	switch (type) {
	case ButtonType::start: {
		// 서버에게 CS_QUERY_ROOMLIST_INFO 패킷을 보내 현재 방 리스트를 보내달라고 요청한다.
		CS_QUERY_ROOMLIST_INFO sPacket;
		sPacket.cid = cid;
		send(server_sock, (char*)&sPacket, sizeof(CS_QUERY_ROOMLIST_INFO), 0);

		changeUI(LobbyState::title, false);
		changeUI(LobbyState::roomList, true);
		break;
	}
	case ButtonType::makeRoom: {

		// 패킷을 보낸 후, 방 생성은 항상 성공하므로 서버에게 따로 패킷을 받지 않고
		// 본인이 만든 방으로 입장한다.
		CS_MAKE_ROOM sPacket;
		sPacket.hostID = cid;
		send(server_sock, (char*)&sPacket, sizeof(CS_MAKE_ROOM), 0);

		// 방 정보를 비운 후 본인을 방장으로 방 하나를 만든다.
		roomInfo.ClearRoom();
		Player_Info pi{ cid, true };
		roomInfo.players.push_back(pi);
		roomInfo.nParticipant++;
		roomInfo.host = cid;

		changeUI(LobbyState::roomList, false);
		changeUI(LobbyState::inRoom, true);
		UpdateReadyState();
		break;
	}
	case ButtonType::quitRoom: {
		// 나간 후 방에서 나갔다는 사실을 알려주는 패킷을 전송.
	// 서버에서 알아서 이 패킷을 받아 룸 리스트를 다시 보내준다.
		CS_OUT_ROOM sPacket;
		sPacket.cid = cid;
		send(server_sock, (char*)&sPacket, sizeof(CS_QUERY_ROOMLIST_INFO), 0);

		changeUI(LobbyState::inRoom, false);
		changeUI(LobbyState::roomList, true);
		break;
	}
	case ButtonType::title: {
		changeUI(LobbyState::roomList, false);
		changeUI(LobbyState::title, true);
		break;
	}
	case ButtonType::room: {
		// 누른 방 번호를 서버로 보내 입장 가능한지 물어본다.
		CS_QUERY_VISIT_ROOM sPacket;
		sPacket.cid = cid;
		sPacket.visitRoomID = reinterpret_pointer_cast<RoomButton>(_pButton)->GetRoomIndex();
		send(server_sock, (char*)&sPacket, sizeof(CS_QUERY_VISIT_ROOM), 0);
		break;
	}
	case ButtonType::gameStart: {
		CS_READY sPacket;
		sPacket.cid = cid;
		send(server_sock, (char*)&sPacket, sizeof(CS_READY), 0);
		break;
	}
	case ButtonType::prevRoomPage: {
			if (roomPage > 1) roomPage--;
			UpdateRoomText();
			break;
	}
	case ButtonType::nextRoomPage: {
			roomPage++;
			UpdateRoomText();
			break;
	}
	case ButtonType::refreshRoomList: {
		CS_QUERY_ROOMLIST_INFO sPacket;
		sPacket.cid = cid;
		send(server_sock, (char*)&sPacket, sizeof(CS_QUERY_ROOMLIST_INFO), 0);
		break;
	}
	}
}

void LobbyScene::NoticeCloseToServer() {
	if (currState == LobbyState::inRoom) {
		// 방 안에 있을때 클라이언트 종료 시 방 나가기 버튼을 눌렀을때와 같은 동작 수행
		CS_OUT_ROOM sPacket;
		sPacket.cid = cid;
		send(server_sock, (char*)&sPacket, sizeof(CS_QUERY_ROOMLIST_INFO), 0);
	}
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          
void LobbyScene::changeUI(LobbyState _state, bool _active) {
	// 로비씬의 각 세부 상태에 따라 버튼, 배경 이미지를 변경한다.  
	if (_state == LobbyState::title) {
		pButtons["startButton"]->SetEnable(_active);
		pButtons["optionButton"]->SetEnable(_active);
		pButtons["exitButton"]->SetEnable(_active);
		if (_active) {
			pBackGround->SetTexture("2DUI_title");
			pBackGround->SetEnable(true);
		}
	}
	if (_state == LobbyState::roomList) {
		pButtons["RoomButton_1"]->SetEnable(_active);
		pButtons["RoomButton_2"]->SetEnable(_active);
		pButtons["RoomButton_3"]->SetEnable(_active);
		pButtons["RoomButton_4"]->SetEnable(_active);
		pButtons["RoomButton_5"]->SetEnable(_active);
		pButtons["RoomButton_6"]->SetEnable(_active);
		pButtons["makeRoom"]->SetEnable(_active);
		pButtons["titleButton"]->SetEnable(_active);
		pButtons["prevRoomButton"]->SetEnable(_active);
		pButtons["nextRoomButton"]->SetEnable(_active);
		pButtons["refreshRoomButton"]->SetEnable(_active);
		pTexts["pageNum"]->SetEnable(_active); 
		
		if (_active) {
			pBackGround->SetEnable(false);
			roomPage = 1;
		}
	}
	if (_state == LobbyState::inRoom) {
		pButtons["gameStartButton"]->SetEnable(_active);
		pButtons["quitRoomButton"]->SetEnable(_active);
		pUIs["2DUI_ready_1"]->SetEnable(_active);
		pUIs["2DUI_ready_2"]->SetEnable(_active);
		pUIs["2DUI_ready_3"]->SetEnable(_active);
		pUIs["2DUI_ready_4"]->SetEnable(_active);
		pUIs["2DUI_ready_5"]->SetEnable(_active);

		if (_active) {
			pBackGround->SetTexture("2DUI_roomInfo");
			pBackGround->SetEnable(true);
		}
	}
	if (_active) currState = _state;
}

void LobbyScene::UpdateRoomText() {
	pTexts["pageNum"]->SetText(to_wstring(roomPage));
	// 현재 페이지 기준 방 리스트의 정보로 갱신 해준다.
	bool lastRoom = false;
	UINT startIndex = 1 + (roomPage - 1) * 6;
	for (UINT i = startIndex; i < startIndex + 6; ++i) {

		UINT participant = 0;
		RoomState state = RoomState::none;
		string baseName = "RoomButton_" + to_string(i - startIndex + 1);

		// 방 정보가 없는 칸일 경우 
		if (roomList.size() < i) {
			state = RoomState::none;
			lastRoom = true; // 이후에 나오는 룸들을 모두 빈 방으로 표시한다.
		}
		UINT roomID = 0;
		if (!lastRoom) {
			SC_SUB_ROOMLIST_INFO packet = roomList[i - 1];
			roomID = packet.roomID;

			participant = packet.nParticipant;
			if (packet.started) { // 이미 시작한 경우
				state = RoomState::started;
			}
			else if (packet.nParticipant == 5) {	// 정원이 꽉 찬 경우
				state = RoomState::full;
			}
			else state = RoomState::joinable;
		}
		reinterpret_cast<RoomButton*>(pButtons[baseName].get())->UpdateState(roomID, participant ,state);
	}
}

void LobbyScene::UpdateReadyState() {

	// 현재 방에 ui, 버튼 상태등을 갱신
	bool bChange = false;
	for (int i = 0; i < 5; ++i) {
		pUIs["2DUI_ready_" + to_string(i + 1)]->SetEnable(false);
	}
	for (UINT i = 0; i < roomInfo.nParticipant; ++i) {
		// 방장일 경우
		if (roomInfo.host == roomInfo.players[i].clientID) {
			pUIs["2DUI_ready_" + to_string(i + 1)]->SetTexture("2DUI_host");
			pUIs["2DUI_ready_" + to_string(i + 1)]->SetEnable(true);
			// 본인이 방장일 경우
			if (cid == roomInfo.players[i].clientID) {
				pButtons["gameStartButton"]->SetTexture("2DUI_startButton");
				bChange = true;
			}
		}
		// 준비상태일 경우 
		else if (roomInfo.players[i].ready) {
			pUIs["2DUI_ready_" + to_string(i + 1)]->SetTexture("2DUI_ready");
			pUIs["2DUI_ready_" + to_string(i + 1)]->SetEnable(true);
			// 본인이 준비상태일 경우
			if (cid == roomInfo.players[i].clientID) {
				pButtons["gameStartButton"]->SetTexture("2DUI_readyCancelButton");
				bChange = true;
			}
		}

	}
	// 준비상태가 아닐경우
	if (!bChange) {
		pButtons["gameStartButton"]->SetTexture("2DUI_readyButton");
	}
}

/////////////////////////

PlayScene::PlayScene() {
	globalAmbient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
}

PlayScene::~PlayScene() {
	pLightsBuffer->Unmap(0, NULL);
}

void PlayScene::SetPlayer(shared_ptr<Player>& _pPlayer) {
	pPlayer = _pPlayer;
}

char PlayScene::CheckCollision() {
	char result = 0;
	XMFLOAT3 velocity = pPlayer->GetVelocity();

	shared_ptr<GameObject> collideObj = pZone->CheckCollisionVertical();

	// 플레이어의 OOBB를 y방향으로 이동시켜 본 후 충돌체크를 진행한다.
	if (!collideObj) {
		result += 1;
	}
	else {
		// 바닥이 없다면 새로 지정한다.
		if(!pPlayer->GetFloor()) pPlayer->SetFloor(collideObj);
	}


	// 플레이어의 OOBB를 x,z방향으로 이동시켜 본 후 충돌체크를 진행한다.
	collideObj = pZone->CheckCollisionHorizontal(pPlayer->GetFloor());
	if (collideObj == 0) {
		result += 2;
	}


	/*if (Vector4::IsSame(rot, Vector4::QuaternionIdentity())) {
		result += 4;
	} 
	else */
	{

		// 플레이어의 OOBB를 회전시켜본 후 충돌체크를 진행한다.
		vector<shared_ptr<GameObject>> collideObjs = pZone->CheckCollisionRotate(pPlayer->GetFloor());
		if (collideObjs.size() == 0) {
			result += 4;
		}
		else {
			// 부딪힐 경우 
			
			XMFLOAT3 knockBack = XMFLOAT3();
			XMFLOAT3 lookVector, direcVector;

			for (auto& collideObj : collideObjs) {
				// 물체의 룩벡터와 두 OOBB의 방향의 각을 비교해 룩벡터가 반대쪽에 있을경우 -1을 곱해준다.
				
				lookVector = collideObj->GetWorldLookVector();
				direcVector = Vector3::Subtract(pPlayer->GetBoundingBox().Center, collideObj->GetBoundingBox().Center);
				direcVector.y = 0;
				if (Vector3::Angle(direcVector, lookVector) > 90.0f) {
					lookVector = Vector3::ScalarProduct(lookVector, -1.f);
				}
				knockBack = Vector3::Add(knockBack, Vector3::ScalarProduct(Vector3::Normalize(lookVector), 0.01f));
			}
			// 부딪힌 오브젝트들의 룩벡터 방향들을 모아 그 방향으로 밀어준다.
			pPlayer->SetKnockBack(knockBack);
		}
	}
	
	return result;
}


void PlayScene::Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();

	// Zone을 생성 후 맵파일을 읽어 오브젝트들을 로드한다.
	pZone = make_shared<Zone>(XMFLOAT3(100.f, 100.f, 100.f), XMINT3(10, 10, 10), shared_from_this());
	pZone->LoadZoneFromFile(_pDevice, _pCommandList);

	// 빛을 추가
	shared_ptr<Light> baseLight = make_shared<Light>();


	baseLight->lightType = 3;
	baseLight->position = XMFLOAT3(0, 500, 0);
	baseLight->direction = XMFLOAT3(0, -1, 0);
	baseLight->diffuse = XMFLOAT4(1, 1, 1, 1);
	baseLight->specular = XMFLOAT4(0.01f, 0.01f, 0.01f, 1.0f);
	AddLight(baseLight);

	pPlayer->UpdateObject();
	camera = pPlayer->GetCamera();

	pFrustumMesh = make_shared<FrustumMesh>();
	pFrustumMesh->Create(camera->GetBoundingFrustum(), _pDevice, _pCommandList);

	ComPtr<ID3D12Resource> temp;
	UINT ncbElementBytes = ((sizeof(LightsMappedFormat) + 255) & ~255); //256의 배수
	pLightsBuffer = ::CreateBufferResource(_pDevice, _pCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, temp);
	pLightsBuffer->Map(0, NULL, (void**)&pMappedLights);

	SkinnedGameObject::InitSkinnedWorldTransformBuffer(_pDevice, _pCommandList);	// skinnedObject를 렌더하기 위한 (월드변환행렬을 담는)리소스를 생성한다.

}

void PlayScene::ReleaseUploadBuffers() {
	GameFramework& gameFramework = GameFramework::Instance();
	gameFramework.GetTextureManager().ReleaseUploadBuffers();
}


void PlayScene::ProcessKeyboardInput(const array<UCHAR, 256>& _keysBuffers, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {


	GameFramework& gameFramework = GameFramework::Instance();
	float angleSpeed = 720.f * _timeElapsed;
	float moveSpeed = 5.f * _timeElapsed;

	if (_keysBuffers['A'] & 0xF0) {
		XMFLOAT3 cameraLeft = pPlayer->GetCamera()->GetWorldRightVector();
		cameraLeft = Vector3::ScalarProduct(cameraLeft, -1);
		pPlayer->RotateMoveHorizontal(cameraLeft, angleSpeed, moveSpeed);

	}
	if (_keysBuffers['D'] & 0xF0) {
		XMFLOAT3 cameraRight = pPlayer->GetCamera()->GetWorldRightVector();
		pPlayer->RotateMoveHorizontal(cameraRight, angleSpeed, moveSpeed);
	}
	if (_keysBuffers['W'] & 0xF0) {
		XMFLOAT3 cameraLook = pPlayer->GetCamera()->GetWorldLookVector();
		pPlayer->RotateMoveHorizontal(cameraLook, angleSpeed, moveSpeed);
	}
	if (_keysBuffers['S'] & 0xF0) {
		XMFLOAT3 cameraBack = pPlayer->GetCamera()->GetWorldLookVector();
		cameraBack = Vector3::ScalarProduct(cameraBack, -1);
		pPlayer->RotateMoveHorizontal(cameraBack, angleSpeed, moveSpeed);
	}
	if (_keysBuffers[32] & 0xF0) {
		pPlayer->Jump(15.0f);
	}
	pZone->UpdatePlayerSector();
}

void PlayScene::AnimateObjects(char _collideCheck, double _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	pPlayer->Animate(_collideCheck, _timeElapsed);
	pZone->UpdatePlayerSector();
	camera->SetPlayerPos(pPlayer->GetWorldPosition());

	for (auto& pLight : pLights) {
		if (pLight) {
			pLight->UpdateLight();
		}
	}
}

void PlayScene::ProcessSocketMessage()
{

}

void PlayScene::UpdateLightShaderVariables(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	int nLight = (UINT)pLights.size();
	for (int i = 0; i < nLight; ++i) {

		memcpy(&pMappedLights->lights[i], pLights[i].get(), sizeof(Light));
	}

	memcpy(&pMappedLights->globalAmbient, &globalAmbient, sizeof(XMFLOAT4));
	memcpy(&pMappedLights->nLight, &nLight, sizeof(int));

	D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = pLightsBuffer->GetGPUVirtualAddress();
	_pCommandList->SetGraphicsRootConstantBufferView(2, gpuVirtualAddress);

}

void PlayScene::ReActButton(shared_ptr<Button> _pButton)
{

}

void PlayScene::ProcessMouseInput(UINT _type, XMFLOAT2 _pos) {

	GameFramework& gameFramework = GameFramework::Instance();
	Scene::ProcessMouseInput(_type, _pos);
	switch (_type) {
	case WM_LBUTTONDOWN:
		ReleaseCapture();

		break;
	case WM_LBUTTONUP:
		SetCapture(hWnd);
		gameFramework.InitOldCursor();
		break;
}
}

void PlayScene::ProcessCursorMove(XMFLOAT2 _delta)  {
	
	shared_ptr<Camera> pCamera = pPlayer->GetCamera();
	if (_delta.x != 0.0f) {
		//XMFLOAT3 upVector = pPlayer->GetCamera()->GetLocalUpVector();
		pCamera->SynchronousRotation(XMFLOAT3(0, 1, 0), _delta.x / 3.f);
		pCamera->UpdateLocalTransform();
	}
	if (_delta.y != 0.0f) {
		XMFLOAT3 rightVector = pCamera->GetLocalRightVector();
		pCamera->SynchronousRotation(rightVector, _delta.y / 3.f);
		pCamera->UpdateLocalTransform();
	}
	//pCamera->UpdateLocalTransform();

	// 적용
	pPlayer->UpdateObject();
}


void PlayScene::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed) {

	GameFramework& gameFramework = GameFramework::Instance();

	// 프레임워크에서 렌더링 전에 루트시그니처를 set
	camera->SetViewPortAndScissorRect(_pCommandList);
	camera->UpdateShaderVariable(_pCommandList);

	UpdateLightShaderVariables(_pCommandList);
	// 쉐이더 클래스에 정적으로 정의된 디스크립터 힙을 연결한다.

	Shader::SetDescriptorHeap(_pCommandList);
	
#ifdef USING_INSTANCING
	gameFramework.GetShader("InstancingShader")->PrepareRender(_pCommandList);
#else
	gameFramework.GetShader("BasicShader")->PrepareRender(_pCommandList);
#endif
	pZone->Render(_pCommandList, pPlayer->GetCamera()->GetBoundingFrustum());

	//gameFramework.GetShader("BoundingMeshShader")->PrepareRender(_pCommandList);
	//pFrustumMesh->UpdateMesh(camera->GetBoundingFrustum());
	//pFrustumMesh->Render(_pCommandList);
}

void PlayScene::AddLight(const shared_ptr<Light>& _pLight) {
	pLights.push_back(_pLight);
}

