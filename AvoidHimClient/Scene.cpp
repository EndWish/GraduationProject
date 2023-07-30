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

char Scene::CheckCollision(float _timeElapsed) {
	return 0;
}

void Scene::LightingRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed) {
}


void Scene::PostRender()
{
}

void Scene::NoticeCloseToServer() {
}

//////////////////////////

LobbyScene::LobbyScene()
{
	roomPage = 1;
	currState = LobbyState::title;
	viewPort = { 0,0, (float)C_WIDTH, (float)C_HEIGHT, 0, 1 };
	scissorRect = { 0,0, C_WIDTH, C_HEIGHT };
	roomList.resize(6);
	isLoading = false;
	for (int i = 0; i < 5; ++i) {
		roomViewPort[i].Width = 0.2 * C_WIDTH;
		roomViewPort[i].Height = 0.5 * C_HEIGHT;

		roomViewPort[i].TopLeftX = (float)i * C_WIDTH * 0.195f + C_WIDTH * 0.01f;
		roomViewPort[i].TopLeftY = 0.2 * C_HEIGHT;
		roomViewPort[i].MinDepth = 0.f;
		roomViewPort[i].MaxDepth = 1.f;
	}
	globalAmbient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
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

	gameFramework.GetSoundManager().Play("noneSound");
	pUIs["2DUI_title"] = make_shared<Image2D>("2DUI_title", XMFLOAT2(2.f, 2.f), XMFLOAT2(0.f,0.f), XMFLOAT2(1.f,1.f), _pDevice, _pCommandList, false);
	pUIs["2DUI_title"]->SetDepth(1.f);

	pUIs["2DUI_roomBG"] = make_shared<Image2D>("2DUI_roomBG", XMFLOAT2(2.f, 2.f), XMFLOAT2(0.f, 0.f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
	pUIs["2DUI_roomBG"]->SetDepth(1.f);
	
	pUIs["2DUI_roomListBG"] = make_shared<Image2D>("2DUI_roomListBG", XMFLOAT2(2.f, 2.f), XMFLOAT2(0.f, 0.f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
	pUIs["2DUI_roomListBG"]->SetDepth(1.f);
	SetBackGround("2DUI_title");

	pUIs["2DUI_ready_1"] = make_shared<Image2D>("2DUI_ready", XMFLOAT2(0.285f, 0.142f), XMFLOAT2(0.08f, 1.58f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
	pUIs["2DUI_ready_2"] = make_shared<Image2D>("2DUI_ready", XMFLOAT2(0.285f, 0.142f), XMFLOAT2(0.47f, 1.58f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
	pUIs["2DUI_ready_3"] = make_shared<Image2D>("2DUI_ready", XMFLOAT2(0.285f, 0.142f), XMFLOAT2(0.856f, 1.58f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
	pUIs["2DUI_ready_4"] = make_shared<Image2D>("2DUI_ready", XMFLOAT2(0.285f, 0.142f), XMFLOAT2(1.244f, 1.58f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
	pUIs["2DUI_ready_5"] = make_shared<Image2D>("2DUI_ready", XMFLOAT2(0.285f, 0.142f), XMFLOAT2(1.632f, 1.58f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);


	pCamera = make_shared<Camera>();
	pCamera->Create(_pDevice, _pCommandList);
	pCamera->UpdateProjectionTransform(0.1f, 10.f, 0.5f, 60.0f);
	pCamera->SetLocalPosition(XMFLOAT3(0.f, 0.8f, 1.5f));
	pCamera->SetLocalRotation(Vector4::QuaternionRotation(XMFLOAT3(0, 1, 0), 180.0f));
	pCamera->UpdateObject();

	pLight = make_shared<Light>();
	pLight->lightType = 3;
	pLight->direction = XMFLOAT3(0.f, -0.5f, -0.1f);

	for (auto& pRoomPlayerObject : pRoomPlayerObjects) {
		pRoomPlayerObject = make_shared<RoomPlayerObject>();
		pRoomPlayerObject->Create("Student"s, _pDevice, _pCommandList);
		pRoomPlayerObject->GetAniController()->ChangeClip("idle");
	}

	pButtons["startButton"] = make_shared<Button>("2DUI_startButton", XMFLOAT2(0.3f, 0.2f), XMFLOAT2(1.5f, 1.35f), ButtonType::start, _pDevice, _pCommandList);;

	//pButtons["optionButton"] = make_shared<Button>("2DUI_optionButton", XMFLOAT2(0.3f, 0.2f), XMFLOAT2(1.5f, 1.4f), ButtonType::option, _pDevice, _pCommandList);;

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

	shared_ptr<TextBox> pText = make_shared<TextBox>((WCHAR*)L"Who asks Satan", D2D1::ColorF(1, 1, 1, 1), XMFLOAT2(0.9f, 1.3f), XMFLOAT2(0.2f, 0.2f), C_WIDTH / 40.0f, false);
	pTexts["pageNum"] = pText;

	pTexts["inRoomName1"] = make_shared<TextBox>((WCHAR*)L"LeeSeoyun", D2D1::ColorF(1, 1, 1, 1), XMFLOAT2(0.08f, 1.38f), XMFLOAT2(0.285f, 0.142f), C_WIDTH / 40.0f, false);
	pTexts["inRoomName2"] = make_shared<TextBox>((WCHAR*)L"LeeSeoyun", D2D1::ColorF(1, 1, 1, 1), XMFLOAT2(0.47f, 1.38f), XMFLOAT2(0.285f, 0.142f), C_WIDTH / 40.0f, false);
	pTexts["inRoomName3"] = make_shared<TextBox>((WCHAR*)L"LeeSeoyun", D2D1::ColorF(1, 1, 1, 1), XMFLOAT2(0.856f, 1.38f), XMFLOAT2(0.285f, 0.142f), C_WIDTH / 40.0f, false);
	pTexts["inRoomName4"] = make_shared<TextBox>((WCHAR*)L"LeeSeoyun", D2D1::ColorF(1, 1, 1, 1), XMFLOAT2(1.244f, 1.38f), XMFLOAT2(0.285f, 0.142f), C_WIDTH / 40.0f, false);
	pTexts["inRoomName5"] = make_shared<TextBox>((WCHAR*)L"LeeSeoyun", D2D1::ColorF(1, 1, 1, 1), XMFLOAT2(1.632f, 1.38f), XMFLOAT2(0.285f, 0.142f), C_WIDTH / 40.0f, false);
	
	
	pTexts["fps"] = make_shared<TextBox>((WCHAR*)L"����ü", D2D1::ColorF(1, 1, 1, 1), XMFLOAT2(0.f, 0.f), XMFLOAT2(0.285f, 0.142f), C_WIDTH / 100.0f, fpsToggle);

	Computer::InitMaterials(_pDevice, _pCommandList);

	////////////////////////////////////

	ComPtr<ID3D12Resource> temp;
	UINT ncbElementBytes = ((sizeof(LightsMappedFormat) + 255) & ~255); //256�� ���
	pLightsBuffer = ::CreateBufferResource(_pDevice, _pCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, temp);
	pLightsBuffer->Map(0, NULL, (void**)&pMappedLights);

	pButtons["gameStartButton"] = make_shared<Button>("2DUI_startButton", XMFLOAT2(0.3f, 0.2f), XMFLOAT2(1.25f, 1.7f), ButtonType::gameStart, _pDevice, _pCommandList, false);
	pButtons["quitRoomButton"] = make_shared<Button>("2DUI_quitRoomButton", XMFLOAT2(0.3f, 0.2f), XMFLOAT2(1.6f, 1.7f), ButtonType::quitRoom, _pDevice, _pCommandList, false);

	// ��� ���
	gameFramework.GetSoundManager().Play("introMusic", true);
}

void LobbyScene::ReleaseUploadBuffers() {

}

void LobbyScene::ProcessKeyboardInput(const array<bool, 256>& _keyDownBuffer, const array<UCHAR, 256>& _keysBuffers, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	if (_keyDownBuffer['F']) {
		fpsToggle = !fpsToggle;
		pTexts["fps"]->SetEnable(fpsToggle);
	}
}

void LobbyScene::AnimateObjects(char _collideCheck, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList)  {
	for (auto& pRoomPlayerObject : pRoomPlayerObjects) {
		pRoomPlayerObject->Animate(_timeElapsed);
	}
	GameFramework& gameFramework = GameFramework::Instance();
	if (fpsToggle) {
		wstring fpsText = L"FPS : " + to_wstring(gameFramework.GetFPS());
		pTexts["fps"]->SetText(fpsText.c_str());
	}
}

void LobbyScene::ProcessSocketMessage(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();

	// ���������� ��Ŷ�� Recv�޴´�.
	static int recvByte = 0;

	int result = RecvFixedPacket(recvByte);
	if (result != SOCKET_ERROR) {
		recvByte += result;
		// �� ���� ���
		if (recvByte < BUFSIZE) {
			return;
		}
		// ��� �޾��� ���
		else {
			recvByte = 0;
		}
	}
	else {
		// EWOULDBLOCK
		return;
	}
	
	
	// ù����Ʈ�� �о� ��Ŷ Ÿ���� �˾Ƴ���.
	SC_PACKET_TYPE packetType = (SC_PACKET_TYPE)recvBuffer[0];

	switch (packetType) {

	case SC_PACKET_TYPE::roomListInfo: {
		SC_ROOMLIST_INFO* packet = GetPacket<SC_ROOMLIST_INFO>();
		// Roomlist �� nRoom�� �� ��ŭ SC_SUB_ROOMLIST_INFO ��Ŷ�� �߰��� �Ѳ����� �޴´�.

		for (int i = 0; i < 6; ++i) {
			roomList[i] = packet->roomInfo[i];
		}
		//recv(server_sock, (char*)roomList.data(), sizeof(SC_SUB_ROOMLIST_INFO) * packet.nRoom, 0);
		UpdateRoomText();
		break;
	}	// ���� ���� ����Ʈ�� �޴� ��Ŷ
	case SC_PACKET_TYPE::roomPlayersInfo: { // ���� ���Ǹ� ������ �� �� ������ �����ϴٰ� ����
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
		UpdateInRoomState();
		break;
	} 
	case SC_PACKET_TYPE::fail: { 	// ���� �����߰ų�, �����ų� �����Ǿ� �� ���� ������ ���
		// ����
		SC_FAIL* packet = GetPacket<SC_FAIL>();
		
		// ���п� ���� ó��
		break;
	}
	case SC_PACKET_TYPE::ready: {	// ������ �غ� ������ ��� 
		SC_READY* packet = GetPacket<SC_READY>();

		// �ش� cid�� ���� �÷��̾ ã�� ������¸� �ݴ�� �ٲپ��ش�.
		auto pindex = roomInfo.findPlayerIndex(packet->readyClientID);
		pindex->ready = !pindex->ready;
		UpdateInRoomState();
		break;
	}
	case SC_PACKET_TYPE::roomVisitPlayerInfo: {	// ���� �濡 ���� ���
		SC_ROOM_VISIT_PLAYER_INFO* packet = GetPacket<SC_ROOM_VISIT_PLAYER_INFO>();

		// �� ������ �ش� �÷��̾��� ������ �߰��Ѵ�.
		Player_Info pi{ packet->visitClientID, false };
		memcpy(pi.name, packet->name, 20);
		roomInfo.players.push_back(pi);
		roomInfo.nParticipant++;
		UpdateInRoomState();
		break;
	}
	case SC_PACKET_TYPE::roomOutPlayerInfo: { // ���� �濡�� ���� ���
		SC_ROOM_OUT_PLAYER_INFO* packet = GetPacket<SC_ROOM_OUT_PLAYER_INFO>();

		// �ش� Ŭ���̾�Ʈ�� ã�� ����� ������ �ٲ���ٸ� ���� �Ӹ��Ѵ�.
		auto pindex = roomInfo.findPlayerIndex(packet->outClientID);
		roomInfo.players.erase(pindex);
		roomInfo.nParticipant--;
		roomInfo.host = packet->newHostID;
		UpdateInRoomState();
		break;
	}
	case SC_PACKET_TYPE::gameStart: {
		gameFramework.GetSoundManager().Stop("introMusic");
		// ������ ���ƿ��� �� ��� �÷��̾ �����¿��� �ϱ� ������ �̸� Ǯ���ش�.
		for (auto& player : roomInfo.players) {
			player.ready = false;
		}
		isLoading = true;
		changeUI(LobbyState::inRoom, false);
		for (shared_ptr<RoomPlayerObject>& pRoomPlayerObject : pRoomPlayerObjects) {
			dynamic_pointer_cast<SkinnedGameObject>(pRoomPlayerObject->GetObj())->SetVisible(false);
		}

		loadingScene = make_shared<PlayScene>();

		// ������ ���۵� ��� ���� ���ӿ��� ���� �ν��Ͻ� ����, �޽�, �ִϸ��̼�, �ؽ�ó ���� ������ �ε��Ѵ�.
		gameFramework.LoadingScene(loadingScene);

		// �÷��� �� �ε��� ��� �Ϸ�� ��� �������� �ε� �Ϸ� ��Ŷ�� ������. 
		CS_LOADING_COMPLETE sendPacket;
		sendPacket.roomID = roomInfo.id;

		sendPacket.cid = cid;
		SendFixedPacket(sendPacket);
		break;
	}
	case SC_PACKET_TYPE::allPlayerLoadingComplete: {
		pBackGround->SetEnable(false);
		SetCapture(hWnd);

		isLoading = false;
		gameFramework.InitOldCursor();
		gameFramework.PushScene(loadingScene);
		gameStartTime = chrono::system_clock::now();
		loadingScene = nullptr;
		break;
	}
	case SC_PACKET_TYPE::yourPlayerObjectID: {
		SC_YOUR_PLAYER_OBJECTID* packet = GetPacket<SC_YOUR_PLAYER_OBJECTID>();

		myObjectID = packet->objectID;
		break;
	}
	case SC_PACKET_TYPE::playersInfo: {
		SC_PLAYERS_INFO* packet = GetPacket<SC_PLAYERS_INFO>();
		cout << "playersInfo : �κ������ �̵��ϸ鼭 ó���� ���� �ʾƵ� �Ǵ� ��Ŷ\n";
		break;
	}
	default:
		cout << "������ ��Ŷ. Ÿ�� = " << (int)packetType << "\n";
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

	// �ؽ�ó�� ����ִ� ��ũ���� ���� �����Ѵ�.	
	Shader::SetDescriptorHeap(_pCommandList);

	gameFramework.GetShader("UIShader")->PrepareRender(_pCommandList);
	pBackGround->Render(_pCommandList);

	if (!isLoading) {
		for (auto [name, pUI] : pUIs) {
			if(pUI.get() != pBackGround.get())
				pUI->Render(_pCommandList);
		}
		for (auto [name, pButton] : pButtons) {
			pButton->Render(_pCommandList);
		}
		if (currState == LobbyState::inRoom) {
			RenderPlayerMesh(_pCommandList);
		}
	}

}

void LobbyScene::PostRender() {
	if (!isLoading) {
		for (auto [name, pButton] : pButtons) {
			pButton->PostRender();
		}
		// ���ڴ� Render������ ���� ���� �Ŀ� ����
		for (auto [name, pText] : pTexts) {
			pText->Render();
		}
	}
}

void LobbyScene::ReActButton(shared_ptr<Button> _pButton) { // ���� ��ư�� ���� ���
	ButtonType type = _pButton->GetType();
	switch (type) {
	case ButtonType::start: {
		// �������� CS_QUERY_ROOMLIST_INFO ��Ŷ�� ���� ���� �� ����Ʈ�� �����޶�� ��û�Ѵ�.
		CS_QUERY_ROOMLIST_INFO sPacket;
		sPacket.cid = cid;
		sPacket.roomPage = 1;
		SendFixedPacket(sPacket);
		changeUI(LobbyState::title, false);
		changeUI(LobbyState::roomList, true);
		break;
	}
	case ButtonType::exit: {
		DestroyWindow(hWnd);
		break;
	}
	case ButtonType::makeRoom: {

		// ��Ŷ�� ���� ��, �� ������ �׻� �����ϹǷ� �������� ���� ��Ŷ�� ���� �ʰ�
		// ������ ���� ������ �����Ѵ�.
		CS_MAKE_ROOM sPacket;
		sPacket.hostID = cid;
		SendFixedPacket(sPacket);

		// �� ������ ��� �� ������ �������� �� �ϳ��� �����.
		roomInfo.ClearRoom();
		Player_Info pi{ cid, true };
		roomInfo.players.push_back(pi);
		roomInfo.nParticipant++;
		roomInfo.host = cid;

		changeUI(LobbyState::roomList, false);
		changeUI(LobbyState::inRoom, true);
		UpdateInRoomState();
		break;
	}
	case ButtonType::quitRoom: {
		// ���� �� �濡�� �����ٴ� ����� �˷��ִ� ��Ŷ�� ����.
		// �������� �˾Ƽ� �� ��Ŷ�� �޾� �� ����Ʈ�� �ٽ� �����ش�.
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
		// ���� �� ��ȣ�� ������ ���� ���� �������� �����.
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
	// �������� �� ���������� ���� ������ ��û�Ѵ�.
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
		// �� �ȿ� ������ Ŭ���̾�Ʈ ���� �� �� ������ ��ư�� ���������� ���� ���� ����
		CS_OUT_ROOM sPacket;
		sPacket.cid = cid;
		SendFixedPacket(sPacket);
	}
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          
void LobbyScene::changeUI(LobbyState _state, bool _active) {
	// �κ���� �� ���� ���¿� ���� ��ư, ��� �̹����� �����Ѵ�.  
	if (_state == LobbyState::title) {
		pButtons["startButton"]->SetEnable(_active);
		//pButtons["optionButton"]->SetEnable(_active);
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

		pTexts["inRoomName1"]->SetEnable(_active);
		pTexts["inRoomName2"]->SetEnable(_active);
		pTexts["inRoomName3"]->SetEnable(_active);
		pTexts["inRoomName4"]->SetEnable(_active);
		pTexts["inRoomName5"]->SetEnable(_active);
		if (_active) {
			SetBackGround("2DUI_roomBG");
		}
	}
	if (_active) currState = _state;
}

array<shared_ptr<RoomPlayerObject>, 5>& LobbyScene::GetRoomPlayerObjects() {
	return pRoomPlayerObjects;
}

void LobbyScene::UpdateRoomText() {
	pTexts["pageNum"]->SetText(to_wstring(roomPage).c_str());
	// ���� ������ ���� �� ����Ʈ�� ������ ���� ���ش�.
	bool lastRoom = false;

	for (UINT i = 0; i < 6; ++i) {

		RoomState state = RoomState::none;
		string baseName = "RoomButton_" + to_string(i + 1);

		// �� ������ ���� ĭ�� ��� 
		if (roomList[i].nParticipant == 0) {
			state = RoomState::none;
			lastRoom = true; // ���Ŀ� ������ ����� ��� �� ������ ǥ���Ѵ�.
		}

		if (!lastRoom) {
			if (roomList[i].started) { // �̹� ������ ���
				state = RoomState::started;
			}
			else if (roomList[i].nParticipant == 5) {	// ������ �� �� ���
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

void LobbyScene::RenderPlayerMesh(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	
	GameFramework& gameFramework = GameFramework::Instance();

	int nLight = 1;
	memcpy(&pMappedLights->lights[0], pLight.get(), sizeof(Light));
	memcpy(&pMappedLights->globalAmbient, &globalAmbient, sizeof(XMFLOAT4));
	memcpy(&pMappedLights->nLight, &nLight, sizeof(int));

	D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = pLightsBuffer->GetGPUVirtualAddress();
	_pCommandList->SetGraphicsRootConstantBufferView(2, gpuVirtualAddress);


	pCamera->UpdateShaderVariable(_pCommandList);
	gameFramework.GetShader("SkinnedLobbyShader")->PrepareRender(_pCommandList);
	for (UINT i = 0; i < roomInfo.nParticipant; ++i) {

		_pCommandList->RSSetViewports(1, &roomViewPort[i]);

		pRoomPlayerObjects[i]->GetObj()->Render(_pCommandList);
	}
}



void LobbyScene::UpdateInRoomState() {

	// ���� �濡 ui, ��ư ����, �ִϸ��̼�, �г��� ���� ���� 
	bool bChange = false;
	string baseName = "inRoomName";
	for (int i = 0; i < 5; ++i) {
		pUIs["2DUI_ready_" + to_string(i + 1)]->SetEnable(false);
		pTexts[baseName + to_string(i + 1)]->SetEnable(false);
	}
	for (UINT i = 0; i < roomInfo.nParticipant; ++i) {

		// �г��� ������Ʈ
		pTexts[baseName + to_string(i + 1)]->SetText(roomInfo.players[i].name);
		pTexts[baseName + to_string(i + 1)]->SetEnable(true);

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
			pRoomPlayerObjects[i]->GetAniController()->ChangeClip("FastRun");
			pUIs["2DUI_ready_" + to_string(i + 1)]->SetTexture("2DUI_ready");
			pUIs["2DUI_ready_" + to_string(i + 1)]->SetEnable(true);
			// ������ �غ������ ���
			if (cid == roomInfo.players[i].clientID) {
				pButtons["gameStartButton"]->SetTexture("2DUI_readyCancelButton");
				bChange = true;
			}
		}
		else {
			pRoomPlayerObjects[i]->GetAniController()->ChangeClip("idle");
		}
	}
	// �غ���°� �ƴҰ��
	if (!bChange) {
		pButtons["gameStartButton"]->SetTexture("2DUI_readyButton");
	}
}

/////////////////////////

PlayScene::PlayScene() {
	globalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);
	//globalAmbient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);

	professorObjectID = 0;
	ranges::fill_n(lightIndex, MAX_LIGHTS, 0);
	radarEnable = false;
	radarInfo = XMFLOAT2(0.f, 0.f);
	radarDuration = 0.f;
}

PlayScene::~PlayScene() {
	pLightsBuffer->Unmap(0, NULL);


}

shared_ptr<FullScreenObject> PlayScene::GetFullScreenObject() const {
	return pFullScreenObject;
}

void PlayScene::changeUI(bool _enable) {
	pUIs["2DUI_stamina"]->SetEnable(_enable);
	pUIs["2DUI_staminaFrame"]->SetEnable(_enable);
	pUIs["2DUI_interact"]->SetEnable(_enable);
	pTexts["leftCoolTime"]->SetEnable(_enable);
	pTexts["rightCoolTime"]->SetEnable(_enable);
	pTexts["remainTime"]->SetEnable(_enable);
	//pTexts["hackRate"]->SetEnable(_enable);
	

	if (isPlayerProfessor) {	// ������ ����� UI 
		pUIs["2DUI_throwAttack"]->SetEnable(_enable);
		pUIs["2DUI_swingAttack"]->SetEnable(_enable);
		pUIs["2DUI_radar"]->SetEnable(_enable);
		pTexts["radarCoolTime"]->SetEnable(_enable);
	}
	else {		// �л��� ����� UI 
		pUIs["2DUI_leftSkill"]->SetEnable(_enable);
		pUIs["2DUI_transparent"]->SetEnable(_enable);
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
	UINT UINTTime = (UINT)max((int)(GAME_TIME - chrono::duration_cast<chrono::seconds>(chrono::system_clock::now() - gameStartTime).count()), 0);
	UINT second = UINTTime % 60;
	wstring mid = second >= 10 ? L" : " : L" : 0";

	pTexts["remainTime"]->SetText((to_wstring(UINTTime / 60) + mid + to_wstring(UINTTime % 60)).c_str());
	
}

void PlayScene::SetPlayer(shared_ptr<Player>& _pPlayer) {
	pPlayer = _pPlayer;
}

char PlayScene::CheckCollision(float _timeElapsed) {
	char result = 0;
	XMFLOAT3 velocity = pPlayer->GetVelocity();
	// �÷��̾ ���� �����ӿ� �̵��� ���� ������
	XMFLOAT3 moveVector = Vector3::ScalarProduct(pPlayer->GetWorldLookVector(), pPlayer->GetVelocity().z);
	float moveDistance = Vector3::Length(moveVector);
	// ���� �����ϴ� �÷��̾ Ż��box �� �浹�ϴ��� Ȯ��
	if (!exit && !isPlayerProfessor && exitBox.Intersects(pPlayer->GetBoundingBox())) {
		exit = true;

		// �������� �ڽ��� Ż���ߴٰ� ��Ŷ�� ������.
		CS_EXIT_PLAYER sendPacket;
		sendPacket.cid = cid;
		sendPacket.playerObjectID = myObjectID;
		SendFixedPacket(sendPacket);
	}

	// �÷��̾ �л��ϰ�� ����, �����۰� �÷��̾��� �浹ó��
	if (!isPlayerProfessor) {
		pZone->CheckCollisionWithAttack();
		pZone->CheckCollisionWithItem();
	}
	else {
		pZone->CheckCollisionWithTrap();
	}
	// ����ü�� ��ֹ����� �浹�� ó��
	pZone->CheckCollisionProjectileWithObstacle();

	shared_ptr<GameObject> collideObj = pZone->CheckCollisionVertical(_timeElapsed);

	// �÷��̾��� OOBB�� y�������� �̵����� �� �� �浹üũ�� �����Ѵ�.
	if (!collideObj) {
		result |= 1<<0;
	}
	else {
		// �ٴ��� ���ٸ� ���� �����Ѵ�.
		if(!pPlayer->GetFloor()) pPlayer->SetFloor(collideObj);
	}

	//XMFLOAT3 normalVector = GetCollideNormalVector(collideObj);
	// �����̵� ����
	//pPlayer->SetMoveFrontVector(Vector3::Subtract(moveVector, Vector3::ScalarProduct(normalVector, Vector3::DotProduct(moveVector, normalVector))));
	


	XMFLOAT3 knockBack = XMFLOAT3();
	// �÷��̾��� x,z�̵� �� OOBB�� ȸ�����Ѻ� �� �浹üũ�� �����Ѵ�.
	vector<shared_ptr<GameObject>> collideObjs = pZone->CheckCollisionRotate(pPlayer->GetFloor());
	collideObj = pZone->CheckCollisionHorizontal(pPlayer->GetFloor());

	if (collideObj) collideObjs.push_back(collideObj);

	 if (collideObjs.size() != 0) {
		// �ε��� ��� 

		XMFLOAT3 collideNormalVector; 

		for (auto& collideObj : collideObjs) {
			collideNormalVector = GetCollideNormalVector(collideObj);
			// ���������� ���
			// ��ü�� �ε��� ���� �������� ���⸸ŭ �о��ش�.
			knockBack = Vector3::Add(knockBack, Vector3::ScalarProduct(collideNormalVector, 0.01f));
			// �߰��� �÷��̾ �̵��� ������ ����, �ε��� ���� �������� ������ ������ ����.
			 if (moveDistance != 0.f)knockBack = Vector3::Subtract(knockBack, Vector3::ScalarProduct(collideNormalVector, Vector3::DotProduct(moveVector, collideNormalVector)));
		}
		pPlayer->SetKnockBack(knockBack);
		// �Ŀ� �÷��̾ �̵���Ų��, �ռ��� knockBack ���ͷ� �ٽ� �о��ش�.


		// ��ü�� ũ�� ������ �������Ͱ� �߸� ������ ��츦 ó���ϱ� ���� �κ�
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
	

	 while (camera->GetCurrentDistance() < camera->GetMaxDistance() && !pZone->CheckObstacleBetweenPlayerAndCamera(camera)) {
		 camera->MoveFront(-0.03f);
		 camera->UpdateObject();
	 }
	 while (camera->GetMinDistance() < camera->GetCurrentDistance() && pZone->CheckObstacleBetweenPlayerAndCamera(camera)) {
		 camera->MoveFront(0.03f);
		 camera->UpdateObject();
	 }

	 // [���� �ڵ�]
	//if (pZone->CheckObstacleBetweenPlayerAndCamera(camera)) {
	//	if (camera->GetMinDistance() < camera->GetCurrentDistance()) {
	//		camera->MoveFront(15.f, min(_timeElapsed, 1.f/30.f));
	//	}
	//}
	//else {
	//	if (camera->GetCurrentDistance() < camera->GetMaxDistance()) {
	//		camera->MoveFront(-15.f, min(_timeElapsed, 1.f / 30.f));
	//		camera->UpdateObject();
	//		if (pZone->CheckObstacleBetweenPlayerAndCamera(camera)) {
	//			camera->MoveFront(15.f, min(_timeElapsed, 1.f / 30.f));
	//			camera->UpdateObject();
	//		}
	//	}
	//}

	return result;
}

XMFLOAT3 PlayScene::GetCollideNormalVector(const shared_ptr<GameObject>& _collideObj) {
	
	bool isOppositeLook = false;
	XMFLOAT3 result, lookVector, rightVector, direcVector, boundaryVector;
	float boundingAngle, playerAngle;

	// �浹�� �ٿ���ڽ��� ������ ��ġ�� ���� ���͸� ���Ѵ�.
	
	BoundingOrientedBox objOOBB = _collideObj->GetBoundingBox();
	BoundingOrientedBox playerOOBB = pPlayer->GetBoundingBox();
	lookVector = _collideObj->GetWorldLookVector();
	rightVector = _collideObj->GetWorldRightVector();

	boundaryVector = Vector3::Rotate(XMFLOAT3(objOOBB.Extents.x, 0, objOOBB.Extents.z), objOOBB.Orientation);
	// �浹�� ��ü�� �躤�Ϳ��� ������ ���Ѵ�.
	boundingAngle = Vector3::Angle(boundaryVector, lookVector, false);
	if (boundingAngle > 90.0f) boundingAngle = 180.0f - boundingAngle;

	// ��ü�� �÷��̾�� ������ ���� ���Ѵ�.
	direcVector = Vector3::Subtract(playerOOBB.Center, objOOBB.Center);
	direcVector.y = 0;

	// �÷��̾�� �躤���� ���� ���Ѵ�.
	playerAngle = Vector3::Angle(direcVector, lookVector, false);

	// �躤�� ����� �ݴ��� ���
	if (playerAngle > 90.0f) {
		playerAngle = 180.0f - playerAngle;
		isOppositeLook = true;
	}
	// ������ ��ġ�� �躤�Ͱ��� �������� �÷��̾�� �躤�Ͱ� �̷�� ���� Ŭ���
	// = RightVector�� ����.
	if (playerAngle > boundingAngle) {
		// RightVector������ �ݴ��ϰ��
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
	// ������� �������� �ʿ��� �ε��� ����
	for (int i = 0; i < MAX_LIGHTS; ++i) {
		lightIndex[i] = i;
	}
	gameFramework.GetGameObjectManager().LoadGameObject("SwingAttack", _pDevice, _pCommandList);
	gameFramework.GetGameObjectManager().LoadGameObject("BookAttack", _pDevice, _pCommandList);
	gameFramework.GetGameObjectManager().LoadGameObject("PrisonKey", _pDevice, _pCommandList);
	gameFramework.GetGameObjectManager().LoadGameObject("MedicalKit", _pDevice, _pCommandList);
	gameFramework.GetGameObjectManager().LoadGameObject("EnergyDrink", _pDevice, _pCommandList);
	gameFramework.GetGameObjectManager().LoadGameObject("Trap", _pDevice, _pCommandList);
	gameFramework.GetGameObjectManager().LoadGameObject("Trap_attack", _pDevice, _pCommandList);
	gameFramework.GetGameObjectManager().LoadGameObject("Book", _pDevice, _pCommandList);

	pFullScreenObject = make_shared<FullScreenObject>(_pDevice, _pCommandList);

	// ���� ���� �ʱ�ȭ
	AllLeverPowerOn = false;
	isPlayerProfessor = false;

	SC_GAME_START* recvPacket = GetPacket<SC_GAME_START>();
	array<UINT, MAX_PARTICIPANT> enComID;
	for (int i = 0; i < MAX_PARTICIPANT; ++i) {
		enComID[i] = recvPacket->activeComputerObjectID[i];
	}

	// Zone�� ���� �� �������� �о� ������Ʈ���� �ε��Ѵ�.
	pZone = make_shared<Zone>(XMFLOAT3(75.0f, 25.0f, 38.0f), XMINT3(10, 5, 5), shared_from_this());
	pZone->LoadZoneFromFile(_pDevice, _pCommandList, enComID);
	

	for (int i = 0; i < pLights.size(); ++i) {
		pLights[i]->SetBakedShadowMap(gameFramework.GetBakedShadowMap(i), gameFramework.GetBakedShadowMapHandle(i));
	}


	gameFramework.GetGameObjectManager().GetExistGameObject("CeilingLampNew")->GetMaterial(0)->SetEmissive(XMFLOAT4(0.2f, 0.1f, 0.1f, 1.0f));
	professorObjectID = recvPacket->professorObjectID;

	for (UINT i = 0; i < recvPacket->nPlayer; ++i) {

		if (recvPacket->playerInfo[i].objectID == myObjectID) {	// ���� ������ ĳ������ ���
			if (professorObjectID == recvPacket->playerInfo[i].objectID) {	// ���� �÷��̾��� ���
				pPlayer = make_shared<Professor>();
				isPlayerProfessor = true;
				pPlayer->Create("Professor"s, _pDevice, _pCommandList);
			}
			else { // �л��� ���
				pPlayer = make_shared<Student>();
				pPlayer->Create("Student"s, _pDevice, _pCommandList);
			}
			
			pPlayer->SetLocalPosition(recvPacket->playerInfo[i].position);
			pPlayer->SetLocalRotation(recvPacket->playerInfo[i].rotation);
			pPlayer->SetLocalScale(recvPacket->playerInfo[i].scale);
			pPlayer->UpdateObject();
			pPlayer->SetID(recvPacket->playerInfo[i].objectID);
			SetPlayer(pPlayer);
			pZone->SetPlayer(pPlayer);
			//[����] �ִϸ��̼� ���� ����

		}
		else {	// �ٸ� �÷��̾��� ĳ���� ������ ���
			shared_ptr<InterpolateMoveGameObject> pOtherPlayer = make_shared<InterpolateMoveGameObject>();

			if (professorObjectID == recvPacket->playerInfo[i].objectID) {	// ���� �÷��̾��� ���
				pOtherPlayer->SetNickname(recvPacket->nickname[i], true);
				pOtherPlayer->Create("Professor"s, _pDevice, _pCommandList);
			}
			else { // �л��� ���
				pOtherPlayer->SetNickname(recvPacket->nickname[i], false);
				pOtherPlayer->Create("Student"s, _pDevice, _pCommandList);
			}
			
			pOtherPlayer->SetLocalPosition(recvPacket->playerInfo[i].position);
			pOtherPlayer->SetLocalRotation(recvPacket->playerInfo[i].rotation);
			pOtherPlayer->SetLocalScale(recvPacket->playerInfo[i].scale);
			pOtherPlayer->UpdateObject();
			pOtherPlayer->SetID(recvPacket->playerInfo[i].objectID);

			pOtherPlayers.emplace(pOtherPlayer->GetID(), pOtherPlayer);
			
			// �浹ó���� ���� Sector�� �߰��Ѵ�.
			pZone->AddObject(SectorLayer::otherPlayer, recvPacket->playerInfo[i].objectID, pOtherPlayer, pZone->GetIndex(pOtherPlayer->GetWorldPosition()));
			//[����] �ִϸ��̼� ���� ����
		}

	}

	pPlayer->UpdateObject();
	camera = pPlayer->GetCamera();

	pSkyBox = make_shared<SkyBox>(_pDevice, _pCommandList);

	shared_ptr<Image2D> pImg;

	pUIs["2DUI_stamina"] = make_shared<Image2D>("2DUI_stamina", XMFLOAT2(0.488f, 0.15f), XMFLOAT2(1.512f, 1.85f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, true);
	pUIs["2DUI_staminaFrame"] = make_shared<Image2D>("2DUI_staminaFrame", XMFLOAT2(0.6f, 0.15f), XMFLOAT2(1.4f, 1.85f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, true);
	pUIs["2DUI_interact"] = make_shared<Image2D>("2DUI_interact", XMFLOAT2(0.3f, 0.1f), XMFLOAT2(0.f, 0.f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
	//pUIs["2DUI_hackRate"] = make_shared<Image2D>("2DUI_hackRate", XMFLOAT2(0.3f, 0.1f), XMFLOAT2(0.f, 0.f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);

	pTexts["leftCoolTime"] = make_shared<TextBox>((WCHAR*)L"Who asks Satan", D2D1::ColorF(1, 1, 1, 1), XMFLOAT2(1.55f, 1.5f), XMFLOAT2(0.1f, 0.1f), C_WIDTH / 30.0f, false);
	pTexts["rightCoolTime"] = make_shared<TextBox>((WCHAR*)L"Who asks Satan", D2D1::ColorF(1, 1, 1, 1), XMFLOAT2(1.8f, 1.5f), XMFLOAT2(0.1f, 0.1f), C_WIDTH / 30.0f, false);

	pTexts["remainTime"] = make_shared<TextBox>((WCHAR*)L"Who asks Satan", D2D1::ColorF(1, 1, 1, 1), XMFLOAT2(0.9f, 0.1f), XMFLOAT2(0.2f, 0.2f), C_WIDTH / 40.0f, true);
	pTexts["hackRate"] = make_shared<TextBox>((WCHAR*)L"Who asks Satan", D2D1::ColorF(1, 1, 1, 1), XMFLOAT2(0.9f, 0.3f), XMFLOAT2(0.2f, 0.2f), C_WIDTH / 60.0f, false);


	if (isPlayerProfessor) {	// ������ ����� UI �ε�
		pUIs["2DUI_swingAttack"] = make_shared<Image2D>("2DUI_swingAttack", XMFLOAT2(0.2f, 0.2f), XMFLOAT2(1.5f, 1.5f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, true);
		pUIs["2DUI_throwAttack"] = make_shared<Image2D>("2DUI_throwAttack", XMFLOAT2(0.2f, 0.2f), XMFLOAT2(1.75f, 1.5f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, true);

		pTexts["radarCoolTime"] = make_shared<TextBox>((WCHAR*)L"Who asks Satan", D2D1::ColorF(1, 1, 1, 1), XMFLOAT2(0.25f, 1.7f), XMFLOAT2(0.1f, 0.1f), C_WIDTH / 30.0f, false);
		pUIs["2DUI_radar"] = make_shared<Image2D>("2DUI_radar", XMFLOAT2(0.2f, 0.2f), XMFLOAT2(0.2f, 1.7f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, true);
	}
	else {		// �л��� ����� UI �ε�
		pUIs["2DUI_leftSkill"] = make_shared<Image2D>("2DUI_skillFrame", XMFLOAT2(0.2f, 0.2f), XMFLOAT2(1.5f, 1.5f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, true);
		pUIs["2DUI_leftSkill"]->SetDark(true);
		
		pUIs["2DUI_transparent"] = make_shared<Image2D>("2DUI_transparent", XMFLOAT2(0.2f, 0.2f), XMFLOAT2(1.75f, 1.5f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, true);

		pUIs["2DUI_energyDrink"] = make_shared<Image2D>("2DUI_energyDrink", XMFLOAT2(0.2f, 0.2f), XMFLOAT2(1.5f, 1.5f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
		pUIs["2DUI_medicalKit"] = make_shared<Image2D>("2DUI_medicalKit", XMFLOAT2(0.2f, 0.2f), XMFLOAT2(1.5f, 1.5f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
		pUIs["2DUI_prisonKey"] = make_shared<Image2D>("2DUI_prisonKey", XMFLOAT2(0.2f, 0.2f), XMFLOAT2(1.5f, 1.5f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
		pUIs["2DUI_trap"] = make_shared<Image2D>("2DUI_trap", XMFLOAT2(0.2f, 0.2f), XMFLOAT2(1.5f, 1.5f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);

		pUIs["2DUI_hp"] = make_shared<Image2D>("2DUI_hp", XMFLOAT2(0.488f, 0.15f), XMFLOAT2(1.512f, 1.7f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, true);
		pUIs["2DUI_hpFrame"] = make_shared<Image2D>("2DUI_hpFrame", XMFLOAT2(0.6f, 0.15f), XMFLOAT2(1.4f, 1.7f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, true);

		pUIs["2DUI_hacking"] = make_shared<Image2D>("2DUI_hacking", XMFLOAT2(0.5f, 0.1f), XMFLOAT2(0.75f, 1.4f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
		pUIs["2DUI_hackingFrame"] = make_shared<Image2D>("2DUI_hackingFrame", XMFLOAT2(0.5f, 0.1f), XMFLOAT2(0.75f, 1.4f), XMFLOAT2(1.f, 1.f), _pDevice, _pCommandList, false);
	}


	pFrustumMesh = make_shared<FrustumMesh>();
	pFrustumMesh->Create(camera->GetBoundingFrustum(), _pDevice, _pCommandList);

	ComPtr<ID3D12Resource> temp;
	UINT ncbElementBytes = ((sizeof(LightsMappedFormat) + 255) & ~255); //256�� ���
	pLightsBuffer = ::CreateBufferResource(_pDevice, _pCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, temp);
	pLightsBuffer->Map(0, NULL, (void**)&pMappedLights);

	gameFramework.GetSoundManager().Play("horror", true);

	Shader::SetCamera(camera);


}

void PlayScene::ReleaseUploadBuffers() {
	GameFramework& gameFramework = GameFramework::Instance();
	gameFramework.GetTextureManager().ReleaseUploadBuffers();
}


void PlayScene::ProcessKeyboardInput(const array<bool, 256>& _keyDownBuffer, const array<UCHAR, 256>& _keysBuffers, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	bool move = false;

	GameFramework& gameFramework = GameFramework::Instance();
	// ��ӿ�� �̸� timeElapsed�� �����ش�

	//if (_keyDownBuffer['N']) {
	//	pPlayer->GetAniController()->AddCurrentClipAniSpeedRatio(-0.05f);
	//}
	//if (_keyDownBuffer['M']) {
	//	pPlayer->GetAniController()->AddCurrentClipAniSpeedRatio(+0.05f);
	//}


	if (_keyDownBuffer['E']) {
		// ��ȣ�ۿ� Ű
		if (pInteractableObject && pInteractableObject->IsEnable())
			pInteractableObject->QueryInteract();
	}
	if (_keyDownBuffer['R']) {
		CS_OPEN_PRISON_DOOR sendPacket;
		sendPacket.cid = cid;
		SendFixedPacket(sendPacket);
	}
	if (_keyDownBuffer['F']) {
		fpsToggle = !fpsToggle;
		pTexts["fps"]->SetEnable(fpsToggle);
	}
	if (_keyDownBuffer[VK_F12]) {
		DestroyWindow(hWnd);
	}


	if (_keysBuffers[VK_SHIFT] & 0xF0) {
		pPlayer->Dash(_timeElapsed);
	}

	float angleSpeed = 720.f * _timeElapsed;
	float moveSpeed = pPlayer->GetSpeed() * _timeElapsed * (1 - pPlayer->GetSlowRate() / 100.0f);
	XMFLOAT3 moveVector = XMFLOAT3();

	// "�л�"�̰ų� "������ �������� �ƴҶ�" �̵��� �� �ִ�.
	auto pProfessor = dynamic_pointer_cast<Professor>(pPlayer);
	if (!pProfessor || !(pProfessor->IsSwingAttacking() || pProfessor->IsThrowAttacking())) {

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
	}
	if (pProfessor) {
		if (_keyDownBuffer['Q'] && pProfessor->GetRadarCoolTime() <= 0) {
			pProfessor->SetRadarCoolTime();
			radarEnable = true;
			// ���̰��� 0���� �ʱ�ȭ�ϰ� ���̴��� Ȱ��ȭ
			radarInfo.x = 0.0f;
			radarInfo.y = 1.0f;
			gameFramework.GetSoundManager().SetPosition("radar", pProfessor->GetWorldPosition());
			gameFramework.GetSoundManager().Play("radar");
			radarDuration = 10.0f;
			GetText("radarCoolTime")->SetEnable(true);
			GetUI("2DUI_radar")->SetDark(true);
		}
	}
	
	if (move && !Vector3::IsSame(XMFLOAT3(), moveVector)) {
		moveVector = Vector3::Normalize(moveVector);
		pPlayer->RotateMoveHorizontal(moveVector, angleSpeed, moveSpeed);

	}
	if (_keysBuffers['1'] & 0xF0) {
		pPlayer->GetAniController()->ChangeClip("run");
	}
	if (_keysBuffers['2'] & 0xF0) {
		pPlayer->GetAniController()->ChangeClip("FastRun");
	}
	if (_keysBuffers['3'] & 0xF0) {
		pPlayer->GetAniController()->ChangeClip("Hacking");
	}
	if (_keysBuffers['4'] & 0xF0) {
		pPlayer->GetAniController()->ChangeClip("Jump");
	}

	if (_keysBuffers[VK_SPACE] & 0xF0) {
		pPlayer->Jump(500.0f);
	}

}

void PlayScene::AnimateObjects(char _collideCheck, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {


	// Ż���ϰ� �Ǹ� ȭ���� ����� �Ǹ鼭 ���� ����� �Ǹ� ������ Ż���ߴٴ� ��Ŷ�� �����鼭 ������ ������.
	if (3 <= fadeOut) {
		GameFramework& gameFramework = GameFramework::Instance();
		changeUI(false);
		gameFramework.PopScene();
		gameFramework.GetSoundManager().Stop("horror");
		gameFramework.GetSoundManager().Stop("lightBGM");
		gameFramework.GetSoundManager().Play("introMusic", true);


		// ������ ������.
		CS_EXIT_GAME sendPacket;
		sendPacket.cid = cid;
		SendFixedPacket(sendPacket);

		ReleaseCapture();

		auto pLobbyScene = dynamic_pointer_cast<LobbyScene>(gameFramework.GetCurrentScene());
		if (pLobbyScene) {
			pLobbyScene->changeUI(LobbyState::inRoom, true);
			pLobbyScene->UpdateInRoomState();
			for (shared_ptr<RoomPlayerObject>& pRoomPlayerObject : pLobbyScene->GetRoomPlayerObjects()) {
				dynamic_pointer_cast<SkinnedGameObject>(pRoomPlayerObject->GetObj())->SetVisible(true);
			}
		}
		return;
	}

	if (radarEnable) {
		radarInfo.x += _timeElapsed * 8.f;
	}
	
	// ���̴� ���ӽð��� ���� ���
	if (radarDuration <= 0) {

		radarEnable = false;
		if (radarInfo.y > 0) {
			radarInfo.y -= 0.005f;
		}
		else radarInfo.y = 0;

	}
	else {
		radarDuration -= _timeElapsed;
	}
	

	GameFramework& gameFramework = GameFramework::Instance();
	for (auto& pEffect : pEffects) {
		pEffect->Animate(_timeElapsed);
	}

	if (fpsToggle) {
		wstring fpsText = L"FPS : " + to_wstring(gameFramework.GetFPS());
		pTexts["fps"]->SetText(fpsText.c_str());
	}

	// �÷��̾� �ִϸ��̼�
	pPlayer->Animate(_collideCheck, _timeElapsed);
	camera->UpdateObject();

	// �׷����� ȭ�� ������Ʈ�� ���� ������Ʈ
	pFullScreenObject->Animate(_timeElapsed);

	// �÷��̾� ���ؿ��� �������ҿ� ���ԵǴ� ���͸� ������Ʈ ���ش�.
	pZone->UpdateFrustumSectors(*camera->GetBoundingFrustum());

	// û���� ��ġ�� ������Ʈ���ش�. ( ���� Play �� ȣ��� ������ ��� )
	gameFramework.GetSoundManager().UpdateListener(pPlayer->GetWorldPosition(), camera->GetWorldLookVector(), camera->GetWorldUpVector());

	bool enable = false;
	// ���� �÷��̾ ��ȣ�ۿ� ������ ������Ʈ�� ã�´�.
	auto pObject = pZone->UpdateInteractableObject();
	if(pObject) pObject->GetObj()->SetDrawOutline(true);

	// ������ �纸Ÿ�� ��Ÿ���϶��� ��ȣ�ۿ����� ���ϵ��� �Ѵ�.
	if (dynamic_pointer_cast<Lever>(pObject) && isPlayerProfessor 
		&& 0 < dynamic_pointer_cast<Professor>(pPlayer)->GetSabotageCoolTime()) {
		pObject = nullptr;
	}

	// �ֺ��� ��ȣ�ۿ� ������Ʈ�� �ִٰ� ������ ���
	if (pInteractableObject && !pObject) {
		pInteractableObject->EndInteract();
		pInteractableObject->GetObj()->SetDrawOutline(false);
	}
	pInteractableObject = pObject;

	auto pComputer = pEnableComputers.end();

	if (!isPlayerProfessor)
		static_pointer_cast<Student>(pPlayer)->SetHacking(false);
	if (pInteractableObject) {

		// ���� �÷��̾ ��ŷ���� ��ǻ�Ͱ� �ִ��� Ȯ��
		// ���� �ֺ��� ��ȣ�ۿ� ������Ʈ�� ������ ��� ������ ���
		enable = pInteractableObject->IsEnable();
		if (enable) {
			XMFLOAT2 pos = GetWorldToScreenCoord(pInteractableObject->GetBoundingBox().Center, camera->GetViewTransform(), camera->GetProjectionTransform());
			// ��ȣ�ۿ� UI�� ��ǥ�� �������ش�.
			// ����Ʈ ��ǥ�� -1~1 -> UI��ǥ�� 0~2        
			pUIs["2DUI_interact"]->SetPosition(XMFLOAT2(pos.x + 1, pos.y + 1));
		}
		// ���� ������� ��ǻ���� ���
		else {

			pComputer = ranges::find(pEnableComputers, myObjectID, &Computer::GetUse);
			if (pComputer != pEnableComputers.end()) {
				float hackingRate = (*pComputer)->GetHackingRate();
				pUIs["2DUI_hacking"]->SetEnable(true);
				pUIs["2DUI_hacking"]->SetSizeUV(XMFLOAT2(hackingRate / 100, 1.f));
				pUIs["2DUI_hackingFrame"]->SetEnable(true);

				// �÷��̾ ��ŷ���̶�� ���¸� �����Ѵ�.(�ִϸ��̼� �Ǵ�)
				if (!isPlayerProfessor)
					static_pointer_cast<Student>(pPlayer)->SetHacking(true);
			}
		}
	}


	// �÷��̾ �����ϰ���� UI
	if (isPlayerProfessor) {
		auto pProfessor = static_pointer_cast<Professor>(pPlayer);

	}
	// �÷��̾ �л��ϰ���� UI
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



	// �ٸ� �÷��̾�鿡 ���� �̵��� ����
	for (auto& [objectID, pOtherPlayer] : pOtherPlayers) {
		XMINT3 prevIndex = pZone->GetIndex(pOtherPlayer->GetWorldPosition());
		pOtherPlayer->Animate(_timeElapsed, pPlayer->GetWorldPosition());

		XMINT3 nextIndex = pZone->GetIndex(pOtherPlayer->GetWorldPosition());

		// ���� �ε����� ���ؼ� �ٲ���ٸ� ���͸� �ٲپ��ش�.
		if (prevIndex.x != nextIndex.x || prevIndex.y != nextIndex.y || prevIndex.z != nextIndex.z) {
			pZone->HandOffObject(SectorLayer::otherPlayer, pOtherPlayer->GetID(), pOtherPlayer, prevIndex, nextIndex);
		}
	}
	for (auto& [objectID, pOtherPlayer] : pOtherPlayers) {
		pOtherPlayer->SetVisible(false);
	}

	// �̵� ������ �ش� �÷��̾ ȭ�鿡 ���̴��� �Ǵ�
	pZone->SetVisiblePlayer(camera);

	for (auto& [objectID, pOtherPlayer] : pOtherPlayers) {

		shared_ptr<TextBox> nickname = pOtherPlayer->GetNickname();
		// ���Ż��°� �ƴϸ鼭 �������� ���� ���
		if (pOtherPlayer->GetVisible() && !pOtherPlayer->GetTransparent()) {
			XMFLOAT3 position = pOtherPlayer->GetBoundingBox().Center;
			position.y += pOtherPlayer->GetBoundingBox().Extents.y;
			// ����Ʈ ��ǥ�� -1~1 -> UI��ǥ�� 0~2    
			XMFLOAT2 pos = GetWorldToScreenCoord(position, camera->GetViewTransform(), camera->GetProjectionTransform());
			nickname->SetPosition(XMFLOAT2(pos.x + 1, pos.y + 1), true);
			nickname->SetEnable(true);
		}
		else {
			nickname->SetEnable(false);
		}
	}


	// ��Ÿ�� �ؽ�Ʈ�� �������ش�.
	if (isPlayerProfessor) {	// ����
		auto pProfessor = static_pointer_cast<Professor>(pPlayer);
		UpdateCoolTimeText("leftCoolTime", "2DUI_swingAttack", pProfessor->GetCoolTime(AttackType::swingAttack));
		UpdateCoolTimeText("rightCoolTime", "2DUI_throwAttack", pProfessor->GetCoolTime(AttackType::throwAttack));
		UpdateCoolTimeText("radarCoolTime", "2DUI_radar", pProfessor->GetRadarCoolTime());
	}
	else {	// �л�
		auto pStudent = static_pointer_cast<Student>(pPlayer);
		UpdateCoolTimeText("rightCoolTime", "2DUI_transparent", pStudent->GetCoolTime());
	}

	// zone �� ������Ʈ�鿡 ���� animate�� ����
	pZone->UpdatePlayerSector();

	for (auto& pLight : pLights) {
		if (pLight) {
			pLight->UpdateLight();
		}
	}

	// �ð��� �����Ѵ�.
	UpdateTimeText();

	pZone->AnimateObjects(_timeElapsed);
}

void PlayScene::ProcessSocketMessage(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList)
{
	GameFramework& gameFramework = GameFramework::Instance();

	// ���������� ��Ŷ�� Recv�޴´�.
	static int recvByte = 0;

	int result = RecvFixedPacket(recvByte);
	if (result != SOCKET_ERROR) {
		recvByte += result;
		// �� ���� ���
		if (recvByte < BUFSIZE) {
			return;
		}
		// ��� �޾��� ���
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
			// ���ο� ���� ������ ��� 
			SC_PLAYER_INFO& pinfo = packet->playersInfo[i];
			if (pinfo.objectID == pPlayer->GetID())
				continue;
			// ���� �÷��̾��� �� ���������� ������Ʈ ���ش�.
			shared_ptr<InterpolateMoveGameObject> pMoveClient = pOtherPlayers.find(pinfo.objectID)->second;
			prevIndex = pZone->GetIndex(pMoveClient->GetWorldPosition());
			pMoveClient->SetNextTransform(pinfo.position, pinfo.rotation, pinfo.scale);
			nextIndex = pZone->GetIndex(pMoveClient->GetWorldPosition());
			// ��ġ�� ���� ���Ͱ� �ٲ���ٸ� ������Ʈ�� �Ű��ش�.
			if (prevIndex.x != nextIndex.x || prevIndex.y != nextIndex.y || prevIndex.z != nextIndex.z) {
				pZone->HandOffObject(SectorLayer::otherPlayer, pMoveClient->GetID(), pMoveClient, prevIndex, nextIndex);
			}
			// �ִϸ��̼� ó��
			string clipName = pinfo.clipName;
			pMoveClient->GetAniController()->ChangeClip(clipName);
		}
		break;
	}
	case SC_PACKET_TYPE::toggleDoor: {
		SC_TOGGLE_DOOR* packet = GetPacket<SC_TOGGLE_DOOR>();
		// �ش� ������Ʈ�� ���� ��ȣ�ۿ��� �Ѵ�.
		pZone->Interact(packet->objectID);
		break;
	}
	case SC_PACKET_TYPE::useWaterDispenser: {
		SC_USE_WATER_DISPENSER* packet = GetPacket<SC_USE_WATER_DISPENSER>();
		XMFLOAT3 particlePos;

		// �ش� ������Ʈ�� ���� ��ȣ�ۿ��� �Ѵ�.
		pZone->Interact(packet->waterDispenserObjectID);
		if (packet->playerObjectID == myObjectID) {	// �ڽ��� �����⸦ ������� ���
			// ���׹̳ʸ� �����Ѵ�.
			pPlayer->SetMP(100.f);
			gameFramework.GetSoundManager().SetPosition("water", pPlayer->GetWorldPosition());
			gameFramework.GetSoundManager().Play("water");
			particlePos = pPlayer->GetHeadObject()->GetWorldPosition();
		}
		else {
			particlePos = pOtherPlayers[packet->playerObjectID]->GetHeadObject()->GetWorldPosition();
		}

		// ��ƼŬ ����
		VS_ParticleMappedFormat particle;
		particle.boardSize = { 0.03f, 0.03f };
		particle.lifetime = 1.f;
		particle.position = particlePos;
		particle.type = (int)PARTICLE_TYPE::waterDispenserUse;

		uniform_real_distribution<float> urd(-5.f, 5.f);
		for (int i = 0; i < 100; ++i) {
			particle.velocity = XMFLOAT3(urd(rd), urd(rd), urd(rd));
			Shader::AddParticle(particle);
		}
		break;
	}
	case SC_PACKET_TYPE::hackingRate: {
		// � �÷��̾ ��ŷ�� ������ �����ϰ� ��������� ���� ���·� �ٲپ��ش�.
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

		// ���� ����ϰ� �� ���
		if (packet->playerObjectID == myObjectID) {
			(*pComputer)->Interact();
		}
		break;
	}
	case SC_PACKET_TYPE::attack: {
		// �˸��� ���Ϳ� ������ �߰��ϰ� ������ ������ ��� ��Ÿ���� �����Ѵ�.
		SC_ATTACK* packet = GetPacket<SC_ATTACK>();
		pZone->AddAttack(packet->attackType, packet->attackObjectID, FindPlayerObject(packet->playerObjectID), _pDevice, _pCommandList);
		if (packet->playerObjectID == myObjectID && isPlayerProfessor) {
			auto pProfessor = static_pointer_cast<Professor>(pPlayer);
			pProfessor->Reload(packet->attackType);
		}
		break;
	}
	case SC_PACKET_TYPE::hit: {
		// �ش��÷��̾��� ü���� ���, �ش� ������Ʈ�� �����Ѵ�.
		SC_ATTACK_HIT* packet = GetPacket<SC_ATTACK_HIT>();
		// ���� ���� ��Ŷ�� ���� �ʴ´�.
		auto pHitPlayerObject = dynamic_pointer_cast<InterpolateMoveGameObject>(FindPlayerObject(packet->hitPlayerObjectID));
		auto pAttack = pZone->GetAttack(packet->attackObjectID);
		if (pHitPlayerObject) {
			if (!pAttack) break;
			pHitPlayerObject->AddHP(-pAttack->GetDamage());
			pHitPlayerObject->SetHit(true);

			// ���� ���
			gameFramework.GetSoundManager().SetPosition("hittingSound", pHitPlayerObject->GetWorldPosition());
			gameFramework.GetSoundManager().Play("hittingSound");
		}
		else {
			cout << "�ش� �÷��̾ �����ϴ�!!\n";
		}
		if (pAttack && pAttack->GetAttackType() == AttackType::throwAttack)
			pAttack->Remove();
		
		break;
		
	}
	case SC_PACKET_TYPE::goPrison: {
		SC_GO_PRISON* packet = GetPacket<SC_GO_PRISON>();
		// �ٸ� �÷��̾�(���������̼� ������Ʈ)�� �����̵� ��Ų��.
		shared_ptr<InterpolateMoveGameObject> pTeleportPlayer = pOtherPlayers.find(packet->playerObjectID)->second;
		XMINT3 prevIndex = pZone->GetIndex(pTeleportPlayer->GetWorldPosition());
		pTeleportPlayer->SetNextTransform(prisonPosition, pTeleportPlayer->GetLocalRotate(), pTeleportPlayer->GetLocalScale());
		pTeleportPlayer->SetNextTransform(prisonPosition, pTeleportPlayer->GetLocalRotate(), pTeleportPlayer->GetLocalScale());
		XMINT3 nextIndex = pZone->GetIndex(pTeleportPlayer->GetWorldPosition());

		// �����ִ� ���·� �ٲ۴�. + hp�� 50���� �ٲ۴�.
		pTeleportPlayer->SetImprisoned(true);
		pTeleportPlayer->SetHP(50.f);

		// ��ġ�� ���� ���Ͱ� �ٲ���ٸ� �ڵ���� ���ش�.
		if (prevIndex.x != nextIndex.x || prevIndex.y != nextIndex.y || prevIndex.z != nextIndex.z) {
			pZone->HandOffObject(SectorLayer::otherPlayer, pTeleportPlayer->GetID(), pTeleportPlayer, prevIndex, nextIndex);
		}
		break;
	}
	case SC_PACKET_TYPE::openPrisonDoor: {
		SC_OPEN_PRISON_DOOR* packet = GetPacket<SC_OPEN_PRISON_DOOR>();
		

		// �ڽ��� �л��̰� �ڱ� �ڽ��� �����ִٸ� ����Ż�� ��ġ�� �����̵� ��Ų��.
		if (!isPlayerProfessor) {
			auto pStudent = static_pointer_cast<Student>(pPlayer);
			// ������ ���� �����̶�� ���踦 �����.
			if (packet->openPlayerObjectID == myObjectID) {
				pStudent->SetItem(ObjectType::none);
				Scene::GetUI("2DUI_prisonKey")->SetEnable(false);
				Scene::GetUI("2DUI_leftSkill")->SetEnable(true);

			}
			if (pStudent->GetImprisoned()) {	// �����Ǿ� �ִٸ�
				// ���������� �ٲ۴�.
				pStudent->SetImprisoned(false);
				// �����̵� ��Ų��.
				pStudent->SetLocalPosition(prisonExitPosition);
				pStudent->UpdateObject();
			}
		}
		// �ٸ� �л��� ���� ���¸� �����Ű�� �����̵� ��Ų��.
		for (auto& [objectID, pOtherPlayer] : pOtherPlayers) {
			if (objectID != professorObjectID && pOtherPlayer->GetImprisoned()) {
				pOtherPlayer->SetImprisoned(false);	// ���� ����
				// �����̵� ��Ų��.
				XMINT3 prevIndex = pZone->GetIndex(pOtherPlayer->GetWorldPosition());
				pOtherPlayer->SetNextTransform(prisonExitPosition, pOtherPlayer->GetLocalRotate(), pOtherPlayer->GetLocalScale());
				pOtherPlayer->SetNextTransform(prisonExitPosition, pOtherPlayer->GetLocalRotate(), pOtherPlayer->GetLocalScale());
				XMINT3 nextIndex = pZone->GetIndex(pOtherPlayer->GetWorldPosition());

				// ��ġ�� ���� ���Ͱ� �ٲ���ٸ� �ڵ���� ���ش�.
				if (prevIndex.x != nextIndex.x || prevIndex.y != nextIndex.y || prevIndex.z != nextIndex.z) {
					pZone->HandOffObject(SectorLayer::otherPlayer, pOtherPlayer->GetID(), pOtherPlayer, prevIndex, nextIndex);
				}
			}
		}

		break;
	}
	case SC_PACKET_TYPE::toggleLever: {
		SC_LEVER_TOGGLE* packet = GetPacket<SC_LEVER_TOGGLE>();
		
		for (auto& pComputer : pEnableComputers) {
			pComputer->SetPower(packet->allLeverPowerOn);
		}
		if (packet->allLeverPowerOn) {
			globalAmbient = XMFLOAT4(0.5, 0.5, 0.5, 1.0);
			AllLeverPowerOn = true;
			gameFramework.GetGameObjectManager().GetExistGameObject("CeilingLampNew")->GetMaterial(0)->SetEmissive(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
			for (auto& pLight : pLights) {
				pLight->diffuse = XMFLOAT4(5.5f, 7.5f, 5.5f, 1.f);
			}
			if (isPlayerProfessor)
				static_pointer_cast<Professor>(pPlayer)->SetSabotageCoolTime(60.f);

			gameFramework.GetSoundManager().Stop("horror");
			gameFramework.GetSoundManager().SetPosition("lightSound", pPlayer->GetWorldPosition());
			gameFramework.GetSoundManager().Play("lightSound");
			gameFramework.GetSoundManager().Play("lightBGM", true);
		}
		else {
			globalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);
			gameFramework.GetGameObjectManager().GetExistGameObject("CeilingLampNew")->GetMaterial(0)->SetEmissive(XMFLOAT4(0.2f, 0.1f, 0.1f, 1.0f));
			for (auto& pLight : pLights) {
				pLight->diffuse = XMFLOAT4(3.0f, 0.0f, 0.0f, 1.0f);
			}
			if (AllLeverPowerOn == true) {
				gameFramework.GetSoundManager().Play("horror", true);
				gameFramework.GetSoundManager().Stop("lightBGM");
			}
			AllLeverPowerOn = false;
		}
			

		pZone->Interact(packet->leverObjectID);
		break;
	}
	case SC_PACKET_TYPE::addItem: { // Ư����ġ�� �������� �߰��� ���
		SC_ADD_ITEM* packet = GetPacket<SC_ADD_ITEM>();
		
		XMFLOAT3 position = itemSpawnLocationPositions[packet->itemLocationIndex];

		pZone->AddItem(packet->objectType, packet->itemLocationIndex, packet->itemObjectID, position, _pDevice, _pCommandList);
		break;
	}
	case SC_PACKET_TYPE::removeItem: {	// Ư����ġ�� �������� ����� ���
		SC_REMOVE_ITEM* packet = GetPacket<SC_REMOVE_ITEM>();
		pZone->RemoveItem(packet->itemObjectID);

		break;
	}
	case SC_PACKET_TYPE::useItem: {	// ������ �Ƿ�ŰƮ, Ʈ���� ��ġ�� ���
		SC_USE_ITEM* packet = GetPacket<SC_USE_ITEM>();
		// ���� �������� ����� ���
		if (packet->playerObjectID == myObjectID) {
			auto pStudent = static_pointer_cast<Student>(pPlayer);
			if(packet->objectType == ObjectType::medicalKitItem) {
				
				pStudent->AddHP(50.0f);
				gameFramework.GetSoundManager().SetPosition("healSound", pStudent->GetWorldPosition());
				gameFramework.GetSoundManager().Play("healSound");

				// ��ƼŬ ����
				VS_ParticleMappedFormat particle;
				particle.boardSize = { 0.03f, 0.03f };
				particle.lifetime = 1.f;
				particle.position = pStudent->GetHeadObject()->GetWorldPosition();
				particle.type = (int)PARTICLE_TYPE::healItemUse;

				uniform_real_distribution<float> urd(-5.f, 5.f);
				for (int i = 0; i < 100; ++i) {
					particle.velocity = XMFLOAT3(urd(rd), urd(rd), urd(rd));
					Shader::AddParticle(particle);
				}

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
		}
		else {
			auto pStudent = static_pointer_cast<InterpolateMoveGameObject>(FindPlayerObject(packet->playerObjectID));
			if (packet->objectType == ObjectType::medicalKitItem) {
				pStudent->AddHP(50.0f);

				// ��ƼŬ ����
				VS_ParticleMappedFormat particle;
				particle.boardSize = { 0.03f, 0.03f };
				particle.lifetime = 1.f;
				particle.position = pStudent->GetHeadObject()->GetWorldPosition();
				particle.type = (int)PARTICLE_TYPE::healItemUse;

				uniform_real_distribution<float> urd(-5.f, 5.f);
				for (int i = 0; i < 100; ++i) {
					particle.velocity = XMFLOAT3(urd(rd), urd(rd), urd(rd));
					Shader::AddParticle(particle);
				}
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
		if (packet->playerObjectID != 0) {
			shared_ptr<InterpolateMoveGameObject> pOtherPlayer = pOtherPlayers[packet->playerObjectID];
			pZone->RemoveObject(SectorLayer::otherPlayer, packet->playerObjectID, pZone->GetIndex(pOtherPlayer->GetWorldPosition()));
			pOtherPlayers.erase(packet->playerObjectID);
		}
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
	case SC_PACKET_TYPE::transparentPlayer: {
		SC_TRANSPARENT_PLAYER* packet = GetPacket<SC_TRANSPARENT_PLAYER>();
		pOtherPlayers[packet->playerObjectID]->SetTransparent(true);
		break;
	}
	default:
		cout << "������ ��Ŷ. Ÿ�� = " << (int)packetType << "\n";
	}
}

void PlayScene::UpdateLightShaderVariables(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	// light�� �÷��̾�������� ������ (����� ����)�� �ִ� �͵��� ���� �����ͺ��� 20�� �׸���.
	XMFLOAT3 playerPos = pPlayer->GetWorldPosition();

	auto IsInside = [](float lightPosY, float playerPosY) {
		return playerPosY <= lightPosY && lightPosY <= playerPosY + 4.f;
	};
	auto Pred = [playerPos, IsInside](const shared_ptr<Light>& a, const shared_ptr<Light>& b) {
		bool isInsideA = IsInside(a->position.y, playerPos.y);
		bool isInsideB = IsInside(b->position.y, playerPos.y);
		// ������ ��ġ�� �Ѵ� �����ȿ� �ְų�, �Ѵ� �����ۿ� ���� ���
		if (isInsideA == isInsideB) {
			// �Ÿ��� �����ͺ��� ������������ ����
			return Vector3::LengthEst(playerPos, a->position) < Vector3::LengthEst(playerPos, b->position);
		}
		return isInsideA;
	};
	ranges::sort(pLights, Pred);
	//cout << "�� : " << pLights[0]->position << "\n";
	
	int nLight = (UINT)pLights.size();

	for (int i = 0; i < nLight; ++i) {
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

void PlayScene::UpdateCoolTimeText(string _coolTimeText, string _UI, float _coolTime) {
	if (pTexts[_coolTimeText]->GetEnable()) {	// ��Ÿ���� ���
		if (_coolTime <= 0.f) {	// ��Ÿ���� �������
			pTexts[_coolTimeText]->SetEnable(false);
			pUIs[_UI]->SetDark(false);
		}
		else {
			if (_coolTime >= 1.0f) { // 1�� �̻��ϰ�쿡�� ������ ����Ѵ�.

				pTexts[_coolTimeText]->SetText(to_wstring((int)_coolTime).c_str());
			}
			else {
				pTexts[_coolTimeText]->SetText(to_wstring(_coolTime).substr(0, 3).c_str());
			}
		}
	}
}

void PlayScene::UpdateCameraShaderVariables(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	camera->SetViewPortAndScissorRect(_pCommandList);
	camera->UpdateShaderVariable(_pCommandList);
}

shared_ptr<Light> PlayScene::GetLight(UINT _lightIndex) {
	return pLights[_lightIndex];
}

void PlayScene::BakeShadowMap(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, D3D12_CPU_DESCRIPTOR_HANDLE& _dsvHandle) {
	
	GameFramework& gameFramework = GameFramework::Instance();
	D3D12_CPU_DESCRIPTOR_HANDLE* rtvShadowCPUDescriptorHandles = new D3D12_CPU_DESCRIPTOR_HANDLE[1];
	float pClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	
	int nLight = (UINT)pLights.size();
	for (int i = 0; i < nLight; ++i) {
		memcpy(&pMappedLights->lights[i], pLights[i].get(), sizeof(Light));
	}

	memcpy(&pMappedLights->globalAmbient, &globalAmbient, sizeof(XMFLOAT4));
	memcpy(&pMappedLights->nLight, &nLight, sizeof(int));

	D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = pLightsBuffer->GetGPUVirtualAddress();
	_pCommandList->SetGraphicsRootConstantBufferView(2, gpuVirtualAddress);

	camera->SetViewPortAndScissorRect(_pCommandList);
	camera->UpdateShaderVariable(_pCommandList);

	int index = 0;

	// �� ���� ���ؼ� �ν��Ͻ� ������Ʈ�� �׸���.
	for (auto& pLight : pLights) {

		// ���� ������ ���̴��� �������ش�.
		_pCommandList->SetGraphicsRoot32BitConstants(14, 1, &lightIndex[index], 0);
		_pCommandList->ClearDepthStencilView(_dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
		
		rtvShadowCPUDescriptorHandles[0].ptr = pLight->bakedShadowMapCPUDescriptorHandle.ptr;
		SynchronizeResourceTransition(_pCommandList.Get(), pLight->pBakedShadowMap->GetResource(0).Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

		// ������ ������ �ִ� ������ ���� OM�ܰ迡 �����Ѵ�.
		_pCommandList->OMSetRenderTargets(1, rtvShadowCPUDescriptorHandles, TRUE, &_dsvHandle);
		_pCommandList->ClearRenderTargetView(rtvShadowCPUDescriptorHandles[0], pClearColor, 0, NULL);

		// ������ ������Ʈ���� �׷��ش�.
		gameFramework.GetShader("InstancingShadowShader")->Render(_pCommandList);

		SynchronizeResourceTransition(_pCommandList.Get(), pLight->pBakedShadowMap->GetResource(0).Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
		++index;
	}
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
		// ����ü ����
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

	// ī�޶��� y���� �ü��� ����.
	if (angle > 40.0f) {
		// ī�޶� ���� �������� ���
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

	// ����
	pPlayer->UpdateObject();
}


void PlayScene::RenderShadowMap(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, UINT _lightIndex) {

	GameFramework& gameFramework = GameFramework::Instance();

	// ���� �ε����� �����ش�.
	_pCommandList->SetGraphicsRoot32BitConstants(14, 1, &lightIndex[_lightIndex], 0);
	// �׸��ڿ� ������ �ִ� ������Ʈ���� �׸���. basic, instancing, effect..

	gameFramework.GetShader("BasicShadowShader")->Render(_pCommandList);
	gameFramework.GetShader("SkinnedShadowShader")->Render(_pCommandList);

}

void PlayScene::WireFrameRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed) {
	GameFramework& gameFramework = GameFramework::Instance();

	gameFramework.UpdateShaderVariables();
	camera->SetViewPortAndScissorRect(_pCommandList);
	camera->UpdateShaderVariable(_pCommandList);

	_pCommandList->SetGraphicsRoot32BitConstants(13, 1, &radarInfo.x, 0);

	gameFramework.GetShader("BasicWireFrameShader")->Render(_pCommandList);
	gameFramework.GetShader("InstancingWireFrameShader")->Render(_pCommandList);
	gameFramework.GetShader("SkinnedWireFrameShader")->Render(_pCommandList);

	gameFramework.GetWireFrameMap()->UpdateShaderVariable(_pCommandList);
}



void PlayScene::PreRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed) {

	GameFramework& gameFramework = GameFramework::Instance();

	gameFramework.UpdateShaderVariables();
	// �����ӿ�ũ���� ������ ���� ��Ʈ�ñ״�ó�� set

	// ���̴� Ŭ������ �������� ���ǵ� ��ũ���� ���� �����Ѵ�.

	gameFramework.GetShader("SkyBoxShader")->PrepareRender(_pCommandList);
	pSkyBox->Render(_pCommandList);

	pZone->Render(_pCommandList, pPlayer->GetCamera()->GetBoundingFrustum());

	gameFramework.GetShader("EffectShader")->Render(_pCommandList);
	Shader::RenderParticle(_pCommandList);

	// �� �׸� �� G���۸� �����Ѵ�.
	gameFramework.GetGBuffer()->UpdateShaderVariable(_pCommandList);
}

void PlayScene::LightingRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed) {
	GameFramework& gameFramework = GameFramework::Instance();


	// Ż���ϰ� �Ǹ� ȭ���� ����� �Ǹ鼭 ���� ����� �Ǹ� ������ Ż���ߴٴ� ��Ŷ�� �����鼭 ������ ������.
	if (exit && fadeOut < 3) {
		fadeOut += _timeElapsed;
		if (isPlayerProfessor)
			globalAmbient = Vector4::Add(XMFLOAT4(0.5, 0.5, 0.5, 0), Vector4::Multiply(fadeOut, XMFLOAT4(-1, -1, -1, 1)));
		else
			globalAmbient = Vector4::Add(XMFLOAT4(0.5, 0.5, 0.5, 0), Vector4::Multiply(fadeOut, XMFLOAT4(5, 5, 5, 5)));
	}

	// ������ ��� �л��� ������ ���Ѱ�� ������ ȭ���� ������� �Ǹ鼭 ������ ������.
	if (professorWin && fadeOut < 3) {
		fadeOut += _timeElapsed;
		if (isPlayerProfessor)
			globalAmbient = Vector4::Add(XMFLOAT4(0.5, 0.5, 0.5, 0), Vector4::Multiply(fadeOut, XMFLOAT4(5, 5, 5, 5)));
		else
			globalAmbient = Vector4::Add(XMFLOAT4(0.5, 0.5, 0.5, 0), Vector4::Multiply(fadeOut, XMFLOAT4(-1, -1, -1, 1)));
	}

	UpdateLightShaderVariables(_pCommandList);
	gameFramework.GetShader("LightingShader")->PrepareRender(_pCommandList);

	pFullScreenObject->Render(_pCommandList);

	// �������� ������Ʈ�� �������� �׸���.
	gameFramework.GetShader("BlendingShader")->Render(_pCommandList);

	// ���̴��� ������ ��ǻƮ ���̴��� �����ش�.
	_pCommandList->SetComputeRoot32BitConstants(4, 2, &radarInfo, 0);
}

void PlayScene::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed) {
	
	GameFramework& gameFramework = GameFramework::Instance();

	gameFramework.GetShader("PostShader")->PrepareRender(_pCommandList);
	// ��ǻƮ ���̴� ������ ���� ����� �ĸ���ۿ� �״�� �׸���.
	gameFramework.GetComputeBuffer()->UpdateShaderVariable(_pCommandList);
	pFullScreenObject->Render(_pCommandList);

	// ��� ó���� ���� �� UI�� �׸���.
	gameFramework.GetShader("UIShader")->PrepareRender(_pCommandList);
	for (auto [name, pUI] : pUIs) {
		pUI->Render(_pCommandList);
	}
	for (auto [name, pButton] : pButtons) {
		pButton->Render(_pCommandList);
	}

	//gameFramework.GetShader("BoundingMeshShader")->PrepareRender(_pCommandList);
	//pFrustumMesh->UpdateMesh(camera->GetBoundingFrustum());
	//pFrustumMesh->Render(_pCommandList);
}

void PlayScene::PostRender() {

	// ���ڴ� Render������ ���� ���� �Ŀ� ����
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

