#include "stdafx.h"
#include "Scene.h"
#include "Timer.h"
#include "GameFramework.h"

Scene::Scene() {
	
}

Scene::~Scene() {
	
}

void Scene::CheckCollision() {

}

///////////////////////////////////////////////////////////////////////////////
/// PlayScene
PlayScene::PlayScene(int _stageNum) {

	nStage = _stageNum;
	globalAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
}

void PlayScene::Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();
	// �������� ����
	LoadStage(_pDevice, _pCommandList);

	camera = make_shared<Camera>();
	camera->Create(_pDevice, _pCommandList);

	camera->SetLocalPosition(XMFLOAT3(0, 10.0, 0.1));
	camera->SetLocalRotation(Vector4::QuaternionRotation(XMFLOAT3(1, 0, 0), 90.0f));
	camera->UpdateLocalTransform();
	camera->UpdateWorldTransform();

	//pPlayers[0]->SetChild(camera);

	// ���� �� �÷��̾ �ִ� ���� ù������ ����
	//pNowRooms[0] = pRooms[0];
	//pNowRooms[1] = pRooms[0];
	ComPtr<ID3D12Resource> temp;
	UINT ncbElementBytes = ((sizeof(LightsMappedFormat) + 255) & ~255); //256�� ���
	pLightsBuffer = ::CreateBufferResource(_pDevice, _pCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, temp);

	pLightsBuffer->Map(0, NULL, (void**)&pMappedLights);
}

PlayScene::~PlayScene() {

}

void PlayScene::ProcessKeyboardInput(const array<UCHAR, 256>& _keysBuffers) {
	// ȸ���� �����ϸ��� ���ʿ� move�� ���ʿ� ���Ѵ�.
	//XMFLOAT4X4 transform = Matrix4x4::Identity();
	//if (_keysBuffers['E'] & 0xF0) {
	//	transform = Matrix4x4::Multiply(pPlayers[0]->GetRotateMatrix(0.0f, 5.0f, 0.0f), transform);
	//}
	//if (_keysBuffers['Q'] & 0xF0) {
	//	transform = Matrix4x4::Multiply(pPlayers[0]->GetRotateMatrix(0.0f, -5.0f, 0.0f), transform);
	//}
	if (_keysBuffers['E'] & 0xF0) {
		pPlayers[0]->RotateRigid(XMFLOAT3(0, 1, 0), 6.0f);
	}
	if (_keysBuffers['Q'] & 0xF0) {
		pPlayers[0]->RotateRigid(XMFLOAT3(0, 1, 0), -6.0f);
	}
	if (_keysBuffers['W'] & 0xF0) {
		pPlayers[0]->MoveFrontRigid(true);
	}
	if (_keysBuffers['S'] & 0xF0) {
		pPlayers[0]->MoveFrontRigid(false);
	}
	if (_keysBuffers['D'] & 0xF0) {
		pPlayers[0]->MoveRightRigid(true);
	}
	if (_keysBuffers['A'] & 0xF0) {
		pPlayers[0]->MoveRightRigid(false);
	}
	if (_keysBuffers['J'] & 0xF0) {
		if (pPlayers[0]->GetWorldPosition().y == 0) {
			pPlayers[0]->Jump(10.0f);
		}
	}
	//pPlayers[0]->ApplyTransform(transform, false);
	

}

void PlayScene::AnimateObjects(double _timeElapsed) {
	// �÷��̾ ����ִ� ��� �ִϸ��̼��� ����
	if (!pPlayers[0]->GetIsDead()) {
		pPlayers[0]->Animate(_timeElapsed);
	}		
	/*if (!pPlayers[1]->GetIsDead()) {
		pPlayers[1]->Animate(_timeElapsed);
	}*/
	// ���� �÷��̾ �ִ� ��, �� ��� ������ �游 ����
	// �� �÷��̾ �ִ� ���� �ٸ� ��� ���� ó��
	pNowRooms[0]->AnimateObjects(_timeElapsed);

	for (const auto& nextRoom : pNowRooms[0]->GetSideRooms()) {
		nextRoom.lock()->AnimateObjects(_timeElapsed);
	}
}
void PlayScene::CheckCollision() {
	pNowRooms[0]->CheckCollision();

	pPlayers[0]->InitVector();
}

