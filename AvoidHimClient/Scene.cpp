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

void Scene::CheckCollision() {
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
}

LobbyScene::~LobbyScene()
{
}
void LobbyScene::Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	viewPort = { 0,0, C_WIDTH, C_HEIGHT, 0, 1 };
	scissorRect = { 0,0, C_WIDTH, C_HEIGHT };

	GameFramework& gameFramework = GameFramework::Instance();
	gameFramework.GetTextureManager().GetTexture("2DUI_readyButton", _pDevice, _pCommandList);
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


	name = "2DUI_optionButton";
	pButton = make_shared<Button>(name, XMFLOAT2(0.3f, 0.2f), XMFLOAT2(1.1f, 1.4f), ButtonType::option, _pDevice, _pCommandList);
	pButtons["optionButton"] = pButton;

	name = "2DUI_exitButton";
	pButton = make_shared<Button>(name, XMFLOAT2(0.3f, 0.2f), XMFLOAT2(1.1f, 1.6f), ButtonType::exit, _pDevice, _pCommandList);
	pButtons["exitButton"] = pButton;

	name = "2DUI_titleButton";
	pButton = make_shared<Button>(name, XMFLOAT2(0.3f, 0.2f), XMFLOAT2(1.6f, 1.7f), ButtonType::title, _pDevice, _pCommandList, false);
	pButtons["titleButton"] = pButton;

	name = "2DUI_makeRoom";
	pButton = make_shared<Button>(name, XMFLOAT2(0.3f, 0.2f), XMFLOAT2(1.25f, 1.7f), ButtonType::makeRoom, _pDevice, _pCommandList, false);
	pButtons["makeRoom"] = pButton;
	
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
	if (_keysBuffers['A'] & 0xF0) {
		if (roomPage > 1) roomPage--;
		UpdateRoomText();
	}
	if (_keysBuffers['D'] & 0xF0) {
		 roomPage++;
		 UpdateRoomText();
	}
}

void LobbyScene::AnimateObjects(double _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList)  {

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
		roomInfo.players.clear();
		roomInfo.host = packet.hostID;
		roomInfo.nParticipant = packet.nParticipant;

		for (int i = 0; i < roomInfo.nParticipant; ++i) {
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
		Player_Info pi{ packet.visitClientID, false };
		roomInfo.players.push_back(pi);
		roomInfo.nParticipant++;
		UpdateReadyState();
		break;
	}
	case SC_PACKET_TYPE::roomOutPlayerInfo: { // ���� �濡�� ���� ���
		SC_ROOM_OUT_PLAYER_INFO packet;
		RecvContents(packet);
		auto pindex = roomInfo.findPlayerIndex(packet.outClientID);
		roomInfo.players.erase(pindex);
		roomInfo.nParticipant--;
		UpdateReadyState();
		break;
	}
	default:
		cout << "������ ��Ŷ\n";
	}
}

void LobbyScene::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed) {
	GameFramework& gameFramework = GameFramework::Instance();

	_pCommandList->RSSetViewports(1, &viewPort);
	_pCommandList->RSSetScissorRects(1, &scissorRect);

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
	case  ButtonType::makeRoom: {

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
	case  ButtonType::quitRoom: {
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
		sPacket.visitRoomID = reinterpret_cast<RoomButton*>(_pButton.get())->GetRoomIndex();
		send(server_sock, (char*)&sPacket, sizeof(CS_QUERY_VISIT_ROOM), 0);
		break;
	}
	case ButtonType::gameStart: {
		CS_READY sPacket;
		sPacket.cid = cid;
		send(server_sock, (char*)&sPacket, sizeof(CS_READY), 0);
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

		if (_active) pBackGround->SetEnable(false);
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
	// ���� ������ ���� �� ����Ʈ�� ������ ���� ���ش�.
	bool lastRoom = false;
	UINT startIndex = 1 + (roomPage - 1) * 6;
	for (UINT i = startIndex; i < startIndex + 6; ++i) {

		UINT participant = 0;
		RoomState state = RoomState::none;
		string baseName = "RoomButton_" + to_string(i);

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
	bool bChange = false;
	for (int i = 0; i < 5; ++i) {
		pUIs["2DUI_ready_" + to_string(i + 1)]->SetEnable(false);
	}
	for (int i = 0; i < roomInfo.nParticipant; ++i) {
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
	globalAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
}

PlayScene::~PlayScene() {
	pLightsBuffer->Unmap(0, NULL);
}

void PlayScene::CheckCollision() {

}


void PlayScene::Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();
	// �������� ����
	// ���� �׷��� ������Ʈ���� ���� ����.
	gameFramework.GetGameObjectManager().GetGameObject("Gunship", _pDevice, _pCommandList);

	pPlayer = make_shared<Player>();
	pPlayer->Create("Gunship", _pDevice, _pCommandList);
	pPlayer->SetLocalScale(XMFLOAT3(2.0f, 2.0f, 2.0f));
	//pPlayer->SetLocalScale(XMFLOAT3(20.0f, 20.0f, 20.0f));
	pPlayer->UpdateObject();


	shared_ptr<Light> baseLight = make_shared<Light>();
	baseLight->lightType = 3;
	baseLight->position = XMFLOAT3(0, 500, 0);
	baseLight->direction = XMFLOAT3(0, -1, 0);
	baseLight->diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.f);
	AddLight(baseLight);

	camera = make_shared<Camera>();
	camera->Create(_pDevice, _pCommandList);

	//camera->SetLocalPosition(XMFLOAT3(0.0, 0.0, 0.0));
	camera->SetLocalPosition(XMFLOAT3(0.0, 10.0, -20));

	camera->SetLocalRotation(Vector4::QuaternionRotation(XMFLOAT3(0, 1, 0), 0.0f));

	camera->SetPlayerPos(pPlayer->GetWorldPosition());

	camera->UpdateLocalTransform();
	camera->UpdateWorldTransform();

	pPlayer->SetChild(camera);
	pPlayer->UpdateObject();

	ComPtr<ID3D12Resource> temp;
	UINT ncbElementBytes = ((sizeof(LightsMappedFormat) + 255) & ~255); //256�� ���
	pLightsBuffer = ::CreateBufferResource(_pDevice, _pCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, temp);
	
	pLightsBuffer->Map(0, NULL, (void**)&pMappedLights);

}

void PlayScene::ReleaseUploadBuffers() {
	GameFramework& gameFramework = GameFramework::Instance();
	gameFramework.GetMeshManager().ReleaseUploadBuffers();
	gameFramework.GetTextureManager().ReleaseUploadBuffers();
}


void PlayScene::ProcessKeyboardInput(const array<UCHAR, 256>& _keysBuffers, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {


	GameFramework& gameFramework = GameFramework::Instance();


}

void PlayScene::AnimateObjects(double _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	pPlayer->Animate(_timeElapsed);
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


void PlayScene::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed) {

	GameFramework& gameFramework = GameFramework::Instance();
	
	// �����ӿ�ũ���� ������ ���� ��Ʈ�ñ״�ó�� set
	camera->SetViewPortAndScissorRect(_pCommandList);
	camera->UpdateShaderVariable(_pCommandList);

	UpdateLightShaderVariables(_pCommandList);

	gameFramework.GetShader("BasicShader")->PrepareRender(_pCommandList);
	pPlayer->Render(_pCommandList);
}

void PlayScene::AddLight(const shared_ptr<Light>& _pLight) {
	pLights.push_back(_pLight);
}

