#include "stdafx.h"
#include "Scene.h"
#include "Timer.h"
#include "GameFramework.h"

Scene::Scene() {
	
}

Scene::~Scene() {
	
}

///////////////////////////////////////////////////////////////////////////////
/// PlayScene
PlayScene::PlayScene(int _stageNum) {

	nStage = _stageNum;
	globalAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
}

void PlayScene::Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();
	// ù ������������ �÷��̾� ����
	if (nStage == 1) {
		pPlayer[0] = make_shared<Player>();
		pPlayer[0]->Create("dummy", _pDevice, _pCommandList);
		//pPlayer[1] = make_shared<Player>();
		//pPlayer[1]->Create();
		cout << "���� ����\n";

		//[�ӽ�]

		cubeObject = make_shared<GameObject>();
		cubeObject->Create("mage", _pDevice, _pCommandList);
		cubeObject->eachTransform._41 = 10;
		cubeObject->UpdateWorldTransform();
	}
	// �� ����
	string fileName = "Stage";
	fileName += (to_string(nStage) + ".bin");
	LoadRoomsForFile(fileName);

	camera = make_shared<Camera>();
	camera->Create(_pDevice, _pCommandList);

	camera->SetEachPosition(XMFLOAT3(0, 0, -10));

	// ���� �� �÷��̾ �ִ� ���� ù������ ����
	//pNowRoom[0] = pRooms[0];
	//pNowRoom[1] = pRooms[0];
	ComPtr<ID3D12Resource> temp;
	UINT ncbElementBytes = ((sizeof(LightsMappedFormat) + 255) & ~255); //256�� ���
	pLightsBuffer = ::CreateBufferResource(_pDevice, _pCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, temp);

	pLightsBuffer->Map(0, NULL, (void**)&pMappedLights);
}

PlayScene::~PlayScene() {

}

void PlayScene::ProcessKeyboardInput(const array<UCHAR, 256>& _keysBuffers) {
	// ȸ���� �����ϸ��� ���ʿ� move�� ���ʿ� ���Ѵ�.
	XMFLOAT4X4 transform = Matrix4x4::Identity();
	if (_keysBuffers['E'] & 0xF0) {
		transform = Matrix4x4::Multiply(pPlayer[0]->GetRotateMatrix(0.0f, 5.0f, 0.0f), transform);
	}
	if (_keysBuffers['Q'] & 0xF0) {
		transform = Matrix4x4::Multiply(pPlayer[0]->GetRotateMatrix(0.0f, -5.0f, 0.0f), transform);
	}
	pPlayer[0]->ApplyTransform(transform);
	transform = Matrix4x4::Identity();
	if (_keysBuffers['W'] & 0xF0) {
		transform = Matrix4x4::Multiply(transform, pPlayer[0]->GetFrontMoveMatrix(1.0f));
	}
	if (_keysBuffers['A'] & 0xF0) {
		transform = Matrix4x4::Multiply(transform, pPlayer[0]->GetRightMoveMatrix(-1.0f));
	}
	if (_keysBuffers['S'] & 0xF0) {
		transform = Matrix4x4::Multiply(transform, pPlayer[0]->GetFrontMoveMatrix(-1.0f));
	}
	if (_keysBuffers['D'] & 0xF0) {
		transform = Matrix4x4::Multiply(transform, pPlayer[0]->GetRightMoveMatrix(1.0f));
	}
	pPlayer[0]->ApplyTransform(transform, false);
}

void PlayScene::AnimateObjects(double _timeElapsed) {
	// �÷��̾ ����ִ� ��� �ִϸ��̼��� ����
	if (!pPlayer[0]->GetIsDead()) {
		pPlayer[0]->Animate(_timeElapsed);
	}
	
	/*if (!pPlayer[1]->GetIsDead()) {
		pPlayer[1]->Animate(_timeElapsed);
	}*/

	auto t = cubeObject->pChildren[0]->pChildren[5];
	t->ApplyTransform(t->GetRotateMatrix(Vector4::QuaternionRotationAxis(XMFLOAT3(0, 0,1), 10.0f)));
	//auto t = cubeObject->pChildren[0]->pChildren[8];

	// �� ���� ��鿡 ���� �ִϸ��̼��� ����
	for (const auto& room : pRooms) {
		room->AnimateObjects(_timeElapsed);
	}
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
	//shared_ptr<Camera> pP1Camera = pPlayer[0]->GetCamera();
	camera->SetViewPortAndScissorRect(_pCommandList);
	camera->UpdateShaderVariable(_pCommandList);

	UpdateLightShaderVariables(_pCommandList);

	pPlayer[0]->Render(_pCommandList);
	cubeObject->Render(_pCommandList);
	
	// �� �������� ������ �ɷ����Ƿ� 
	for (const auto& room : pRooms) {
		room->Render(_pCommandList);
	}
	
}

void PlayScene::LoadRoomsForFile(string _fileName) {

	// �������� �� ���� ������ŭ �� �迭�� ���� �Ҵ�
	// pRooms.resize(�� ũ��);

	// �������� ���� ���� �� ������ �̸��� ����

	// ���� ������ǥ ���� ��ǥ(Center), ���� ����(Extent)�� �ҷ���

	// �� �� ������Ʈ���� �ε�

	// ��� ������ ����� ����
}

void PlayScene::AddLight(const shared_ptr<Light>& _pLight) {
	pLights.push_back(_pLight);
}

void PlayScene::CheckCurrentRoom(const BoundingOrientedBox& _playerOOBB, int _playerNum) {

	// ���� ������ �����ߴ� ��� ���� �浹üũ
	if (pNowRoom[_playerNum]->GetBoundingBox().Contains(XMLoadFloat3(&_playerOOBB.Center)) != DISJOINT) {	// �浹�� ���
	}
	// ���� �濡�� �������ִ� ��� �浹üũ
	else {
		for (const auto& room : pNowRoom[_playerNum]->GetSideRooms()) {
			if (room.lock()->GetBoundingBox().Contains(XMLoadFloat3(&_playerOOBB.Center)) != DISJOINT) {
				pNowRoom[_playerNum] = room.lock();
			}
		}
	}
}