void PlayScene::UpdateLightShaderVariables(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	int nLight = (UINT)pLights.size();

	for (int i = 0; i < nLight; ++i) {
		
		memcpy(&pMappedLights->lights[i], pLights[i].lock().get(), sizeof(Light));
	}

	memcpy(&pMappedLights->globalAmbient, &globalAmbient, sizeof(XMFLOAT4));

	memcpy(&pMappedLights->nLight, &nLight, sizeof(int));

	D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = pLightsBuffer->GetGPUVirtualAddress();
	_pCommandList->SetGraphicsRootConstantBufferView(2, gpuVirtualAddress);

}

void PlayScene::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	// �����ӿ�ũ���� ������ ���� ��Ʈ�ñ״�ó�� set
	//shared_ptr<Camera> pP1Camera = pPlayers[0]->GetCamera();
	camera->SetViewPortAndScissorRect(_pCommandList);
	camera->UpdateShaderVariable(_pCommandList);

	UpdateLightShaderVariables(_pCommandList);


	GameFramework& gameFramework = GameFramework::Instance();
	Mesh::GetShader()->PrepareRender(_pCommandList);

	//pPlayers[0]->Render(_pCommandList);
	for (const auto& room : pRooms) {
		room->Render(_pCommandList);
	}
	
	HitBoxMesh::GetShader()->PrepareRender(_pCommandList);
	HitBoxMesh& hitBoxMesh = GameFramework::Instance().GetMeshManager().GetHitBoxMesh();
	pPlayers[0]->RenderHitBox(_pCommandList, hitBoxMesh);
	if (gameFramework.GetDrawHitBox()) {
		for (const auto& room : pRooms) {
			room->RenderHitBox(_pCommandList, hitBoxMesh);
		}
	}
}

void PlayScene::AddLight(const shared_ptr<Light>& _pLight) {
	pLights.push_back(_pLight);
}

void PlayScene::CheckCurrentRoom(const BoundingOrientedBox& _playerOOBB, int _playerNum) {

	// ���� ������ �����ߴ� ��� ���� �浹üũ
	if (pNowRooms[_playerNum]->GetBoundingBox().Contains(XMLoadFloat3(&_playerOOBB.Center)) != DISJOINT) {	// �浹�� ���
	}
	// ���� �濡�� �������ִ� ��� �浹üũ
	else {
		for (const auto& room : pNowRooms[_playerNum]->GetSideRooms()) {
			if (room.lock()->GetBoundingBox().Contains(XMLoadFloat3(&_playerOOBB.Center)) != DISJOINT) {
				pNowRooms[_playerNum] = room.lock();
			}
		}
	}
}

void PlayScene::LoadStage(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	
	pPlayers[0] = make_shared<Player>();
	pPlayers[0]->Create("Mage", _pDevice, _pCommandList);
	pPlayers[1] = make_shared<Player>();
	pPlayers[1]->Create("Mage", _pDevice, _pCommandList);
	vector<vector<int>> sideRooms;

	for (int i = 0; ; ++i) {
		// �ش� �������� �� ���ϵ��� ��� �д´�.
		ifstream file("Room/Room" + to_string(nStage*100 + i), ios::binary);	

		if (file.fail()) break;
		
		// Room������ ���� �� pRooms�� ����
		shared_ptr<Room> newRoom = make_shared<Room>();
		sideRooms.push_back(newRoom->LoadRoom(file, _pDevice, _pCommandList));
		newRoom->SetPlayer(pPlayers);
		pRooms.push_back(newRoom);
	}
	// �� �渶�� ������ ���� �ּҵ��� �־��ش�. 
	for (int i = 0; i < sideRooms.size(); ++i) {
		for (auto rooms : sideRooms[i]) {
			pRooms[i]->GetSideRooms().push_back(pRooms[rooms]);
		}
	}
	// �÷��̾��� ��ġ�� �׻� RoomNum = 0�� ���� ��ġ(x=0, z=0)�� �������� ����


	pNowRooms[0] = pRooms[0];
	pNowRooms[1] = pRooms[0];
	//pPlayers[1] = make_shared<Player>();
	//pPlayers[1]->Create("mage", _pDevice, _pCommandList);
}