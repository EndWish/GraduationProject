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
	virtual void RenderShadowMap(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, UINT _lightIndex) = 0;
	virtual void PreRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed) = 0;
	virtual void LightingRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed);
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed) = 0;
	virtual void PostRender();
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
	bool isLoading;
	shared_ptr<Image2D> pBackGround;

	D3D12_VIEWPORT viewPort;
	D3D12_RECT scissorRect;

	vector<SC_SUB_ROOMLIST_INFO> roomList;

	// 미리 로딩을 해놓은 후 게임이 시작되었을 때 push 한다.
	shared_ptr<PlayScene> loadingScene;

	UINT roomPage;	// 1페이지부터 시작
	RoomInfo roomInfo;
	LobbyState currState;

	ComPtr<ID3D12Resource> pLightsBuffer;
	shared_ptr<Light> pLight;
	shared_ptr<LightsMappedFormat> pMappedLights;

	XMFLOAT4 globalAmbient;

	shared_ptr<Camera> pCamera;
	array<D3D12_VIEWPORT, 5> roomViewPort;
	array<shared_ptr<RoomPlayerObject>, 5> pRoomPlayerObjects;

public:
	LobbyScene();
	~LobbyScene();
	virtual void Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void ReleaseUploadBuffers();
	virtual void ProcessKeyboardInput(const array<bool, 256>& _keyDownBuffer, const array<UCHAR, 256>& _keysBuffers, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void AnimateObjects(char _collideCheck, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void ProcessSocketMessage(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	virtual void RenderShadowMap(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, UINT _lightIndex);
	virtual void PreRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed);
	

	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed);
	
	virtual void PostRender();
	virtual void ReActButton(shared_ptr<Button> _pButton);
	virtual void NoticeCloseToServer();
	void changeUI(LobbyState _state, bool _active);
	array<shared_ptr<RoomPlayerObject>, 5>& GetRoomPlayerObjects();
	void UpdateInRoomState();
	void UpdateRoomText();
	void SetBackGround(string _bgName);
	void RenderPlayerMesh(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	
};
class PlayScene : public Scene, public enable_shared_from_this<PlayScene> {

private:
	int lightIndex[MAX_LIGHTS];

	bool radarEnable;
	float radarDuration;
	// radarRange, radarRatio
	XMFLOAT2 radarInfo;

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

	// Pre Render의 결과를 이 오브젝트에 그린다.
	shared_ptr<FullScreenObject> pFullScreenObject;


	shared_ptr<TextBox> pFrustum;
	int nSector;
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
	
	virtual void RenderShadowMap(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, UINT _lightIndex);
	virtual void WireFrameRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed);
	virtual void PreRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed);
	virtual void LightingRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed);
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed);
	virtual void PostRender();

	virtual void ReActButton(shared_ptr<Button> _pButton);
	virtual void ProcessMouseInput(UINT _type, XMFLOAT2 _pos);
	virtual void ProcessCursorMove(XMFLOAT2 _delta);

	shared_ptr<FullScreenObject> GetFullScreenObject() const;
	void changeUI(bool _enable);
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
	void UpdateCoolTimeText(string _coolTime, string _UI, float coolTime);
	void UpdateCameraShaderVariables(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	shared_ptr<Light> GetLight(UINT _lightIndex);
	void BakeShadowMap(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, D3D12_CPU_DESCRIPTOR_HANDLE& _dsvHandle);
};
