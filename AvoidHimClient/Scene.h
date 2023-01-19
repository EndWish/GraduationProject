#pragma once
#include "Player.h"
#include "Light.h"
#include "Button.h"
#include "Zone.h"

typedef SC_SUB_ROOM_PLAYERS_INFO Player_Info;

class Scene {
protected:

	unordered_map<string, shared_ptr<Button>> pButtons;
	unordered_map<string, shared_ptr<TextBox>> pTexts;


public:
	Scene();
	virtual ~Scene();
	virtual void Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) = 0;
	virtual void ReleaseUploadBuffers() = 0;
	virtual void ProcessKeyboardInput(const array<UCHAR, 256>& _keysBuffers, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) = 0;
	virtual void AnimateObjects(double _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) = 0;
	virtual void ProcessSocketMessage() = 0;
	virtual void ProcessMouseInput(UINT _type, XMFLOAT2 _pos);

	virtual void ReActButton(shared_ptr<Button> _pButton) = 0;
	virtual void CheckCollision();

	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed) = 0;
	virtual void PostRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void NoticeCloseToServer();
};

enum class LobbyState : unsigned char {
	title,
	roomList,
	inRoom,
};

struct RoomInfo {
	UINT host = 0;
	UINT nParticipant = 0;
	vector<Player_Info> players;
	auto findPlayerIndex(UINT _cid) {
		return ranges::find(players, _cid, &Player_Info::clientID);
	}
	void ClearRoom() {
		host = -1;
		nParticipant = 0;
		players.clear();
	}
};

class LobbyScene : public Scene {
private:
	shared_ptr<Image2D> pBackGround;
	static const int maxParticipant = 5;
	D3D12_VIEWPORT viewPort;
	D3D12_RECT scissorRect;

	unordered_map<string, shared_ptr<Image2D>> pUIs;
	vector<SC_SUB_ROOMLIST_INFO> roomList;

	UINT roomPage;	// 1페이지부터 시작
	RoomInfo roomInfo;
	LobbyState currState;

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
	virtual void ReActButton(shared_ptr<Button> _pButton);
	virtual void NoticeCloseToServer();
	void changeUI(LobbyState _state, bool _active);
	void UpdateReadyState();
	void UpdateRoomText();
};
class PlayScene : public Scene {

private:

	shared_ptr<FrustumMesh> pFrustumMesh;

	shared_ptr<Player> pPlayer;
	// 다른 플레이어들도 추가
	shared_ptr<Zone> pZone;

	ComPtr<ID3D12Resource> pLightsBuffer;
	vector<shared_ptr<Light>> pLights;
	shared_ptr<LightsMappedFormat> pMappedLights;

	shared_ptr<Camera> camera;

	XMFLOAT4 globalAmbient;
public:
	PlayScene();
	~PlayScene();

	virtual void Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void ReleaseUploadBuffers();
	virtual void ProcessKeyboardInput(const array<UCHAR, 256>& _keysBuffers, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void AnimateObjects(double _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void ProcessSocketMessage();
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed);
	virtual void ReActButton(shared_ptr<Button> _pButton);

	void SetPlayer(shared_ptr<Player> _pPlayer);
	void CheckCollision();
	void AddLight(const shared_ptr<Light>& _pLight);
	void UpdateLightShaderVariables(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};