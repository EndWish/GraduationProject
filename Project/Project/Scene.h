#pragma once
#include "Room.h"
#include "Player.h"
#include "Light.h"

class Scene {
protected:

public:
	Scene();
	virtual ~Scene();

public:
	virtual void Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) = 0;
	virtual void ProcessKeyboardInput(const array<UCHAR, 256>& _keysBuffers) = 0;
	virtual void AnimateObjects(double _timeElapsed) = 0;
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// PlayScene
class PlayScene : public Scene {
private:
	int nStage;
	// 룸(방) 포인터의 배열
	vector<shared_ptr<Room>> pRooms;

	// 플레이어의 포인터. 첫 플레이 씬 생성 시에 플레이어가 생성되어 저장
	// 스테이지(씬) 전환 시에 그 씬으로 플레이어 포인터를	넘겨줌
	array<shared_ptr<Player>, 2> pPlayer;

	// 현재 플레이어가 있는 룸의 포인터
	array<shared_ptr<Room> , 2> pNowRoom;

	shared_ptr<GameObject> cubeObject;	//[임시]

	ComPtr<ID3D12Resource> pLightsBuffer;
	vector<weak_ptr<Light>> pLights;
	shared_ptr<LightsMappedFormat> pMappedLights;

	XMFLOAT4 globalAmbient;
	shared_ptr<Camera> camera;
public:
	PlayScene(int _stageNum);
	~PlayScene() final;

public:
	void Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) final;
	void ProcessKeyboardInput(const array<UCHAR, 256>& _keysBuffers) final;
	void AnimateObjects(double _timeElapsed) final;
	void UpdateLightShaderVariables(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) final;

	void AddLight(const shared_ptr<Light>& _pLight);

	// 현재 플레이어가 속해있는 방 ( 충돌검사를 진행할 방 ) 을 찾는 함수
	void CheckCurrentRoom(const BoundingOrientedBox& _playerOOBB, int _playerNum);
	void LoadStage(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};