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
	// 첫 스테이지에서 플레이어 생성
	if (nStage == 1) {
		pPlayer[0] = make_shared<Player>();
		pPlayer[0]->Create("dummy", _pDevice, _pCommandList);
		//pPlayer[1] = make_shared<Player>();
		//pPlayer[1]->Create();
		cout << "더미 성공\n";

		//[임시]

		cubeObject = make_shared<GameObject>();
		cubeObject->Create("mage", _pDevice, _pCommandList);
		cubeObject->eachTransform._41 = 10;
		cubeObject->UpdateWorldTransform();
	}
	// 룸 생성
	string fileName = "Stage";
	fileName += (to_string(nStage) + ".bin");
	LoadRoomsForFile(fileName);

	camera = make_shared<Camera>();
	camera->Create(_pDevice, _pCommandList);

	camera->SetEachPosition(XMFLOAT3(0, 0, -10));

	// 현재 두 플레이어가 있는 방을 첫방으로 설정
	//pNowRoom[0] = pRooms[0];
	//pNowRoom[1] = pRooms[0];
	ComPtr<ID3D12Resource> temp;
	UINT ncbElementBytes = ((sizeof(LightsMappedFormat) + 255) & ~255); //256의 배수
	pLightsBuffer = ::CreateBufferResource(_pDevice, _pCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, temp);

	pLightsBuffer->Map(0, NULL, (void**)&pMappedLights);
}

PlayScene::~PlayScene() {

}

void PlayScene::ProcessKeyboardInput(const array<UCHAR, 256>& _keysBuffers) {
	// 회전과 스케일링은 앞쪽에 move는 뒤쪽에 곱한다.
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
	// 플레이어가 살아있는 경우 애니메이션을 수행
	if (!pPlayer[0]->GetIsDead()) {
		pPlayer[0]->Animate(_timeElapsed);
	}
	
	/*if (!pPlayer[1]->GetIsDead()) {
		pPlayer[1]->Animate(_timeElapsed);
	}*/

	auto t = cubeObject->pChildren[0]->pChildren[5];
	t->ApplyTransform(t->GetRotateMatrix(Vector4::QuaternionRotationAxis(XMFLOAT3(0, 0,1), 10.0f)));
	//auto t = cubeObject->pChildren[0]->pChildren[8];

	// 씬 내의 룸들에 대해 애니메이션을 수행
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
	// 프레임워크에서 렌더링 전에 루트시그니처를 set
	//shared_ptr<Camera> pP1Camera = pPlayer[0]->GetCamera();
	camera->SetViewPortAndScissorRect(_pCommandList);
	camera->UpdateShaderVariable(_pCommandList);

	UpdateLightShaderVariables(_pCommandList);

	pPlayer[0]->Render(_pCommandList);
	cubeObject->Render(_pCommandList);
	
	// 뷰 프러스텀 내에서 걸러지므로 
	for (const auto& room : pRooms) {
		room->Render(_pCommandList);
	}
	
}

void PlayScene::LoadRoomsForFile(string _fileName) {

	// 스테이지 내 룸의 개수만큼 룸 배열의 공간 할당
	// pRooms.resize(룸 크기);

	// 스테이지 파일 내에 룸 파일의 이름이 있음

	// 룸의 월드좌표 기준 좌표(Center), 방의 넓이(Extent)를 불러옴

	// 룸 내 오브젝트들을 로드

	// 룸과 인접한 룸들을 담음
}

void PlayScene::AddLight(const shared_ptr<Light>& _pLight) {
	pLights.push_back(_pLight);
}

void PlayScene::CheckCurrentRoom(const BoundingOrientedBox& _playerOOBB, int _playerNum) {

	// 먼저 기존에 존재했던 방과 먼저 충돌체크
	if (pNowRoom[_playerNum]->GetBoundingBox().Contains(XMLoadFloat3(&_playerOOBB.Center)) != DISJOINT) {	// 충돌할 경우
	}
	// 기존 방에서 인접해있던 방과 충돌체크
	else {
		for (const auto& room : pNowRoom[_playerNum]->GetSideRooms()) {
			if (room.lock()->GetBoundingBox().Contains(XMLoadFloat3(&_playerOOBB.Center)) != DISJOINT) {
				pNowRoom[_playerNum] = room.lock();
			}
		}
	}
}