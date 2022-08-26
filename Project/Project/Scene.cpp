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
	// 스테이지 생성
	LoadStage(_pDevice, _pCommandList);

	camera = make_shared<Camera>();
	camera->Create(_pDevice, _pCommandList);

	camera->SetLocalPosition(XMFLOAT3(0, 1.6, 0.1));
	camera->UpdateLocalTransform();
	camera->UpdateWorldTransform();

	pPlayer[0]->SetChild(camera);

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
	//XMFLOAT4X4 transform = Matrix4x4::Identity();
	//if (_keysBuffers['E'] & 0xF0) {
	//	transform = Matrix4x4::Multiply(pPlayer[0]->GetRotateMatrix(0.0f, 5.0f, 0.0f), transform);
	//}
	//if (_keysBuffers['Q'] & 0xF0) {
	//	transform = Matrix4x4::Multiply(pPlayer[0]->GetRotateMatrix(0.0f, -5.0f, 0.0f), transform);
	//}
	if (_keysBuffers['E'] & 0xF0) {
		pPlayer[0]->Rotate(XMFLOAT3(0, 1, 0), 30.0f);
	}
	if (_keysBuffers['Q'] & 0xF0) {
		pPlayer[0]->Rotate(XMFLOAT3(0, 1, 0), -30.0f);
	}
	if (_keysBuffers['W'] & 0xF0) {
		pPlayer[0]->MoveFront(1.0f);
	}
	if (_keysBuffers['S'] & 0xF0) {
		pPlayer[0]->MoveFront(-1.0f);
	}
	if (_keysBuffers['D'] & 0xF0) {
		pPlayer[0]->MoveRight(1.0f);
		
	}
	if (_keysBuffers['A'] & 0xF0) {
		pPlayer[0]->MoveRight(-1.0f);
	}
	//pPlayer[0]->ApplyTransform(transform, false);
	pPlayer[0]->UpdateLocalTransform();
	pPlayer[0]->UpdateWorldTransform();

}

void PlayScene::AnimateObjects(double _timeElapsed) {
	// 플레이어가 살아있는 경우 애니메이션을 수행
	if (!pPlayer[0]->GetIsDead()) {
		pPlayer[0]->Animate(_timeElapsed);
	}	
	/*if (!pPlayer[1]->GetIsDead()) {
		pPlayer[1]->Animate(_timeElapsed);
	}*/
	// 현재 플레이어가 있는 방, 그 방과 인접한 방만 수행
	// 각 플레이어가 있는 방이 다를 경우 따로 처리
	pNowRoom[0]->AnimateObjects(_timeElapsed);

	for (const auto& nextRoom : pNowRoom[0]->GetSideRooms()) {
		nextRoom.lock()->AnimateObjects(_timeElapsed);
	}
}
void PlayScene::CheckCollision() {
	pNowRoom[0]->CheckCollision();
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

	//pPlayer[0]->Render(_pCommandList);

	// 뷰 프러스텀 내에서 걸러지므로 
	for (const auto& room : pRooms) {
		room->Render(_pCommandList);
	}
	
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

void PlayScene::LoadStage(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	
	vector<vector<int>> sideRooms;

	for (int i = 0; ; ++i) {
		// 해당 스테이지 룸 파일들을 모두 읽는다.
		ifstream file("Room/Room" + to_string(nStage*100 + i), ios::binary);	

		if (file.fail()) break;
		
		// Room파일을 읽은 후 pRooms에 저장
		shared_ptr<Room> newRoom = make_shared<Room>();
		sideRooms.push_back(newRoom->LoadRoom(file, _pDevice, _pCommandList));
		pRooms.push_back(newRoom);
	}
	// 각 방마다 인접한 방의 주소들을 넣어준다. 
	for (int i = 0; i < sideRooms.size(); ++i) {
		for (auto rooms : sideRooms[i]) {
			pRooms[i]->GetSideRooms().push_back(pRooms[rooms]);
		}
	}
	// 플레이어의 위치는 항상 RoomNum = 0인 방의 위치(x=0, z=0)를 기준으로 설정

	pPlayer[0] = make_shared<Player>();
	pPlayer[0]->Create("Mage", _pDevice, _pCommandList);
	pNowRoom[0] = pRooms[0];
	pNowRoom[1] = pRooms[0];
	//pPlayer[1] = make_shared<Player>();
	//pPlayer[1]->Create("mage", _pDevice, _pCommandList);
}