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
			if (pButton->CheckEnable() && pButton->CheckClick(_pos)) { // ��ư�� Ŭ���Ǿ��� ���
				pButton->Press(true, _pos);

				break;
			}
		}
		break;
	case WM_LBUTTONUP:
		for (auto [name, pButton] : pButtons) {
			if (pButton->CheckEnable()) {
				// �����鼭 ��ư�� ���������� Ȯ��. �ƴ϶�� pressed�� false�� �ٲ���
				if (pButton->Press(false, _pos)) {
					ReActButton(pButton);	// �÷��̾ ��ư�� �����ٸ� ��ư�� ���� ���� ����
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

	shared_ptr<TextBox> pText = make_shared<TextBox>((WCHAR*)L"�޸յ���ü", D2D1::ColorF(1, 1, 1, 1), XMFLOAT2(0.9f, 1.3f), XMFLOAT2(0.2f, 0.2f), C_WIDTH / 40.0f, false);
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
	case SC_PACKET_TYPE::giveClientID: { // ó�� ���ӽ� �÷��̾� cid�� �ο��޴� ��Ŷ
		SC_GIVE_CLIENT_ID packet;
		RecvContents(packet);
		cid = packet.clientID;
		break;
	}
	case SC_PACKET_TYPE::roomListInfo: {
		SC_ROOMLIST_INFO packet;
		RecvContents(packet);

		// Roomlist �� nRoom�� �� ��ŭ SC_SUB_ROOMLIST_INFO ��Ŷ�� �߰��� �Ѳ����� �޴´�.
		roomList.clear();
		roomList.resize(packet.nRoom);
		recv(server_sock, (char*)roomList.data(), sizeof(SC_SUB_ROOMLIST_INFO) * packet.nRoom, 0);
		UpdateRoomText();
		break;
	}	// ���� ���� ����Ʈ�� �޴� ��Ŷ
	case SC_PACKET_TYPE::roomPlayersInfo: { // ���� ���Ǹ� ������ �� �� ������ �����ϴٰ� ����
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
	case SC_PACKET_TYPE::fail: { 	// ���� �����߰ų�, �����ų� �����Ǿ� �� ���� ������ ���
		// ����
		SC_FAIL packet;
		RecvContents(packet);
		
		break;
	}
	case SC_PACKET_TYPE::ready: {	// ������ �غ� ������ ��� 
		SC_READY packet;
		RecvContents(packet);

		// �ش� cid�� ���� �÷��̾ ã�� ������¸� �ݴ�� �ٲپ��ش�.
		auto pindex = roomInfo.findPlayerIndex(packet.readyClientID);
		pindex->ready = !pindex->ready;
		UpdateReadyState();
		break;
	}
	case SC_PACKET_TYPE::roomVisitPlayerInfo: {	// ���� �濡 ���� ���
		SC_ROOM_VISIT_PLAYER_INFO packet;
		RecvContents(packet);

		// �� ������ �ش� �÷��̾��� ������ �߰��Ѵ�.
		Player_Info pi{ packet.visitClientID, false };
		roomInfo.players.push_back(pi);
		roomInfo.nParticipant++;
		UpdateReadyState();
		break;
	}
	case SC_PACKET_TYPE::roomOutPlayerInfo: { // ���� �濡�� ���� ���
		SC_ROOM_OUT_PLAYER_INFO packet;
		RecvContents(packet);

		// �ش� Ŭ���̾�Ʈ�� ã�� ����� ������ �ٲ���ٸ� ���� �Ӹ��Ѵ�.
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

		// ������ ���۵� ��� ���� ���ӿ��� ���� �ν��Ͻ� ����, �޽�, �ִϸ��̼�, �ؽ�ó ���� ������ �ε��Ѵ�.
		gameFramework.LoadingScene(loadingScene);

		// �÷��� �� �ε��� ��� �Ϸ�� ��� �������� �ε� �Ϸ� ��Ŷ�� ������. 
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

		cout << "������ ��Ŷ. Ÿ�� = " << (int)packetType << "\n";
	}
}

void LobbyScene::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed) {
	GameFramework& gameFramework = GameFramework::Instance();

	_pCommandList->RSSetViewports(1, &viewPort);
	_pCommandList->RSSetScissorRects(1, &scissorRect);

	// �ؽ�ó�� ����ִ� ��ũ���� ���� �����Ѵ�.
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
	// ���ڴ� Render������ ���� ���� �Ŀ� ����
	for (auto [name, pText] : pTexts) {
		pText->Render();
	}
}

void LobbyScene::ReActButton(shared_ptr<Button> _pButton) { // ���� ��ư�� ���� ���
	ButtonType type = _pButton->GetType();
	switch (type) {
	case ButtonType::start: {
		// �������� CS_QUERY_ROOMLIST_INFO ��Ŷ�� ���� ���� �� ����Ʈ�� �����޶�� ��û�Ѵ�.
		CS_QUERY_ROOMLIST_INFO sPacket;
		sPacket.cid = cid;
		send(server_sock, (char*)&sPacket, sizeof(CS_QUERY_ROOMLIST_INFO), 0);

		changeUI(LobbyState::title, false);
		changeUI(LobbyState::roomList, true);
		break;
	}
	case ButtonType::makeRoom: {

		// ��Ŷ�� ���� ��, �� ������ �׻� �����ϹǷ� �������� ���� ��Ŷ�� ���� �ʰ�
		// ������ ���� ������ �����Ѵ�.
		CS_MAKE_ROOM sPacket;
		sPacket.hostID = cid;
		send(server_sock, (char*)&sPacket, sizeof(CS_MAKE_ROOM), 0);

		// �� ������ ��� �� ������ �������� �� �ϳ��� �����.
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
		// ���� �� �濡�� �����ٴ� ����� �˷��ִ� ��Ŷ�� ����.
	// �������� �˾Ƽ� �� ��Ŷ�� �޾� �� ����Ʈ�� �ٽ� �����ش�.
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
		// ���� �� ��ȣ�� ������ ���� ���� �������� �����.
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
		// �� �ȿ� ������ Ŭ���̾�Ʈ ���� �� �� ������ ��ư�� ���������� ���� ���� ����
		CS_OUT_ROOM sPacket;
		sPacket.cid = cid;
		send(server_sock, (char*)&sPacket, sizeof(CS_QUERY_ROOMLIST_INFO), 0);
	}
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          
void LobbyScene::changeUI(LobbyState _state, bool _active) {
	// �κ���� �� ���� ���¿� ���� ��ư, ��� �̹����� �����Ѵ�.  
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
	// ���� ������ ���� �� ����Ʈ�� ������ ���� ���ش�.
	bool lastRoom = false;
	UINT startIndex = 1 + (roomPage - 1) * 6;
	for (UINT i = startIndex; i < startIndex + 6; ++i) {

		UINT participant = 0;
		RoomState state = RoomState::none;
		string baseName = "RoomButton_" + to_string(i - startIndex + 1);

		// �� ������ ���� ĭ�� ��� 
		if (roomList.size() < i) {
			state = RoomState::none;
			lastRoom = true; // ���Ŀ� ������ ����� ��� �� ������ ǥ���Ѵ�.
		}
		UINT roomID = 0;
		if (!lastRoom) {
			SC_SUB_ROOMLIST_INFO packet = roomList[i - 1];
			roomID = packet.roomID;

			participant = packet.nParticipant;
			if (packet.started) { // �̹� ������ ���
				state = RoomState::started;
			}
			else if (packet.nParticipant == 5) {	// ������ �� �� ���
				state = RoomState::full;
			}
			else state = RoomState::joinable;
		}
		reinterpret_cast<RoomButton*>(pButtons[baseName].get())->UpdateState(roomID, participant ,state);
	}
}

void LobbyScene::UpdateReadyState() {

	// ���� �濡 ui, ��ư ���µ��� ����
	bool bChange = false;
	for (int i = 0; i < 5; ++i) {
		pUIs["2DUI_ready_" + to_string(i + 1)]->SetEnable(false);
	}
	for (UINT i = 0; i < roomInfo.nParticipant; ++i) {
		// ������ ���
		if (roomInfo.host == roomInfo.players[i].clientID) {
			pUIs["2DUI_ready_" + to_string(i + 1)]->SetTexture("2DUI_host");
			pUIs["2DUI_ready_" + to_string(i + 1)]->SetEnable(true);
			// ������ ������ ���
			if (cid == roomInfo.players[i].clientID) {
				pButtons["gameStartButton"]->SetTexture("2DUI_startButton");
				bChange = true;
			}
		}
		// �غ������ ��� 
		else if (roomInfo.players[i].ready) {
			pUIs["2DUI_ready_" + to_string(i + 1)]->SetTexture("2DUI_ready");
			pUIs["2DUI_ready_" + to_string(i + 1)]->SetEnable(true);
			// ������ �غ������ ���
			if (cid == roomInfo.players[i].clientID) {
				pButtons["gameStartButton"]->SetTexture("2DUI_readyCancelButton");
				bChange = true;
			}
		}

	}
	// �غ���°� �ƴҰ��
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

	// �÷��̾��� OOBB�� y�������� �̵����� �� �� �浹üũ�� �����Ѵ�.
	if (!collideObj) {
		result += 1;
	}
	else {
		// �ٴ��� ���ٸ� ���� �����Ѵ�.
		if(!pPlayer->GetFloor()) pPlayer->SetFloor(collideObj);
	}


	// �÷��̾��� OOBB�� x,z�������� �̵����� �� �� �浹üũ�� �����Ѵ�.
	collideObj = pZone->CheckCollisionHorizontal(pPlayer->GetFloor());
	if (collideObj == 0) {
		result += 2;
	}


	/*if (Vector4::IsSame(rot, Vector4::QuaternionIdentity())) {
		result += 4;
	} 
	else */
	{

		// �÷��̾��� OOBB�� ȸ�����Ѻ� �� �浹üũ�� �����Ѵ�.
		vector<shared_ptr<GameObject>> collideObjs = pZone->CheckCollisionRotate(pPlayer->GetFloor());
		if (collideObjs.size() == 0) {
			result += 4;
		}
		else {
			// �ε��� ��� 
			
			XMFLOAT3 knockBack = XMFLOAT3();
			XMFLOAT3 lookVector, direcVector;

			for (auto& collideObj : collideObjs) {
				// ��ü�� �躤�Ϳ� �� OOBB�� ������ ���� ���� �躤�Ͱ� �ݴ��ʿ� ������� -1�� �����ش�.
				
				lookVector = collideObj->GetWorldLookVector();
				direcVector = Vector3::Subtract(pPlayer->GetBoundingBox().Center, collideObj->GetBoundingBox().Center);
				direcVector.y = 0;
				if (Vector3::Angle(direcVector, lookVector) > 90.0f) {
					lookVector = Vector3::ScalarProduct(lookVector, -1.f);
				}
				knockBack = Vector3::Add(knockBack, Vector3::ScalarProduct(Vector3::Normalize(lookVector), 0.01f));
			}
			// �ε��� ������Ʈ���� �躤�� ������� ��� �� �������� �о��ش�.
			pPlayer->SetKnockBack(knockBack);
		}
	}
	
	return result;
}


void PlayScene::Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();

	// Zone�� ���� �� �������� �о� ������Ʈ���� �ε��Ѵ�.
	pZone = make_shared<Zone>(XMFLOAT3(100.f, 100.f, 100.f), XMINT3(10, 10, 10), shared_from_this());
	pZone->LoadZoneFromFile(_pDevice, _pCommandList);

	// ���� �߰�
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
	UINT ncbElementBytes = ((sizeof(LightsMappedFormat) + 255) & ~255); //256�� ���
	pLightsBuffer = ::CreateBufferResource(_pDevice, _pCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, temp);
	pLightsBuffer->Map(0, NULL, (void**)&pMappedLights);

	SkinnedGameObject::InitSkinnedWorldTransformBuffer(_pDevice, _pCommandList);	// skinnedObject�� �����ϱ� ���� (���庯ȯ����� ���)���ҽ��� �����Ѵ�.

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

	// ����
	pPlayer->UpdateObject();
}


void PlayScene::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed) {

	GameFramework& gameFramework = GameFramework::Instance();

	// �����ӿ�ũ���� ������ ���� ��Ʈ�ñ״�ó�� set
	camera->SetViewPortAndScissorRect(_pCommandList);
	camera->UpdateShaderVariable(_pCommandList);

	UpdateLightShaderVariables(_pCommandList);
	// ���̴� Ŭ������ �������� ���ǵ� ��ũ���� ���� �����Ѵ�.

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

