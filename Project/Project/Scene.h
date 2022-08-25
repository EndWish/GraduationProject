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
	// ��(��) �������� �迭
	vector<shared_ptr<Room>> pRooms;

	// �÷��̾��� ������. ù �÷��� �� ���� �ÿ� �÷��̾ �����Ǿ� ����
	// ��������(��) ��ȯ �ÿ� �� ������ �÷��̾� �����͸�	�Ѱ���
	array<shared_ptr<Player>, 2> pPlayer;

	// ���� �÷��̾ �ִ� ���� ������
	array<shared_ptr<Room> , 2> pNowRoom;

	shared_ptr<GameObject> cubeObject;	//[�ӽ�]

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

	// ���� �÷��̾ �����ִ� �� ( �浹�˻縦 ������ �� ) �� ã�� �Լ�
	void CheckCurrentRoom(const BoundingOrientedBox& _playerOOBB, int _playerNum);
	void LoadStage(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};