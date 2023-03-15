#pragma once
#include "Player.h"
#include "Light.h"
#include "Button.h"
#include "Zone.h"

typedef SC_SUB_ROOM_PLAYERS_INFO Player_Info;

class Scene {
protected:
	static unordered_map<string, shared_ptr<Button>> pButtons;
	static unordered_map<string, shared_ptr<TextBox>> pTexts;

	static unordered_map<string, shared_ptr<Image2D>> pUIs;
public:
	static shared_ptr<Image2D> GetUI(string _name);
	static shared_ptr<TextBox> GetText(string _name);
	static shared_ptr<Button> GetButton(string _name);
protected:



public:
	Scene();
	virtual ~Scene();
	virtual void Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) = 0;
	virtual void ReleaseUploadBuffers() = 0;
	virtual void ProcessKeyboardInput(const array<bool, 256>& _keyDownBuffer, const array<UCHAR, 256>& _keysBuffers, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) = 0;
	virtual void AnimateObjects(char _collideCheck, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) = 0;
	virtual void ProcessSocketMessage(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) = 0;
	virtual void ProcessMouseInput(UINT _type, XMFLOAT2 _pos);
	virtual void ProcessCursorMove(XMFLOAT2 _delta);

	virtual void ReActButton(shared_ptr<Button> _pButton) = 0;
	virtual char CheckCollision(float _timeElapsed);

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
	UINT id = 0;
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

	D3D12_VIEWPORT viewPort;
	D3D12_RECT scissorRect;

	vector<SC_SUB_ROOMLIST_INFO> roomList;

	// 미리 로딩을 해놓은 후 게임이 시작되었을 때 push 한다.
	shared_ptr<PlayScene> loadingScene;

	UINT roomPage;	// 1페이지부터 시작
	RoomInfo roomInfo;
	LobbyState currState;


public:
	LobbyScene();
	~LobbyScene();
	virtual void Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void ReleaseUploadBuffers();
	virtual void ProcessKeyboardInput(const array<bool, 256>& _keyDownBuffer, const array<UCHAR, 256>& _keysBuffers, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void AnimateObjects(char _collideCheck, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void ProcessSocketMessage(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed);
	virtual void PostRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void ReActButton(shared_ptr<Button> _pButton);
	virtual void NoticeCloseToServer();
	void changeUI(LobbyState _state, bool _active);
	void UpdateReadyState();
	void UpdateRoomText();
};
class PlayScene : public Scene, public enable_shared_from_this<PlayScene> {

private:
	float remainTime;

	shared_ptr<FrustumMesh> pFrustumMesh;

	shared_ptr<Player> pPlayer;
	// 다른 플레이어들도 추가
	UINT professorObjectID;
	unordered_map<UINT, shared_ptr<InterpolateMoveGameObject>> pOtherPlayers;
	vector<shared_ptr<Computer>> pEnableComputers;
	shared_ptr<Zone> pZone;

	ComPtr<ID3D12Resource> pLightsBuffer;
	vector<shared_ptr<Light>> pLights;
	shared_ptr<LightsMappedFormat> pMappedLights;

	shared_ptr<Camera> camera;

	XMFLOAT4 globalAmbient;
	
	// 현재 상호작용 가능한 오브젝트의 포인터
	shared_ptr<InteractObject> pInteractableObject;

	shared_ptr<SkyBox> pSkyBox;
	
	vector<shared_ptr<GameObject>> pEffects;
	vector<XMFLOAT3> itemSpawnLocationPositions;

	BoundingBox exitBox;
	bool exit = false;
	float fadeOut = 0.0f;
	bool professorWin = false;

public:
	PlayScene();
	~PlayScene();

	virtual void Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void ReleaseUploadBuffers();
	virtual void ProcessKeyboardInput(const array<bool, 256>& _keyDownBuffer, const array<UCHAR, 256>& _keysBuffers, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void AnimateObjects(char _collideCheck, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void ProcessSocketMessage(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed);
	virtual void PostRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	virtual void ReActButton(shared_ptr<Button> _pButton);
	virtual void ProcessMouseInput(UINT _type, XMFLOAT2 _pos);
	virtual void ProcessCursorMove(XMFLOAT2 _delta);

	void AddComputer(const shared_ptr<Computer>& _pComputer);
	void UpdateTimeText();
	void SetPlayer(shared_ptr<Player>& _pPlayer);
	virtual char CheckCollision(float _timeElapsed);
	// 플레이어가 충돌시 충돌한 바운딩박스 면의 법선벡터를 구하는 함수
	XMFLOAT3 GetCollideNormalVector(const shared_ptr<GameObject>& _collideObj);
	void AddLight(const shared_ptr<Light>& _pLight);
	void UpdateLightShaderVariables(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	
	// 오브젝트 ID로 해당 플레이어 인스턴스 포인터를 찾아 반환
	shared_ptr<GameObject> FindPlayerObject(UINT _objectID) const;
	UINT GetProfessorObjectID() const;
	void AddItemSpawnLocation(const XMFLOAT3& _position);

	void SetExitBox(const BoundingBox& _exitBox);
};
