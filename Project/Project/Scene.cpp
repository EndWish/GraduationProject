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

void PlayScene::Init() {
	GameFramework& gameFramework = GameFramework::Instance();
	// 첫 스테이지에서 플레이어 생성
	if (nStage == 1) {
		pPlayer[0] = make_shared<Player>();
		pPlayer[0]->Create();
		//pPlayer[1] = make_shared<Player>();
		//pPlayer[1]->Create();
	}
	// 룸 생성
	string fileName = "Stage";
	fileName += (to_string(nStage) + ".bin");
	LoadRoomsForFile(fileName);



	// 현재 두 플레이어가 있는 방을 첫방으로 설정
	//pNowRoom[0] = pRooms[0];
	//pNowRoom[1] = pRooms[0];
	ComPtr<ID3D12Resource> temp;
	UINT ncbElementBytes = ((sizeof(LightsMappedFormat) + 255) & ~255); //256의 배수
	pLightsBuffer = ::CreateBufferResource(gameFramework.GetDevice(), gameFramework.GetCommandList(), NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, temp);

	pLightsBuffer->Map(0, NULL, (void**)&pMappedLights);
}

PlayScene::~PlayScene() {

}

void PlayScene::FrameAdvance(double _timeElapsed) {
	GameFramework& gameFramework = GameFramework::Instance();


	// 충돌검사를 진행할 방들을 체크.
	AnimateObjects(_timeElapsed);

	

	//if (pNowRoom[0]->GetType() == "Enemy" && pNowRoom[0]->GetID() == pNowRoom[1]->GetID()) {
		// 방 문이 닫힘
		// 클리어까지 다른방으로 이동 불가
	//}
	
}

void PlayScene::AnimateObjects(double _timeElapsed) {
	// 플레이어가 살아있는 경우 애니메이션을 수행
	if (!pPlayer[0]->GetIsDead()) {
		pPlayer[0]->Animate(_timeElapsed);
	}
	
	/*if (!pPlayer[1]->GetIsDead()) {
		pPlayer[1]->Animate(_timeElapsed);
	}*/

	// 씬 내의 룸들에 대해 애니메이션을 수행
	for (const auto& room : pRooms) {
		room->AnimateObjects(_timeElapsed);
	}
}

void PlayScene::UpdateLightShaderVariables(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	int nLight = pLights.size();

	for (int i = 0; i < nLight; ++i) {
		
		memcpy(&pMappedLights->lights[i], pLights[i].lock().get(), sizeof(Light));
	}

	memcpy(&pMappedLights->globalAmbient, &globalAmbient, sizeof(XMFLOAT4));
	XMFLOAT4 tmpx;
	memcpy(&tmpx, &pMappedLights->globalAmbient, sizeof(XMFLOAT4));
	cout << tmpx << " , ";

	memcpy(&pMappedLights->nLight, &nLight, sizeof(int));
	int tmp;
	memcpy(&tmp, &pMappedLights->nLight, sizeof(int));
	cout << tmp << "\n";

	D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = pLightsBuffer->GetGPUVirtualAddress();
	_pCommandList->SetGraphicsRootConstantBufferView(2, gpuVirtualAddress);

}

void PlayScene::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	// 프레임워크에서 렌더링 전에 루트시그니처를 set
	shared_ptr<Camera> pP1Camera = pPlayer[0]->GetCamera();
	pP1Camera->SetViewPortAndScissorRect();
	pP1Camera->UpdateShaderVariable();

	UpdateLightShaderVariables(_pCommandList);

	pPlayer[0]->Render(_pCommandList);
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