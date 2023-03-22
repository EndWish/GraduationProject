#include "stdafx.h"
#include "Scene.h"
#include "Timer.h"
#include "GameFramework.h"

unordered_map<string, shared_ptr<Button>> Scene::pButtons;
unordered_map<string, shared_ptr<TextBox>> Scene::pTexts;
unordered_map<string, shared_ptr<Image2D>> Scene::pUIs;

shared_ptr<Image2D> Scene::GetUI(string _name) {
	return pUIs[_name];
}

shared_ptr<TextBox> Scene::GetText(string _name) {
	return pTexts[_name];
}

shared_ptr<Button> Scene::GetButton(string _name) {
	return pButtons[_name];
}


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

char Scene::CheckCollision(float _timeElapsed) {
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
	roomList.resize(6);
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

	gameFramework.GetSoundManager().Play("step");
	pUIs["2DUI_title"] = make_shared<Image2D>("2DUI_title", XMFLOAT2(2.f, 2.f), XMFLOAT2(0.f,0.f), XMFLOAT2(1.f,1.f), _pDevice, _pCommandList, false);
	pUIs["2DUI_roomBG"] = make_shared<Image2D>("2DUI_roomBG", XMFLOAT2(2.f, 2.f), XMFLOAT2(0.f, 0.f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
	pUIs["2DUI_roomListBG"] = make_shared<Image2D>("2DUI_roomListBG", XMFLOAT2(2.f, 2.f), XMFLOAT2(0.f, 0.f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);

	SetBackGround("2DUI_title");

	pUIs["2DUI_ready_1"] = make_shared<Image2D>("2DUI_ready", XMFLOAT2(0.285f, 0.142f), XMFLOAT2(0.08f, 1.58f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
	pUIs["2DUI_ready_2"] = make_shared<Image2D>("2DUI_ready", XMFLOAT2(0.285f, 0.142f), XMFLOAT2(0.47f, 1.58f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
	pUIs["2DUI_ready_3"] = make_shared<Image2D>("2DUI_ready", XMFLOAT2(0.285f, 0.142f), XMFLOAT2(0.856f, 1.58f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
	pUIs["2DUI_ready_4"] = make_shared<Image2D>("2DUI_ready", XMFLOAT2(0.285f, 0.142f), XMFLOAT2(1.244f, 1.58f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
	pUIs["2DUI_ready_5"] = make_shared<Image2D>("2DUI_ready", XMFLOAT2(0.285f, 0.142f), XMFLOAT2(1.632f, 1.58f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);


	pButtons["startButton"] = make_shared<Button>("2DUI_startButton", XMFLOAT2(0.3f, 0.2f), XMFLOAT2(1.5f, 1.2f), ButtonType::start, _pDevice, _pCommandList);;

	pButtons["optionButton"] = make_shared<Button>("2DUI_optionButton", XMFLOAT2(0.3f, 0.2f), XMFLOAT2(1.5f, 1.4f), ButtonType::option, _pDevice, _pCommandList);;

	pButtons["exitButton"] = make_shared<Button>("2DUI_exitButton", XMFLOAT2(0.3f, 0.2f), XMFLOAT2(1.5f, 1.6f), ButtonType::exit, _pDevice, _pCommandList);;

	/////////////////////////////////

	pButtons["RoomButton_1"] = make_shared<RoomButton>("2DUI_room", XMFLOAT2(0.4f, 0.2f), XMFLOAT2(0.5f, 0.4f), ButtonType::room, _pDevice, _pCommandList, false);
	pButtons["RoomButton_2"] = make_shared<RoomButton>("2DUI_room", XMFLOAT2(0.4f, 0.2f), XMFLOAT2(0.5f, 0.7f), ButtonType::room, _pDevice, _pCommandList, false);
	pButtons["RoomButton_3"] = make_shared<RoomButton>("2DUI_room", XMFLOAT2(0.4f, 0.2f), XMFLOAT2(0.5f, 1.0f), ButtonType::room, _pDevice, _pCommandList, false);
	pButtons["RoomButton_4"] = make_shared<RoomButton>("2DUI_room", XMFLOAT2(0.4f, 0.2f), XMFLOAT2(1.1f, 0.4f), ButtonType::room, _pDevice, _pCommandList, false);
	pButtons["RoomButton_5"] = make_shared<RoomButton>("2DUI_room", XMFLOAT2(0.4f, 0.2f), XMFLOAT2(1.1f, 0.7f), ButtonType::room, _pDevice, _pCommandList, false);
	pButtons["RoomButton_6"] = make_shared<RoomButton>("2DUI_room", XMFLOAT2(0.4f, 0.2f), XMFLOAT2(1.1f, 1.0f), ButtonType::room, _pDevice, _pCommandList, false);

	pButtons["refreshRoomButton"] = make_shared<Button>("2DUI_refreshRoomButton", XMFLOAT2(0.2f, 0.2f), XMFLOAT2(1.7f, 0.1f), ButtonType::refreshRoomList, _pDevice, _pCommandList, false);
	pButtons["prevRoomButton"] = make_shared<Button>("2DUI_leftArrowButton", XMFLOAT2(0.2f, 0.1f), XMFLOAT2(0.7f, 1.3f), ButtonType::prevRoomPage, _pDevice, _pCommandList, false);
	pButtons["nextRoomButton"] = make_shared<Button>("2DUI_rightArrowButton", XMFLOAT2(0.2f, 0.1f), XMFLOAT2(1.1f, 1.3f), ButtonType::nextRoomPage, _pDevice, _pCommandList, false);
	pButtons["titleButton"] = make_shared<Button>("2DUI_titleButton", XMFLOAT2(0.3f, 0.2f), XMFLOAT2(1.6f, 1.7f), ButtonType::title, _pDevice, _pCommandList, false);
	pButtons["makeRoom"] = make_shared<Button>("2DUI_makeRoom", XMFLOAT2(0.3f, 0.2f), XMFLOAT2(1.25f, 1.7f), ButtonType::makeRoom, _pDevice, _pCommandList, false);

	shared_ptr<TextBox> pText = make_shared<TextBox>((WCHAR*)L"휴먼돋움체", D2D1::ColorF(1, 1, 1, 1), XMFLOAT2(0.9f, 1.3f), XMFLOAT2(0.2f, 0.2f), C_WIDTH / 40.0f, false);
	pTexts["pageNum"] = pText;

	
	////////////////////////////////////



	pButtons["gameStartButton"] = make_shared<Button>("2DUI_startButton", XMFLOAT2(0.3f, 0.2f), XMFLOAT2(1.25f, 1.7f), ButtonType::gameStart, _pDevice, _pCommandList, false);
	pButtons["quitRoomButton"] = make_shared<Button>("2DUI_quitRoomButton", XMFLOAT2(0.3f, 0.2f), XMFLOAT2(1.6f, 1.7f), ButtonType::quitRoom, _pDevice, _pCommandList, false);
}

void LobbyScene::ReleaseUploadBuffers() {

}

void LobbyScene::ProcessKeyboardInput(const array<bool, 256>& _keyDownBuffer, const array<UCHAR, 256>& _keysBuffers, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

}

void LobbyScene::AnimateObjects(char _collideCheck, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList)  {

}

void LobbyScene::ProcessSocketMessage(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();

	// 고정길이의 패킷을 Recv받는다.
	static int recvByte = 0;

	int result = RecvFixedPacket(recvByte);
	if (result != SOCKET_ERROR) {
		recvByte += result;
		// 덜 받은 경우
		if (recvByte < BUFSIZE) {
			return;
		}
		// 모두 받았을 경우
		else {
			recvByte = 0;
		}
	}
	else {
		// EWOULDBLOCK
		return;
	}
	
	
	// 첫바이트를 읽어 패킷 타입을 알아낸다.
	SC_PACKET_TYPE packetType = (SC_PACKET_TYPE)recvBuffer[0];

	switch (packetType) {

	case SC_PACKET_TYPE::roomListInfo: {
		SC_ROOMLIST_INFO* packet = GetPacket<SC_ROOMLIST_INFO>();
		// Roomlist 내 nRoom의 수 만큼 SC_SUB_ROOMLIST_INFO 패킷을 추가로 한꺼번에 받는다.

		for (int i = 0; i < 6; ++i) {
			roomList[i] = packet->roomInfo[i];
		}
		//recv(server_sock, (char*)roomList.data(), sizeof(SC_SUB_ROOMLIST_INFO) * packet.nRoom, 0);
		UpdateRoomText();
		break;
	}	// 현재 방의 리스트를 받는 패킷
	case SC_PACKET_TYPE::roomPlayersInfo: { // 입장 질의를 보내고 난 후 입장이 가능하다고 받음
		SC_ROOM_PLAYERS_INFO* packet = GetPacket<SC_ROOM_PLAYERS_INFO>();
		roomInfo.players.clear();
		roomInfo.id = packet->roomID;
		roomInfo.host = packet->hostID;
		roomInfo.nParticipant = packet->nParticipant;

		for (UINT i = 0; i < roomInfo.nParticipant; ++i) {
			Player_Info pi{ packet->participantInfos[i].clientID, packet->participantInfos[i].ready };
			memcpy(pi.name, packet->participantInfos[i].name, 20);
			roomInfo.players.push_back(pi);
		}

		changeUI(LobbyState::roomList, false);
		changeUI(LobbyState::inRoom, true);
		UpdateReadyState();
		break;
	} 
	case SC_PACKET_TYPE::fail: { 	// 방이 시작했거나, 꽉차거나 삭제되어 방 입장 실패한 경우
		// 실패
		SC_FAIL* packet = GetPacket<SC_FAIL>();
		
		// 실패에 대한 처리
		break;
	}
	case SC_PACKET_TYPE::ready: {	// 누군가 준비를 눌렀을 경우 
		SC_READY* packet = GetPacket<SC_READY>();

		// 해당 cid를 가진 플레이어를 찾아 레디상태를 반대로 바꾸어준다.
		auto pindex = roomInfo.findPlayerIndex(packet->readyClientID);
		pindex->ready = !pindex->ready;
		UpdateReadyState();
		break;
	}
	case SC_PACKET_TYPE::roomVisitPlayerInfo: {	// 누가 방에 들어온 경우
		SC_ROOM_VISIT_PLAYER_INFO* packet = GetPacket<SC_ROOM_VISIT_PLAYER_INFO>();

		// 방 정보에 해당 플레이어의 정보를 추가한다.
		Player_Info pi{ packet->visitClientID, false };
		memcpy(pi.name, packet->name, 20);
		roomInfo.players.push_back(pi);
		roomInfo.nParticipant++;
		UpdateReadyState();
		break;
	}
	case SC_PACKET_TYPE::roomOutPlayerInfo: { // 누가 방에서 나간 경우
		SC_ROOM_OUT_PLAYER_INFO* packet = GetPacket<SC_ROOM_OUT_PLAYER_INFO>();

		// 해당 클라이언트를 찾아 지우고 방장이 바뀌었다면 새로 임명한다.
		auto pindex = roomInfo.findPlayerIndex(packet->outClientID);
		roomInfo.players.erase(pindex);
		roomInfo.nParticipant--;
		roomInfo.host = packet->newHostID;
		UpdateReadyState();
		break;
	}
	case SC_PACKET_TYPE::gameStart: {
		// 게임이 돌아왔을 때 모든 플레이어가 대기상태여야 하기 때문에 미리 풀어준다.
		for (auto& player : roomInfo.players) {
			player.ready = false;
		}

		changeUI(LobbyState::inRoom, false);
		loadingScene = make_shared<PlayScene>();

		// 게임이 시작된 경우 먼저 게임에서 사용될 인스턴스 정보, 메쉬, 애니메이션, 텍스처 등의 정보를 로드한다.
		gameFramework.LoadingScene(loadingScene);

		// 플레이 씬 로딩이 모두 완료된 경우 서버에게 로딩 완료 패킷을 보낸다. 
		CS_LOADING_COMPLETE sendPacket;
		sendPacket.roomID = roomInfo.id;

		sendPacket.cid = cid;
		SendFixedPacket(sendPacket);
		break;
	}
	case SC_PACKET_TYPE::allPlayerLoadingComplete: {
		pBackGround->SetEnable(false);
		SetCapture(hWnd);
		gameFramework.InitOldCursor();
		gameFramework.PushScene(loadingScene);
		loadingScene = nullptr;
		break;
	}
	case SC_PACKET_TYPE::yourPlayerObjectID: {
		SC_YOUR_PLAYER_OBJECTID* packet = GetPacket<SC_YOUR_PLAYER_OBJECTID>();

		myObjectID = packet->objectID;
		break;
	}
	default:
		cout << "나머지 패킷. 타입 = " << (int)packetType << "\n";
	}

}

void LobbyScene::RenderShadowMap(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, UINT _lightIndex) {

}

void LobbyScene::PreRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed) {
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
		sPacket.roomPage = 1;
		SendFixedPacket(sPacket);
		changeUI(LobbyState::title, false);
		changeUI(LobbyState::roomList, true);
		break;
	}
	case ButtonType::makeRoom: {

		// 패킷을 보낸 후, 방 생성은 항상 성공하므로 서버에게 따로 패킷을 받지 않고
		// 본인이 만든 방으로 입장한다.
		CS_MAKE_ROOM sPacket;
		sPacket.hostID = cid;
		SendFixedPacket(sPacket);

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
		SendFixedPacket(sPacket);

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
		sPacket.visitRoomID = dynamic_pointer_cast<RoomButton>(_pButton)->GetRoomIndex();
		SendFixedPacket(sPacket);
		break;
	}
	case ButtonType::gameStart: {
		CS_READY sPacket;
		sPacket.cid = cid;
		SendFixedPacket(sPacket);
		break;
	}
	// 서버에게 한 페이지씩만 룸의 정보를 요청한다.
	case ButtonType::prevRoomPage: {
		if (roomPage > 1) roomPage--;
		CS_QUERY_ROOMLIST_INFO sPacket;
		sPacket.cid = cid;
		sPacket.roomPage = roomPage;
		SendFixedPacket(sPacket);
		break;
	}
	case ButtonType::nextRoomPage: {
		roomPage++;
		CS_QUERY_ROOMLIST_INFO sPacket;
		sPacket.cid = cid;
		sPacket.roomPage = roomPage;
		SendFixedPacket(sPacket);
		break;
	}
	case ButtonType::refreshRoomList: {
		CS_QUERY_ROOMLIST_INFO sPacket;
		sPacket.cid = cid;
		sPacket.roomPage = roomPage;
		SendFixedPacket(sPacket);
		break;
	}
	}
}

void LobbyScene::NoticeCloseToServer() {
	if (currState == LobbyState::inRoom) {
		// 방 안에 있을때 클라이언트 종료 시 방 나가기 버튼을 눌렀을때와 같은 동작 수행
		CS_OUT_ROOM sPacket;
		sPacket.cid = cid;
		SendFixedPacket(sPacket);
	}
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          
void LobbyScene::changeUI(LobbyState _state, bool _active) {
	// 로비씬의 각 세부 상태에 따라 버튼, 배경 이미지를 변경한다.  
	if (_state == LobbyState::title) {
		pButtons["startButton"]->SetEnable(_active);
		pButtons["optionButton"]->SetEnable(_active);
		pButtons["exitButton"]->SetEnable(_active);
		if (_active) {
			SetBackGround("2DUI_title");
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

			SetBackGround("2DUI_roomListBG");
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
			SetBackGround("2DUI_roomBG");
		}
	}
	if (_active) currState = _state;
}

void LobbyScene::UpdateRoomText() {
	pTexts["pageNum"]->SetText(to_wstring(roomPage));
	// 현재 페이지 기준 방 리스트의 정보로 갱신 해준다.
	bool lastRoom = false;

	for (UINT i = 0; i < 6; ++i) {

		RoomState state = RoomState::none;
		string baseName = "RoomButton_" + to_string(i + 1);

		// 방 정보가 없는 칸일 경우 
		if (roomList[i].nParticipant == 0) {
			state = RoomState::none;
			lastRoom = true; // 이후에 나오는 룸들을 모두 빈 방으로 표시한다.
		}

		if (!lastRoom) {
			if (roomList[i].started) { // 이미 시작한 경우
				state = RoomState::started;
			}
			else if (roomList[i].nParticipant == 5) {	// 정원이 꽉 찬 경우
				state = RoomState::full;
			}
			else state = RoomState::joinable;
		}
		dynamic_cast<RoomButton*>(pButtons[baseName].get())->UpdateState(roomList[i].roomID, roomList[i].nParticipant ,state);
	}
}

void LobbyScene::SetBackGround(string _bgName) {
	if(pBackGround) pBackGround->SetEnable(false);
	pBackGround = pUIs[_bgName];
	pBackGround->SetEnable(true);
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
	globalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);
	remainTime = 1000.f;
	professorObjectID = 0;
}

PlayScene::~PlayScene() {
	pLightsBuffer->Unmap(0, NULL);
}

void PlayScene::changeUI(bool _enable) {
	pUIs["2DUI_stamina"]->SetEnable(_enable);
	pUIs["2DUI_staminaFrame"]->SetEnable(_enable);
	pUIs["2DUI_interact"]->SetEnable(_enable);
	pUIs["2DUI_hackRate"]->SetEnable(_enable);
	pTexts["leftCoolTime"]->SetEnable(_enable);
	pTexts["rightCoolTime"]->SetEnable(_enable);
	pTexts["remainTime"]->SetEnable(_enable);
	pTexts["hackRate"]->SetEnable(_enable);
	

	if (isPlayerProfessor) {	// 교수일 경우의 UI 
		pUIs["2DUI_throwAttack"]->SetEnable(_enable);
		pUIs["2DUI_swingAttack"]->SetEnable(_enable);
	}
	else {		// 학생일 경우의 UI 
		pUIs["2DUI_leftSkill"]->SetEnable(_enable);
		pUIs["2DUI_rightSkill"]->SetEnable(_enable);
		pUIs["2DUI_energyDrink"]->SetEnable(_enable);
		pUIs["2DUI_medicalKit"]->SetEnable(_enable);
		pUIs["2DUI_prisonKey"]->SetEnable(_enable);
		pUIs["2DUI_trap"]->SetEnable(_enable);

		pUIs["2DUI_hp"]->SetEnable(_enable);
		pUIs["2DUI_hpFrame"]->SetEnable(_enable);

		pUIs["2DUI_hacking"]->SetEnable(_enable);
		pUIs["2DUI_hackingFrame"]->SetEnable(_enable);
	}
}

void PlayScene::AddComputer(const shared_ptr<Computer>& _pComputer) {
	pEnableComputers.push_back(_pComputer);
}
 
void PlayScene::UpdateTimeText() {
	UINT UINTTime = (UINT)remainTime;

	pTexts["remainTime"]->SetText(to_wstring(UINTTime / 60) + L" : " + to_wstring(UINTTime % 60));
	
}

void PlayScene::SetPlayer(shared_ptr<Player>& _pPlayer) {
	pPlayer = _pPlayer;
}

char PlayScene::CheckCollision(float _timeElapsed) {
	char result = 0;
	XMFLOAT3 velocity = pPlayer->GetVelocity();
	// 플레이어가 다음 프레임에 이동할 실제 변위값
	XMFLOAT3 moveVector = Vector3::ScalarProduct(pPlayer->GetWorldLookVector(), pPlayer->GetVelocity().z);
	float moveDistance = Vector3::Length(moveVector);
	// 내가 조종하는 플레이어가 탈출box 와 충돌하는지 확인
	if (!exit && !isPlayerProfessor && exitBox.Intersects(pPlayer->GetBoundingBox())) {
		exit = true;

		// 서버에게 자신이 탈출했다고 패킷을 보낸다.
		CS_EXIT_PLAYER sendPacket;
		sendPacket.cid = cid;
		sendPacket.playerObjectID = myObjectID;
		SendFixedPacket(sendPacket);
	}

	// 플레이어가 학생일경우 공격, 아이템과 플레이어의 충돌처리
	if (!isPlayerProfessor) {
		pZone->CheckCollisionWithAttack();
		pZone->CheckCollisionWithItem();
	}
	else {
		pZone->CheckCollisionWithTrap();
	}
	// 투사체와 장애물간의 충돌을 처리
	pZone->CheckCollisionProjectileWithObstacle();

	shared_ptr<GameObject> collideObj = pZone->CheckCollisionVertical(_timeElapsed);

	// 플레이어의 OOBB를 y방향으로 이동시켜 본 후 충돌체크를 진행한다.
	if (!collideObj) {
		result |= 1<<0;
	}
	else {
		// 바닥이 없다면 새로 지정한다.
		if(!pPlayer->GetFloor()) pPlayer->SetFloor(collideObj);
	}

	//XMFLOAT3 normalVector = GetCollideNormalVector(collideObj);
	// 슬라이딩 벡터
	//pPlayer->SetMoveFrontVector(Vector3::Subtract(moveVector, Vector3::ScalarProduct(normalVector, Vector3::DotProduct(moveVector, normalVector))));
	


	XMFLOAT3 knockBack = XMFLOAT3();
	// 플레이어의 x,z이동 및 OOBB를 회전시켜본 후 충돌체크를 진행한다.
	vector<shared_ptr<GameObject>> collideObjs = pZone->CheckCollisionRotate(pPlayer->GetFloor());
	collideObj = pZone->CheckCollisionHorizontal(pPlayer->GetFloor());

	if (collideObj) collideObjs.push_back(collideObj);

	 if (collideObjs.size() != 0) {
		// 부딪힐 경우 

		XMFLOAT3 collideNormalVector; 

		for (auto& collideObj : collideObjs) {
			collideNormalVector = GetCollideNormalVector(collideObj);
			// 정지상태일 경우
			if (moveDistance == 0.f) {
				knockBack = Vector3::Add(knockBack, Vector3::ScalarProduct(collideNormalVector, 0.01f));
			}
			
			// 플레이어가 이동한 변위에 대해, 부딪힌 면의 법선벡터 방향의 성분을 뺀다.
			else knockBack = Vector3::Subtract(knockBack, Vector3::ScalarProduct(collideNormalVector, Vector3::DotProduct(moveVector, collideNormalVector)));
		}
		pPlayer->SetKnockBack(knockBack);
		// 후에 플레이어를 이동시킨후, 합성된 knockBack 벡터로 다시 밀어준다.


		// 물체의 크기 때문에 법선벡터가 잘못 구해진 경우를 처리하기 위한 부분
		if (moveDistance != 0.f) {
			for (auto& collideObj : collideObjs) {
				BoundingOrientedBox checkOOBB = pPlayer->GetBoundingBox();
				checkOOBB.Center = Vector3::Add(Vector3::Add(checkOOBB.Center, moveVector), knockBack);
				checkOOBB.Orientation = Vector4::QuaternionMultiply(checkOOBB.Orientation, pPlayer->GetRotation());
				if (pPlayer->GetFloor() == collideObj) continue;
				if (checkOOBB.Intersects(collideObj->GetBoundingBox())) {
					result |= 1 << 1;
					break;
				}
			}
		}
	}
	

	if (pZone->CheckObstacleBetweenPlayerAndCamera(camera)) {
		if (camera->GetMinDistance() < camera->GetCurrentDistance()) {
			camera->MoveFront(15.f, min(_timeElapsed, 1.f/30.f));
		}
	}
	else {
		if (camera->GetCurrentDistance() < camera->GetMaxDistance()) {
			camera->MoveFront(-15.f, min(_timeElapsed, 1.f / 30.f));
			camera->UpdateObject();
			if (pZone->CheckObstacleBetweenPlayerAndCamera(camera)) {
				camera->MoveFront(15.f, min(_timeElapsed, 1.f / 30.f));
				camera->UpdateObject();
			}
		}
	}

	return result;
}

XMFLOAT3 PlayScene::GetCollideNormalVector(const shared_ptr<GameObject>& _collideObj) {
	
	bool isOppositeLook = false;
	XMFLOAT3 result, lookVector, rightVector, direcVector, boundaryVector;
	float boundingAngle, playerAngle;

	// 충돌한 바운딩박스의 꼭짓점 위치에 대한 벡터를 구한다.
	
	BoundingOrientedBox objOOBB = _collideObj->GetBoundingBox();
	BoundingOrientedBox playerOOBB = pPlayer->GetBoundingBox();
	lookVector = _collideObj->GetWorldLookVector();
	rightVector = _collideObj->GetWorldRightVector();

	boundaryVector = Vector3::Rotate(XMFLOAT3(objOOBB.Extents.x, 0, objOOBB.Extents.z), objOOBB.Orientation);
	// 충돌한 물체의 룩벡터와의 각도를 구한다.
	boundingAngle = Vector3::Angle(boundaryVector, lookVector, false);
	if (boundingAngle > 90.0f) boundingAngle = 180.0f - boundingAngle;

	// 물체와 플레이어간의 벡터의 각을 구한다.
	direcVector = Vector3::Subtract(playerOOBB.Center, objOOBB.Center);
	direcVector.y = 0;

	// 플레이어와 룩벡터의 각을 구한다.
	playerAngle = Vector3::Angle(direcVector, lookVector, false);

	// 룩벡터 방향과 반대일 경우
	if (playerAngle > 90.0f) {
		playerAngle = 180.0f - playerAngle;
		isOppositeLook = true;
	}
	// 꼭짓점 위치와 룩벡터간의 각도보다 플레이어와 룩벡터가 이루는 각이 클경우
	// = RightVector로 본다.
	if (playerAngle > boundingAngle) {
		// RightVector방향의 반대일경우
		result = rightVector;
		if (Vector3::Angle(direcVector, rightVector, false) > 90.0f) {
			result = Vector3::ScalarProduct(result, -1.f);
		}
	}
	else {
		result = lookVector;
		if (isOppositeLook) {
			result = Vector3::ScalarProduct(result, -1.f);
		}
	}
	return Vector3::Normalize(result);
}

void PlayScene::Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();
	
	gameFramework.GetGameObjectManager().LoadGameObject("SwingAttack", _pDevice, _pCommandList);
	gameFramework.GetGameObjectManager().LoadGameObject("BookAttack", _pDevice, _pCommandList);
	gameFramework.GetGameObjectManager().LoadGameObject("PrisonKey", _pDevice, _pCommandList);
	gameFramework.GetGameObjectManager().LoadGameObject("MedicalKit", _pDevice, _pCommandList);
	gameFramework.GetGameObjectManager().LoadGameObject("EnergyDrink", _pDevice, _pCommandList);
	gameFramework.GetGameObjectManager().LoadGameObject("Trap", _pDevice, _pCommandList);
	gameFramework.GetGameObjectManager().LoadGameObject("Trap_attack", _pDevice, _pCommandList);

	pFullScreenObject = make_shared<FullScreenObject>(_pDevice, _pCommandList);

	// 전역 변수 초기화
	AllLeverPowerOn = false;
	isPlayerProfessor = false;

	SC_GAME_START* recvPacket = GetPacket<SC_GAME_START>();
	array<UINT, MAX_PARTICIPANT> enComID;
	for (int i = 0; i < MAX_PARTICIPANT; ++i) {
		enComID[i] = recvPacket->activeComputerObjectID[i];
	}

	// Zone을 생성 후 맵파일을 읽어 오브젝트들을 로드한다.
	pZone = make_shared<Zone>(XMFLOAT3(100.f, 100.f, 100.f), XMINT3(10, 10, 10), shared_from_this());
	pZone->LoadZoneFromFile(_pDevice, _pCommandList, enComID);
	
	professorObjectID = recvPacket->professorObjectID;

	for (UINT i = 0; i < recvPacket->nPlayer; ++i) {

		if (recvPacket->playerInfo[i].objectID == myObjectID) {	// 내가 조종할 캐릭터일 경우
			if (professorObjectID == recvPacket->playerInfo[i].objectID) {	// 교수 플레이어일 경우
				pPlayer = make_shared<Professor>();
				isPlayerProfessor = true;
				
			}
			else { // 학생일 경우
				pPlayer = make_shared<Student>();
			}
			
			pPlayer->Create("Player"s, _pDevice, _pCommandList);
			pPlayer->SetLocalPosition(recvPacket->playerInfo[i].position);
			pPlayer->SetLocalRotation(recvPacket->playerInfo[i].rotation);
			pPlayer->SetLocalScale(recvPacket->playerInfo[i].scale);
			pPlayer->UpdateObject();
			pPlayer->SetID(recvPacket->playerInfo[i].objectID);
			SetPlayer(pPlayer);
			pZone->SetPlayer(pPlayer);
			//[수정] 애니메이션 정보 갱신

		}
		else {	// 다른 플레이어의 캐릭터 정보일 경우
			shared_ptr<InterpolateMoveGameObject> pOtherPlayer = make_shared<InterpolateMoveGameObject>();

			if (professorObjectID == recvPacket->playerInfo[i].objectID) {	// 교수 플레이어일 경우
				pOtherPlayer->SetNickname(wstring(recvPacket->nickname[i]), true);
			}
			else { // 학생일 경우
				pOtherPlayer->SetNickname(wstring(recvPacket->nickname[i]), false);
			}
			pOtherPlayer->Create("Player"s, _pDevice, _pCommandList);
			pOtherPlayer->SetLocalPosition(recvPacket->playerInfo[i].position);
			pOtherPlayer->SetLocalRotation(recvPacket->playerInfo[i].rotation);
			pOtherPlayer->SetLocalScale(recvPacket->playerInfo[i].scale);
			pOtherPlayer->UpdateObject();
			pOtherPlayer->SetID(recvPacket->playerInfo[i].objectID);

			pOtherPlayers.emplace(pOtherPlayer->GetID(), pOtherPlayer);
			
			// 충돌처리를 위해 Sector에 추가한다.
			pZone->AddObject(SectorLayer::otherPlayer, recvPacket->playerInfo[i].objectID, pOtherPlayer, pZone->GetIndex(pOtherPlayer->GetWorldPosition()));
			//[수정] 애니메이션 정보 갱신
		}

	}

	pPlayer->UpdateObject();
	camera = pPlayer->GetCamera();

	pSkyBox = make_shared<SkyBox>(_pDevice, _pCommandList);

	shared_ptr<Image2D> pImg;

	pUIs["2DUI_stamina"] = make_shared<Image2D>("2DUI_stamina", XMFLOAT2(0.6f, 0.15f), XMFLOAT2(1.4f, 1.85f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, true);
	pUIs["2DUI_staminaFrame"] = make_shared<Image2D>("2DUI_staminaFrame", XMFLOAT2(0.6f, 0.15f), XMFLOAT2(1.4f, 1.85f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, true);
	pUIs["2DUI_interact"] = make_shared<Image2D>("2DUI_interact", XMFLOAT2(0.3f, 0.1f), XMFLOAT2(0.f, 0.f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
	pUIs["2DUI_hackRate"] = make_shared<Image2D>("2DUI_hackRate", XMFLOAT2(0.3f, 0.1f), XMFLOAT2(0.f, 0.f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);

	pTexts["leftCoolTime"] = make_shared<TextBox>((WCHAR*)L"휴먼돋움체", D2D1::ColorF(1, 1, 1, 1), XMFLOAT2(1.55f, 1.5f), XMFLOAT2(0.1f, 0.1f), C_WIDTH / 30.0f, false);
	pTexts["rightCoolTime"] = make_shared<TextBox>((WCHAR*)L"휴먼돋움체", D2D1::ColorF(1, 1, 1, 1), XMFLOAT2(1.8f, 1.5f), XMFLOAT2(0.1f, 0.1f), C_WIDTH / 30.0f, false);

	pTexts["remainTime"] = make_shared<TextBox>((WCHAR*)L"휴먼돋움체", D2D1::ColorF(1, 1, 1, 1), XMFLOAT2(0.9f, 0.1f), XMFLOAT2(0.2f, 0.2f), C_WIDTH / 40.0f, true);
	pTexts["hackRate"] = make_shared<TextBox>((WCHAR*)L"휴먼돋움체", D2D1::ColorF(1, 1, 1, 1), XMFLOAT2(0.9f, 0.3f), XMFLOAT2(0.2f, 0.2f), C_WIDTH / 60.0f, true);


	if (isPlayerProfessor) {	// 교수일 경우의 UI 로드
		pUIs["2DUI_swingAttack"] = make_shared<Image2D>("2DUI_swingAttack", XMFLOAT2(0.2f, 0.2f), XMFLOAT2(1.5f, 1.5f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, true);
		pUIs["2DUI_throwAttack"] = make_shared<Image2D>("2DUI_throwAttack", XMFLOAT2(0.2f, 0.2f), XMFLOAT2(1.75f, 1.5f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, true);
	}
	else {		// 학생일 경우의 UI 로드
		pUIs["2DUI_leftSkill"] = make_shared<Image2D>("2DUI_skillFrame", XMFLOAT2(0.2f, 0.2f), XMFLOAT2(1.5f, 1.5f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, true);
		pUIs["2DUI_rightSkill"] = make_shared<Image2D>("2DUI_skillFrame", XMFLOAT2(0.2f, 0.2f), XMFLOAT2(1.75f, 1.5f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, true);

		pUIs["2DUI_energyDrink"] = make_shared<Image2D>("2DUI_energyDrink", XMFLOAT2(0.2f, 0.2f), XMFLOAT2(1.5f, 1.5f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
		pUIs["2DUI_medicalKit"] = make_shared<Image2D>("2DUI_medicalKit", XMFLOAT2(0.2f, 0.2f), XMFLOAT2(1.5f, 1.5f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
		pUIs["2DUI_prisonKey"] = make_shared<Image2D>("2DUI_prisonKey", XMFLOAT2(0.2f, 0.2f), XMFLOAT2(1.5f, 1.5f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
		pUIs["2DUI_trap"] = make_shared<Image2D>("2DUI_trap", XMFLOAT2(0.2f, 0.2f), XMFLOAT2(1.5f, 1.5f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);

		pUIs["2DUI_hp"] = make_shared<Image2D>("2DUI_hp", XMFLOAT2(0.6f, 0.15f), XMFLOAT2(1.4f, 1.7f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, true);
		pUIs["2DUI_hpFrame"] = make_shared<Image2D>("2DUI_hpFrame", XMFLOAT2(0.6f, 0.15f), XMFLOAT2(1.4f, 1.7f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, true);

		pUIs["2DUI_hacking"] = make_shared<Image2D>("2DUI_hacking", XMFLOAT2(0.5f, 0.1f), XMFLOAT2(0.75f, 1.4f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
		pUIs["2DUI_hackingFrame"] = make_shared<Image2D>("2DUI_hackingFrame", XMFLOAT2(0.5f, 0.1f), XMFLOAT2(0.75f, 1.4f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
	}
	


	// 빛을 추가
	shared_ptr<Light> baseLight = make_shared<Light>();

	baseLight->lightType = 3;
	baseLight->position = XMFLOAT3(0, 500, 0);
	baseLight->direction = XMFLOAT3(0, -1, 0);
	baseLight->diffuse = XMFLOAT4(0.5, 0.5, 0.5, 1);
	baseLight->specular = XMFLOAT4(0.01f, 0.01f, 0.01f, 1.0f);
	AddLight(baseLight);
	baseLight->UpdateViewTransform();

	pFrustumMesh = make_shared<FrustumMesh>();
	pFrustumMesh->Create(camera->GetBoundingFrustum(), _pDevice, _pCommandList);

	ComPtr<ID3D12Resource> temp;
	UINT ncbElementBytes = ((sizeof(LightsMappedFormat) + 255) & ~255); //256의 배수
	pLightsBuffer = ::CreateBufferResource(_pDevice, _pCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, temp);
	pLightsBuffer->Map(0, NULL, (void**)&pMappedLights);

	SkinnedGameObject::InitSkinnedWorldTransformBuffer(_pDevice, _pCommandList);	// skinnedObject를 렌더하기 위한 (월드변환행렬을 담는)리소스를 생성한다.
	
	Shader::SetCamera(camera);
}

void PlayScene::ReleaseUploadBuffers() {
	GameFramework& gameFramework = GameFramework::Instance();
	gameFramework.GetTextureManager().ReleaseUploadBuffers();
}


void PlayScene::ProcessKeyboardInput(const array<bool, 256>& _keyDownBuffer, const array<UCHAR, 256>& _keysBuffers, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	bool move = false;

	GameFramework& gameFramework = GameFramework::Instance();
	// 등속운동은 미리 timeElapsed를 곱해준다


	if (_keyDownBuffer['E']) {
		// 상호작용 키
		if (pInteractableObject && pInteractableObject->IsEnable())
			pInteractableObject->QueryInteract();
	}
	if (_keyDownBuffer['R']) {
		CS_OPEN_PRISON_DOOR sendPacket;
		sendPacket.cid = cid;
		SendFixedPacket(sendPacket);
	}

	if (_keyDownBuffer['T']) {

	}

	if (_keysBuffers[VK_SHIFT] & 0xF0) {
		pPlayer->Dash(_timeElapsed);
	}

	float angleSpeed = 720.f * _timeElapsed;
	float moveSpeed = pPlayer->GetSpeed() * _timeElapsed * (1 - pPlayer->GetSlowRate() / 100.0f);
	XMFLOAT3 moveVector = XMFLOAT3();

	if (_keysBuffers['A'] & 0xF0) {
		XMFLOAT3 cameraLeft = pPlayer->GetCamera()->GetWorldRightVector();
		cameraLeft = Vector3::ScalarProduct(cameraLeft, -1);
		moveVector = Vector3::Add(moveVector, cameraLeft);
		move = true;
	}
	if (_keysBuffers['D'] & 0xF0) {
		XMFLOAT3 cameraRight = pPlayer->GetCamera()->GetWorldRightVector();
		moveVector = Vector3::Add(moveVector, cameraRight);
		move = true;
	}
	if (_keysBuffers['W'] & 0xF0) {
		XMFLOAT3 cameraLook = pPlayer->GetCamera()->GetWorldLookVector();
		moveVector = Vector3::Add(moveVector, cameraLook);
		move = true;
	}
	if (_keysBuffers['S'] & 0xF0) {
		XMFLOAT3 cameraBack = pPlayer->GetCamera()->GetWorldLookVector();
		cameraBack = Vector3::ScalarProduct(cameraBack, -1);
		moveVector = Vector3::Add(moveVector, cameraBack);
		move = true;
	}
	if (move && !Vector3::IsSame(XMFLOAT3(), moveVector)) {
		moveVector = Vector3::Normalize(moveVector);
		pPlayer->RotateMoveHorizontal(moveVector, angleSpeed, moveSpeed);

	}
	if (_keysBuffers[VK_SPACE] & 0xF0) {
		pPlayer->Jump(500.0f);
	}
}

void PlayScene::AnimateObjects(char _collideCheck, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	// 탈출하게 되면 화면이 흰색이 되면서 완전 흰색이 되면 서버에 탈출했다는 패킷을 보내면서 게임이 끝난다.
	if (3 <= fadeOut) {
		GameFramework& gameFramework = GameFramework::Instance();

		// 게임을 끝낸다.
		CS_EXIT_GAME sendPacket;
		sendPacket.cid = cid;
		SendFixedPacket(sendPacket);
		changeUI(false);
		gameFramework.PopScene();
		ReleaseCapture();

		auto pLobbyScene = dynamic_pointer_cast<LobbyScene>(gameFramework.GetCurrentScene());
		if (pLobbyScene) {
			pLobbyScene->changeUI(LobbyState::inRoom, true);
			pLobbyScene->UpdateReadyState();
		}
		return;
	}

	GameFramework& gameFramework = GameFramework::Instance();
	for (auto& pEffect : pEffects) {
		pEffect->Animate(_timeElapsed);
	}

	// 플레이어 애니메이션
	pPlayer->Animate(_collideCheck, _timeElapsed);

	// 청자의 위치를 업데이트해준다. ( 최초 Play 전 호출시 프레임 드랍 )
	gameFramework.GetSoundManager().UpdateListener(pPlayer->GetWorldPosition(), camera->GetWorldLookVector(), camera->GetWorldUpVector());

	bool enable = false;
	// 현재 플레이어가 상호작용 가능한 오브젝트를 찾는다.
	auto pObject = pZone->UpdateInteractableObject();

	// 교수의 사보타지 쿨타임일때는 상호작용하지 못하도록 한다.
	if (dynamic_pointer_cast<Lever>(pObject) && isPlayerProfessor 
		&& 0 < dynamic_pointer_cast<Professor>(pPlayer)->GetSabotageCoolTime()) {
		pObject = nullptr;
	}

	// 주변에 상호작용 오브젝트가 있다가 없어진 경우
	if (pInteractableObject && !pObject) {
		pInteractableObject->EndInteract();
	}
	pInteractableObject = pObject;

	auto pComputer = pEnableComputers.end();


	if (pInteractableObject) {

		// 현재 플레이어가 해킹중인 컴퓨터가 있는지 확인
		// 현재 주변에 상호작용 오브젝트가 있으며 사용 가능할 경우
		enable = pInteractableObject->IsEnable();
		if (enable) {
			XMFLOAT2 pos = GetWorldToScreenCoord(pInteractableObject->GetBoundingBox().Center, camera->GetViewTransform(), camera->GetProjectionTransform());
			// 상호작용 UI의 좌표를 갱신해준다.
			// 뷰포트 좌표계 -1~1 -> UI좌표계 0~2        
			pUIs["2DUI_interact"]->SetPosition(XMFLOAT2(pos.x + 1, pos.y + 1));
		}
		// 내가 사용중인 컴퓨터인 경우
		else {
			pComputer = ranges::find(pEnableComputers, myObjectID, &Computer::GetUse);
			if (pComputer != pEnableComputers.end()) {
				float hackingRate = (*pComputer)->GetHackingRate();
				pUIs["2DUI_hacking"]->SetEnable(true);
				pUIs["2DUI_hacking"]->SetSizeUV(XMFLOAT2(hackingRate / 100, 1.f));
				pUIs["2DUI_hackingFrame"]->SetEnable(true);
			}
		}
	}

	// 플레이어가 교수일경우의 UI
	if (isPlayerProfessor) {
		auto pProfessor = static_pointer_cast<Professor>(pPlayer);

	}
	// 플레이어가 학생일경우의 UI
	else {
		auto pStudent = static_pointer_cast<Student>(pPlayer);
			pUIs["2DUI_hp"]->SetSizeUV(XMFLOAT2(pStudent->GetHP() / 100, 1.f));
			if (pComputer == pEnableComputers.end()) {
				pUIs["2DUI_hacking"]->SetEnable(false);
				pUIs["2DUI_hackingFrame"]->SetEnable(false);
		}
	}

	pUIs["2DUI_stamina"]->SetSizeUV(XMFLOAT2(pPlayer->GetMP() / 100, 1.f));
	pUIs["2DUI_interact"]->SetEnable(enable);



	// 다른 플레이어들에 대한 이동을 수행
	for (auto& [objectID, pOtherPlayer] : pOtherPlayers) {
		XMINT3 prevIndex = pZone->GetIndex(pOtherPlayer->GetWorldPosition());
		pOtherPlayer->Animate(_timeElapsed);

		XMINT3 nextIndex = pZone->GetIndex(pOtherPlayer->GetWorldPosition());

		// 이전 인덱스와 비교해서 바뀌었다면 섹터를 바꾸어준다.
		if (prevIndex.x != nextIndex.x || prevIndex.y != nextIndex.y || prevIndex.z != nextIndex.z) {
			pZone->HandOffObject(SectorLayer::otherPlayer, pOtherPlayer->GetID(), pOtherPlayer, prevIndex, nextIndex);
		}
	}

	// 이동 수행후 해당 플레이어가 화면에 보이는지 판단
	pZone->SetVisiblePlayer(camera);

	for (auto& [objectID, pOtherPlayer] : pOtherPlayers) {
		
		shared_ptr<TextBox> nickname = pOtherPlayer->GetNickname();
		if (pOtherPlayer->GetVisible()) {
			XMFLOAT3 position = pOtherPlayer->GetBoundingBox().Center;
			position.y += pOtherPlayer->GetBoundingBox().Extents.y;
			// 뷰포트 좌표계 -1~1 -> UI좌표계 0~2    
			XMFLOAT2 pos = GetWorldToScreenCoord(position, camera->GetViewTransform(), camera->GetProjectionTransform());
			nickname->SetPosition(XMFLOAT2(pos.x + 1, pos.y + 1), true);
			nickname->SetEnable(true);
		}
		else {
			nickname->SetEnable(false);
		}
	}


	// 쿨타임 텍스트를 갱신해준다.
	if (isPlayerProfessor) {	// 교수
		auto pProfessor = static_pointer_cast<Professor>(pPlayer);
		if (pTexts["leftCoolTime"]->GetEnable()) {	// 왼쪽 스킬이 쿨타임일 경우
			if (pProfessor) {
				float coolTime = pProfessor->GetCoolTime(AttackType::swingAttack);
				if (coolTime <= 0.f) {	// 쿨타임이 끝난경우
					pTexts["leftCoolTime"]->SetEnable(false);
					pUIs["2DUI_swingAttack"]->SetDark(false);
				}
				else {
					if (coolTime >= 1.0f) { // 1초 이상일경우에는 정수로 출력한다.

						pTexts["leftCoolTime"]->SetText(to_wstring((int)coolTime));
					}
					else {
						pTexts["leftCoolTime"]->SetText(to_wstring(coolTime).substr(0, 3));
					}
				}
				
			}
		}
		if (pTexts["rightCoolTime"]->GetEnable()) {	// 왼쪽 스킬이 쿨타임일 경우
			if (pProfessor) {
				float coolTime = pProfessor->GetCoolTime(AttackType::throwAttack);
				if (coolTime <= 0.f) {	// 쿨타임이 끝난경우
					pTexts["rightCoolTime"]->SetEnable(false);
					pUIs["2DUI_throwAttack"]->SetDark(false);
				}
				else {
					if (coolTime >= 1.0f) { // 1초 이상일경우에는 정수로 출력한다.

						pTexts["rightCoolTime"]->SetText(to_wstring((int)coolTime));
					}
					else {
						pTexts["rightCoolTime"]->SetText(to_wstring(coolTime).substr(0, 3));
					}
				}

			}
		}

	}
	else {	// 학생


	}

	// zone 내 오브젝트들에 대한 animate를 수행
	pZone->UpdatePlayerSector();

	for (auto& pLight : pLights) {
		if (pLight) {
			pLight->UpdateLight();
		}
	}

	// 시간을 갱신한다.
	UpdateTimeText();
	remainTime -= _timeElapsed;

	pZone->AnimateObjects(_timeElapsed);
}

void PlayScene::ProcessSocketMessage(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList)
{
	GameFramework& gameFramework = GameFramework::Instance();

	// 고정길이의 패킷을 Recv받는다.
	static int recvByte = 0;

	int result = RecvFixedPacket(recvByte);
	if (result != SOCKET_ERROR) {
		recvByte += result;
		// 덜 받은 경우
		if (recvByte < BUFSIZE) {
			return;
		}
		// 모두 받았을 경우
		else {
			recvByte = 0;
		}
	}
	else {
		// EWOULDBLOCK
		return;
	}
	SC_PACKET_TYPE packetType = (SC_PACKET_TYPE)recvBuffer[0];
	
	switch (packetType) {
	case SC_PACKET_TYPE::playersInfo: {
		SC_PLAYERS_INFO* packet = GetPacket<SC_PLAYERS_INFO>();
		XMINT3 prevIndex, nextIndex;
		for (UINT i = 0; i < packet->nPlayer; ++i) {
			// 본인에 대한 정보일 경우 
			SC_PLAYER_INFO& pinfo = packet->playersInfo[i];
			if (pinfo.objectID == pPlayer->GetID())
				continue;
			// 받은 플레이어의 새 월드정보를 업데이트 해준다.
			shared_ptr<InterpolateMoveGameObject> pMoveClient = pOtherPlayers.find(pinfo.objectID)->second;
			prevIndex = pZone->GetIndex(pMoveClient->GetWorldPosition());
			pMoveClient->SetNextTransform(pinfo.position, pinfo.rotation, pinfo.scale);
			nextIndex = pZone->GetIndex(pMoveClient->GetWorldPosition());
			// 위치에 의해 섹터가 바뀌었다면 오브젝트를 옮겨준다.
			if (prevIndex.x != nextIndex.x || prevIndex.y != nextIndex.y || prevIndex.z != nextIndex.z) {
				pZone->HandOffObject(SectorLayer::otherPlayer, pMoveClient->GetID(), pMoveClient, prevIndex, nextIndex);
			}
		}
		break;
	}
	case SC_PACKET_TYPE::toggleDoor: {
		SC_TOGGLE_DOOR* packet = GetPacket<SC_TOGGLE_DOOR>();
		// 해당 오브젝트에 대한 상호작용을 한다.
		pZone->Interact(packet->objectID);
		break;
	}
	case SC_PACKET_TYPE::useWaterDispenser: {
		SC_USE_WATER_DISPENSER* packet = GetPacket<SC_USE_WATER_DISPENSER>();
		// 해당 오브젝트에 대한 상호작용을 한다.
		pZone->Interact(packet->waterDispenserObjectID);
		if (packet->playerObjectID == myObjectID) {	// 자신이 정수기를 사용했을 경우
			// 스테미너를 충전한다.
			pPlayer->SetMP(100.f);
		}
		break;
	}
	case SC_PACKET_TYPE::hackingRate: {
		// 어떤 플레이어가 해킹한 내용을 저장하고 사용중이지 않은 상태로 바꾸어준다.
		SC_HACKING_RATE* packet = GetPacket<SC_HACKING_RATE>();

		auto pComputer = ranges::find(pEnableComputers, packet->computerObjectID, &Computer::GetID);
		(*pComputer)->SetHackingRate(packet->rate);
		(*pComputer)->SetUse(0);
		break;
	}
	case SC_PACKET_TYPE::useComputer: {
		SC_USE_COMPUTER* packet = GetPacket<SC_USE_COMPUTER>();
		auto pComputer = ranges::find(pEnableComputers, packet->computerObjectID, &Computer::GetID);
		(*pComputer)->SetUse(packet->playerObjectID);

		// 내가 사용하게 될 경우
		if (packet->playerObjectID == myObjectID) {
			(*pComputer)->Interact();
		}
		break;
	}
	case SC_PACKET_TYPE::attack: {
		// 알맞은 섹터에 공격을 추가하고 본인이 공격한 경우 쿨타임을 적용한다.
		SC_ATTACK* packet = GetPacket<SC_ATTACK>();
		pZone->AddAttack(packet->attackType, packet->attackObjectID, FindPlayerObject(packet->playerObjectID), _pDevice, _pCommandList);
		if (packet->playerObjectID == myObjectID && isPlayerProfessor) {
			auto pProfessor = static_pointer_cast<Professor>(pPlayer);
			if(pProfessor) pProfessor->Reload(packet->attackType);
		}
		break;
	}
	case SC_PACKET_TYPE::hit: {
		SC_ATTACK_HIT* packet = GetPacket<SC_ATTACK_HIT>();
		// 내가 맞은 패킷은 받지 않는다.
		auto pHitPlayerObject = dynamic_pointer_cast<InterpolateMoveGameObject>(FindPlayerObject(packet->hitPlayerObjectID));
		if (pHitPlayerObject) {
			auto pAttack = pZone->GetAttack(packet->attackObjectID);
			if (!pAttack) break;
			pHitPlayerObject->AddHP(-pAttack->GetDamage());
		}
		else {
			cout << "해당 플레이어가 없습니다!!\n";
		}
		pZone->RemoveAttack(packet->attackObjectID);
		break;
		// 해당플레이어의 체력을 깎고, 해당 오브젝트를 삭제한다.
	}
	case SC_PACKET_TYPE::goPrison: {
		SC_GO_PRISON* packet = GetPacket<SC_GO_PRISON>();
		// 다른 플레이어(인터폴레이션 오브젝트)를 순간이동 시킨다.
		shared_ptr<InterpolateMoveGameObject> pTeleportPlayer = pOtherPlayers.find(packet->playerObjectID)->second;
		XMINT3 prevIndex = pZone->GetIndex(pTeleportPlayer->GetWorldPosition());
		pTeleportPlayer->SetNextTransform(prisonPosition, pTeleportPlayer->GetLocalRotate(), pTeleportPlayer->GetLocalScale());
		pTeleportPlayer->SetNextTransform(prisonPosition, pTeleportPlayer->GetLocalRotate(), pTeleportPlayer->GetLocalScale());
		XMINT3 nextIndex = pZone->GetIndex(pTeleportPlayer->GetWorldPosition());

		// 갇혀있는 상태로 바꾼다. + hp를 50으로 바꾼다.
		pTeleportPlayer->SetImprisoned(true);
		pTeleportPlayer->SetHP(50.f);

		// 위치에 의해 섹터가 바뀌었다면 핸드오프 해준다.
		if (prevIndex.x != nextIndex.x || prevIndex.y != nextIndex.y || prevIndex.z != nextIndex.z) {
			pZone->HandOffObject(SectorLayer::otherPlayer, pTeleportPlayer->GetID(), pTeleportPlayer, prevIndex, nextIndex);
		}
		break;
	}
	case SC_PACKET_TYPE::openPrisonDoor: {
		SC_OPEN_PRISON_DOOR* packet = GetPacket<SC_OPEN_PRISON_DOOR>();
		

		// 자신이 학생이고 자기 자신이 갇혀있다면 감옥탈출 위치로 순간이동 시킨다.
		if (!isPlayerProfessor) {
			auto pStudent = static_pointer_cast<Student>(pPlayer);
			// 본인이 문을 연것이라면 열쇠를 지운다.
			if (packet->openPlayerObjectID == myObjectID) {
				pStudent->SetItem(ObjectType::none);
				Scene::GetUI("2DUI_prisonKey")->SetEnable(false);
			}
			if (pStudent->GetImprisoned()) {	// 수감되어 있다면
				// 수감해제로 바꾼다.
				pStudent->SetImprisoned(false);
				// 순간이동 시킨다.
				pStudent->SetLocalPosition(prisonExitPosition);
				pStudent->UpdateObject();
			}
		}
		// 다른 학생의 수감 상태를 변경시키고 순간이동 시킨다.
		for (auto& [objectID, pOtherPlayer] : pOtherPlayers) {
			if (objectID != professorObjectID && pOtherPlayer->GetImprisoned()) {
				pOtherPlayer->SetImprisoned(false);	// 수감 해제
				// 순간이동 시킨다.
				XMINT3 prevIndex = pZone->GetIndex(pOtherPlayer->GetWorldPosition());
				pOtherPlayer->SetNextTransform(prisonExitPosition, pOtherPlayer->GetLocalRotate(), pOtherPlayer->GetLocalScale());
				pOtherPlayer->SetNextTransform(prisonExitPosition, pOtherPlayer->GetLocalRotate(), pOtherPlayer->GetLocalScale());
				XMINT3 nextIndex = pZone->GetIndex(pOtherPlayer->GetWorldPosition());

				// 위치에 의해 섹터가 바뀌었다면 핸드오프 해준다.
				if (prevIndex.x != nextIndex.x || prevIndex.y != nextIndex.y || prevIndex.z != nextIndex.z) {
					pZone->HandOffObject(SectorLayer::otherPlayer, pOtherPlayer->GetID(), pOtherPlayer, prevIndex, nextIndex);
				}
			}
		}

		break;
	}
	case SC_PACKET_TYPE::toggleLever: {
		SC_LEVER_TOGGLE* packet = GetPacket<SC_LEVER_TOGGLE>();
		
		pZone->SetAllComputerPower(packet->allLeverPowerOn);
		if (packet->allLeverPowerOn) {
			globalAmbient = XMFLOAT4(0.5, 0.5, 0.5, 1.0);
			AllLeverPowerOn = true;
			if (isPlayerProfessor)
				static_pointer_cast<Professor>(pPlayer)->SetSabotageCoolTime(60.f);
		}
		else {
			globalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);
			AllLeverPowerOn = false;
		}
			

		pZone->Interact(packet->leverObjectID);
		// 교수일 경우 쿨타임 초기화해준다.
		if (!packet->setPower && isPlayerProfessor) {
			dynamic_pointer_cast<Professor>(pPlayer)->SetSabotageCoolTime(10.f);
		}
		break;
	}
	case SC_PACKET_TYPE::addItem: { // 특정위치에 아이템이 추가된 경우
		SC_ADD_ITEM* packet = GetPacket<SC_ADD_ITEM>();
		
		XMFLOAT3 position = itemSpawnLocationPositions[packet->itemLocationIndex];

		pZone->AddItem(packet->objectType, packet->itemLocationIndex, packet->itemObjectID, position, _pDevice, _pCommandList);
		break;
	}
	case SC_PACKET_TYPE::removeItem: {	// 특정위치에 아이템이 사라진 경우
		SC_REMOVE_ITEM* packet = GetPacket<SC_REMOVE_ITEM>();
		pZone->RemoveItem(packet->itemObjectID);
		break;
	}
	case SC_PACKET_TYPE::useItem: {	// 누군가 의료키트, 트랩을 설치한 경우
		SC_USE_ITEM* packet = GetPacket<SC_USE_ITEM>();
		auto pStudent = static_pointer_cast<Student>(FindPlayerObject(packet->playerObjectID));
		if (packet->objectType == ObjectType::medicalKitItem) {
			pStudent->AddHP(50.0f);
		}
		if (packet->objectType == ObjectType::trapItem) {
			XMFLOAT3 position = pStudent->GetWorldPosition();
			shared_ptr<Trap> pTrap = make_shared<Trap>();

			pTrap->Create("Trap_attack", _pDevice, _pCommandList);
			pTrap->SetLocalPosition(position);
			pTrap->SetID(packet->itemObjectID);
			pTrap->UpdateObject();
			pZone->AddTrap(packet->itemObjectID, pTrap);

		}
		break;
	}
	case SC_PACKET_TYPE::removeTrap: {
		SC_REMOVE_TRAP* packet = GetPacket<SC_REMOVE_TRAP>();
		shared_ptr<Trap> pTrap = pZone->GetTrap(packet->trapObjectID);

		pZone->RemoveTrap(packet->trapObjectID);
		break;
	}
	case SC_PACKET_TYPE::exitPlayer: {
		SC_EXIT_PLAYER* packet = GetPacket<SC_EXIT_PLAYER>();
		shared_ptr<InterpolateMoveGameObject> pOtherPlayer = pOtherPlayers[packet->playerObjectID];
		pZone->RemoveObject(SectorLayer::otherPlayer, packet->playerObjectID, pZone->GetIndex(pOtherPlayer->GetWorldPosition()));
		pOtherPlayers.erase(packet->playerObjectID);
		exit = true;
		break;
	}
	case SC_PACKET_TYPE::professorWin: {
		SC_PROFESSOR_WIN* packet = GetPacket<SC_PROFESSOR_WIN>();
		professorWin = true;
		break;
	}   
	case SC_PACKET_TYPE::escapeProfessor: {
		SC_ESCAPE_PROFESSOR* packet = GetPacket<SC_ESCAPE_PROFESSOR>();
		exit = true;
		break;
	}
	case SC_PACKET_TYPE::escapeStudent: {
		SC_ESCAPE_STUDENT* packet = GetPacket<SC_ESCAPE_STUDENT>();
		shared_ptr<InterpolateMoveGameObject> pOtherPlayer = pOtherPlayers[packet->escapeObjectID];
		cout << format("escapeObjectID : {}\n", packet->escapeObjectID);
		pZone->RemoveObject(SectorLayer::otherPlayer, packet->escapeObjectID, pZone->GetIndex(pOtherPlayer->GetWorldPosition()));
		pOtherPlayers.erase(packet->escapeObjectID);
		break;
	}
	default:
		cout << "나머지 패킷. 타입 = " << (int)packetType << "\n";
	}

}

void PlayScene::UpdateLightShaderVariables(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	// light를 플레이어기준으로 같은층 (비슷한 높이)에 있는 것들중 가장 가까운것부터 20개 그린다.
	XMFLOAT3 playerPos = pPlayer->GetWorldPosition();

	auto IsInside = [](float lightPosY, float playerPosY) {
		return playerPosY <= lightPosY && lightPosY <= playerPosY + 3.f;
	};
	auto Pred = [playerPos, IsInside](const shared_ptr<Light>& a, const shared_ptr<Light>& b) {
		bool isInsideA = IsInside(a->position.y, playerPos.y);
		bool isInsideB = IsInside(b->position.y, playerPos.y);
		// 조명의 위치가 둘다 범위안에 있거나, 둘다 범위밖에 있을 경우
		if (isInsideA == isInsideB) {
			// 거리가 가까운것부터 오름차순으로 정렬
			return Vector3::LengthEst(playerPos, a->position) < Vector3::LengthEst(playerPos, b->position);
		}
		return isInsideA;
	};
	ranges::sort(pLights, Pred);
	//cout << "빛 : " << pLights[0]->position << "\n";
	
	int nLight = (UINT)pLights.size();

	for (int i = 0; i < min(nLight, MAX_LIGHTS); ++i) {
		memcpy(&pMappedLights->lights[i], pLights[i].get(), sizeof(Light));
	}

	memcpy(&pMappedLights->globalAmbient, &globalAmbient, sizeof(XMFLOAT4));
	memcpy(&pMappedLights->nLight, &nLight, sizeof(int));

	D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = pLightsBuffer->GetGPUVirtualAddress();
	_pCommandList->SetGraphicsRootConstantBufferView(2, gpuVirtualAddress);

}

shared_ptr<GameObject> PlayScene::FindPlayerObject(UINT _objectID) const {
	shared_ptr<GameObject> pPlayerObject = nullptr;

	if (_objectID == myObjectID) {
		pPlayerObject = pPlayer;
	}
	else {
		for (auto& [objectID, pOtherPlayer] : pOtherPlayers) {
			if (_objectID == objectID) {
				pPlayerObject = pOtherPlayer;
				break;
			}
		}
	}
	return pPlayerObject;
}

UINT PlayScene::GetProfessorObjectID() const {
	return professorObjectID;
}

void PlayScene::AddItemSpawnLocation(const XMFLOAT3& _position) {
	itemSpawnLocationPositions.push_back(_position);
}

void PlayScene::SetExitBox(const BoundingBox& _exitBox) {
	exitBox = _exitBox;
}

void PlayScene::ReActButton(shared_ptr<Button> _pButton) {

}

void PlayScene::ProcessMouseInput(UINT _type, XMFLOAT2 _pos) {

	GameFramework& gameFramework = GameFramework::Instance();
	Scene::ProcessMouseInput(_type, _pos);
	switch (_type) {
	case WM_LBUTTONDOWN:
		ReleaseCapture();
		pPlayer->LeftClick();
		break;
	case WM_LBUTTONUP:
		SetCapture(hWnd);
		gameFramework.InitOldCursor();
		break;
	case WM_RBUTTONDOWN:
		// 투사체 공격
		pPlayer->RightClick();


		break;
}
}

void PlayScene::ProcessCursorMove(XMFLOAT2 _delta)  {
	
	shared_ptr<Camera> pCamera = pPlayer->GetCamera();
	if (_delta.x != 0.0f) {
		//XMFLOAT3 upVector = pPlayer->GetCamera()->GetLocalUpVector();
		pCamera->SynchronousRotation(XMFLOAT3(0, 1, 0), _delta.x / 3.f);
	}
	
	XMFLOAT3 rightVector = pCamera->GetLocalRightVector();
	float angle = Vector3::Angle(pCamera->GetWorldUpVector(), XMFLOAT3(0, 1, 0));

	// 카메라의 y방향 시선을 제한.
	if (angle > 40.0f) {
		// 카메라가 위를 보고있을 경우
		if (pCamera->GetWorldLookVector().y > 0) {
			pCamera->SynchronousRotation(rightVector, angle / 300.f);
		}
		else {
			pCamera->SynchronousRotation(rightVector, - angle / 300.f);
		}
	}
	else if (_delta.y != 0.0f) {
		pCamera->SynchronousRotation(rightVector, _delta.y / 3.f);
	}
	pCamera->UpdateLocalTransform();

	//pCamera->UpdateLocalTransform();

	// 적용
	pPlayer->UpdateObject();
}


void PlayScene::RenderShadowMap(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, UINT _lightIndex) {

	GameFramework& gameFramework = GameFramework::Instance();

	shared_ptr<Light> pLight = pLights[_lightIndex];
	// 해당 빛 관점에서의 카메라로 set해준다.
	VS_CameraMappedFormat format;

	format.view = pLight->viewTransform;
	format.projection = pLight->projectionTransform;
	format.position = pLight->position;
	// 카메라를 해당 빛의 정보로 바꾼 후 렌더링을 한다.
	camera->UpdateShaderVariable(_pCommandList, &format);

	// 쉐이더를 연결

	// 그림자에 영향을 주는 오브젝트들을 그린다. basic, instancing, effect..

}

void PlayScene::PreRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed) {

	GameFramework& gameFramework = GameFramework::Instance();
	// 프레임워크에서 렌더링 전에 루트시그니처를 set
	camera->SetViewPortAndScissorRect(_pCommandList);
	camera->UpdateShaderVariable(_pCommandList);

	UpdateLightShaderVariables(_pCommandList);
	// 쉐이더 클래스에 정적으로 정의된 디스크립터 힙을 연결한다.

	Shader::SetDescriptorHeap(_pCommandList);

	gameFramework.GetShader("SkyBoxShader")->PrepareRender(_pCommandList);
	pSkyBox->Render(_pCommandList);


	pZone->Render(_pCommandList, pPlayer->GetCamera()->GetBoundingFrustum());

	gameFramework.GetShader("EffectShader")->Render(_pCommandList);

	// 다 그린 후 G버퍼를 쉐이더에 연결한다.
	gameFramework.GetGBuffer()->UpdateShaderVariable(_pCommandList);
}

void PlayScene::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed) {

	GameFramework& gameFramework = GameFramework::Instance();

	camera->UpdateShaderVariable(_pCommandList);
	UpdateLightShaderVariables(_pCommandList);

	// 탈출하게 되면 화면이 흰색이 되면서 완전 흰색이 되면 서버에 탈출했다는 패킷을 보내면서 게임이 끝난다.
	if (exit && fadeOut < 3) {
		fadeOut += _timeElapsed;
		if(isPlayerProfessor)
			globalAmbient = Vector4::Add(XMFLOAT4(0.5, 0.5, 0.5, 0), Vector4::Multiply(fadeOut, XMFLOAT4(-1, -1, -1, 1)));
		else
			globalAmbient = Vector4::Add(XMFLOAT4(0.5, 0.5, 0.5, 0), Vector4::Multiply(fadeOut, XMFLOAT4(5, 5, 5, 5)));
	}

	// 교수가 모든 학생을 감옥에 가둘경우 교수만 화면이 흰색으로 되면서 게임이 끝난다.
	if (professorWin && fadeOut < 3) {
		fadeOut += _timeElapsed;
		if (isPlayerProfessor)
			globalAmbient = Vector4::Add(XMFLOAT4(0.5, 0.5, 0.5, 0), Vector4::Multiply(fadeOut, XMFLOAT4(5, 5, 5, 5)));
		else
			globalAmbient = Vector4::Add(XMFLOAT4(0.5, 0.5, 0.5, 0), Vector4::Multiply(fadeOut, XMFLOAT4(-1, -1, -1, 1)));
	}

	gameFramework.GetShader("LightingShader")->PrepareRender(_pCommandList);
	pFullScreenObject->Render(_pCommandList);

	// 빛 처리를 끝낸 후 UI를 그린다.
	gameFramework.GetShader("UIShader")->PrepareRender(_pCommandList);
	for (auto [name, pUI] : pUIs) {
		pUI->Render(_pCommandList);
	}
	for (auto [name, pButton] : pButtons) {
		pButton->Render(_pCommandList);
	}

	// 반투명한 오브젝트를 마지막에 그린다.
	gameFramework.GetShader("BlendingShader")->Render(_pCommandList);

	//gameFramework.GetShader("BoundingMeshShader")->PrepareRender(_pCommandList);
	//pFrustumMesh->UpdateMesh(camera->GetBoundingFrustum());
	//pFrustumMesh->Render(_pCommandList);
}

void PlayScene::PostRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	// 글자는 Render과정이 전부 끝난 후에 가능
	for (auto [name, pButton] : pButtons) {
		pButton->PostRender();
	}

	for (auto [name, pText] : pTexts) {
		pText->Render();
	}
	for (auto [objectID, pOtherPlayer] : pOtherPlayers) {
		pOtherPlayer->GetNickname()->Render();
	}
}

void PlayScene::AddLight(const shared_ptr<Light>& _pLight) {
	pLights.push_back(_pLight);
}

