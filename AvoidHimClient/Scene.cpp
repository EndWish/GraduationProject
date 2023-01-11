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
					ReActButton(name);	// �÷��̾ ��ư�� �����ٸ� ��ư�� ���� ���� ����
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

//////////////////////////

LobbyScene::LobbyScene()
{
	roomPage = 1;
}

LobbyScene::~LobbyScene()
{
}
void LobbyScene::Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	viewPort = { 0,0, 1920, 1080, 0, 1 };
	scissorRect = { 0,0, 1920, 1080 };

	
	
	string name = "2DUI_title";
	shared_ptr<Image2D> title = make_shared<Image2D>(name, XMFLOAT2(2.f, 2.f), XMFLOAT2(0.f,0.f), XMFLOAT2(1.f,1.f), _pDevice, _pCommandList);
	pUIs[name] = title;

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

	if (packetType == SC_PACKET_TYPE::giveClientID) {
		SC_GIVE_CLIENT_ID packet;
		recv(server_sock, (char*)&packet + sizeof(SC_PACKET_TYPE), sizeof(SC_GIVE_CLIENT_ID) - sizeof(SC_PACKET_TYPE), 0);
		cid = packet.clientID;
	}
	if (packetType == SC_PACKET_TYPE::roomListInfo) {
		SC_ROOMLIST_INFO packet;
		recv(server_sock, (char*)&packet + sizeof(SC_PACKET_TYPE), sizeof(SC_ROOMLIST_INFO) - sizeof(SC_PACKET_TYPE), 0);
		
		// Roomlist �� nRoom�� �� ��ŭ SC_SUB_ROOMLIST_INFO ��Ŷ�� �߰��� �Ѳ����� �޴´�.
		roomList.clear();
		roomList.resize(packet.nRoom);
		recv(server_sock, (char*)roomList.data(), sizeof(SC_SUB_ROOMLIST_INFO) * packet.nRoom, 0);
		UpdateRoomText();
	
	}

	else cout << "������ ��Ŷ\n";
}

void LobbyScene::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed) {
	GameFramework& gameFramework = GameFramework::Instance();

	_pCommandList->RSSetViewports(1, &viewPort);
	_pCommandList->RSSetScissorRects(1, &scissorRect);

	gameFramework.GetShader("UIShader")->PrepareRender(_pCommandList);
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

void LobbyScene::ReActButton(string _name) { // ���� ��ư�� ���� ���
	if (_name == "startButton") {
		pButtons["startButton"]->SetEnable(false);
		pButtons["optionButton"]->SetEnable(false);
		pButtons["exitButton"]->SetEnable(false);
		pUIs["2DUI_title"]->SetEnable(false);

		// �� ��Ͽ� ���� ��ư�� Ȱ��ȭ�Ѵ�.
		pButtons["RoomButton_1"]->SetEnable(true);
		pButtons["RoomButton_2"]->SetEnable(true);
		pButtons["RoomButton_3"]->SetEnable(true);
		pButtons["RoomButton_4"]->SetEnable(true);
		pButtons["RoomButton_5"]->SetEnable(true);
		pButtons["RoomButton_6"]->SetEnable(true);

		// �������� CS_QUERY_ROOMLIST_INFO ��Ŷ�� ���� ���� �� ����Ʈ�� �����޶�� ��û�Ѵ�.
		CS_QUERY_ROOMLIST_INFO queryRLInfo;
		queryRLInfo.cid = cid;
		send(server_sock, (char*)&queryRLInfo, sizeof(CS_QUERY_ROOMLIST_INFO), 0);
	}
}

void LobbyScene::UpdateRoomText() {
	// ���� ������ ���� �� ����Ʈ�� ������ ���� ���ش�.
	bool lastRoom = false;
	for (UINT i = 1; i <= 6; ++i) {
		UINT roomID = i - 1 + (roomPage - 1) * 6;
		UINT participant = 0;
		RoomState state = RoomState::none;
		string baseName = "RoomButton_" + to_string(i);

		// �� ������ ���� ĭ�� ��� 
		if (roomList.size() < roomID + 1) {
			state = RoomState::none;
			lastRoom = true; // ���Ŀ� ������ ����� ��� �� ������ ǥ���Ѵ�.
		}
		if (!lastRoom) {

			SC_SUB_ROOMLIST_INFO packet = roomList[roomID];
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

void PlayScene::ReActButton(string _name)
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

