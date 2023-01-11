#pragma once
#include "Player.h"
#include "Light.h"
#include "Button.h"

class Scene {
protected:
public:

	unordered_map<string, shared_ptr<Button>> pButtons;
	Scene();
	virtual ~Scene();

public:
	virtual void Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) = 0;
	virtual void ReleaseUploadBuffers() = 0;
	virtual void ProcessKeyboardInput(const array<UCHAR, 256>& _keysBuffers, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) = 0;
	virtual void AnimateObjects(double _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) = 0;
	virtual void ProcessSocketMessage() = 0;
	virtual void ProcessMouseInput(UINT _type, XMFLOAT2 _pos);

	virtual void ReActButton(string _name) = 0;
	virtual void CheckCollision();

	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed) = 0;
	virtual void PostRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};



class LobbyScene : public Scene {
private:
	static const int maxParticipant = 5;
	D3D12_VIEWPORT viewPort;
	D3D12_RECT scissorRect;
	unordered_map<string, shared_ptr<Image2D>> pUIs;
	vector<SC_SUB_ROOMLIST_INFO> roomList;
	UINT roomPage;	// 1페이지부터 시작


public:
	LobbyScene();
	~LobbyScene();
	virtual void Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void ReleaseUploadBuffers();
	virtual void ProcessKeyboardInput(const array<UCHAR, 256>& _keysBuffers, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void AnimateObjects(double _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void ProcessSocketMessage();

	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed);
	virtual void PostRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void ReActButton(string _name);

	void UpdateRoomText();
};
class PlayScene : public Scene {
public:
	PlayScene();
	~PlayScene();
private:
	shared_ptr<Player> pPlayer;
	// 다른 플레이어들도 추가

	ComPtr<ID3D12Resource> pLightsBuffer;
	vector<shared_ptr<Light>> pLights;
	shared_ptr<LightsMappedFormat> pMappedLights;

	shared_ptr<Camera> camera;

	XMFLOAT4 globalAmbient;
public:

	virtual void Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void ReleaseUploadBuffers();
	virtual void ProcessKeyboardInput(const array<UCHAR, 256>& _keysBuffers, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void AnimateObjects(double _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void ProcessSocketMessage();
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed);
	virtual void ReActButton(string _name);

	void CheckCollision();
	void AddLight(const shared_ptr<Light>& _pLight);
	void UpdateLightShaderVariables(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};