#pragma once
#include "Mesh.h"
#include "Animation.h"
#include "Sound.h"

#define RESOURCE_TEXTURE1D			0x01
#define RESOURCE_TEXTURE2D			0x02
#define RESOURCE_TEXTURE2D_ARRAY	0x03	//[]
#define RESOURCE_TEXTURE2DARRAY		0x04
#define RESOURCE_TEXTURE_CUBE		0x05
#define RESOURCE_BUFFER				0x06
#define RESOURCE_STRUCTURED_BUFFER	0x07


class Light;
class Player;
class TerrainMap;
class Camera;

struct Instancing_Data {
	UINT activeInstanceCount;
	ComPtr<ID3D12Resource> resource;
	D3D12_VERTEX_BUFFER_VIEW bufferView;
	XMFLOAT4X4* mappedResource;
};

class GameObject : public enable_shared_from_this<GameObject> {
protected:
	static unordered_map<string, Instancing_Data> instanceDatas;
public:
	static unordered_map<string, Instancing_Data>& GetInstanceDatas() { return instanceDatas; };
	static void RenderInstanceObjects(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
protected:
	ShaderType shaderType;
	bool isSkinnedObject;
	UINT id;
	string name;

	XMFLOAT4X4 worldTransform;

	// 부모좌표계 기준
	XMFLOAT4X4 localTransform;
	XMFLOAT3 localPosition;
	XMFLOAT3 localScale;
	XMFLOAT4 localRotation;

	// 물체가 가지고 있는 빛의 포인터
	shared_ptr<Light> pLight;

	BoundingOrientedBox boundingBox;

	// true일경우 하위 오브젝트들을 모두 포함하는 바운딩박스를 가짐
	bool isOOBBCover;
	BoundingOrientedBox baseOrientedBox;


	weak_ptr<GameObject> pParent;
	vector<shared_ptr<GameObject>> pChildren;

	shared_ptr<Mesh> pMesh;

	vector<shared_ptr<Material>> materials;

public:

	GameObject();
	virtual ~GameObject();
	// 게임 오브젝트 복사 생성자
	
	virtual void Create();
	virtual void Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	// get set 함수
	void SetID(UINT _id) { id = _id; }
	UINT GetID() const { return id; }

	const string& GetName() const;

	// 부모좌표계기준 벡터들을 얻는다.
	XMFLOAT3 GetLocalRightVector() const;
	XMFLOAT3 GetLocalUpVector() const;
	XMFLOAT3 GetLocalLookVector() const;
	XMFLOAT4 GetLocalRotate() const;
	XMFLOAT3 GetLocalPosition() const;

	// 로컬 이동
	void Move(const XMFLOAT3& _moveVector, float _timeElapsed = 1.0f);
	void MoveRight(float distance, float _timeElapsed = 1.0f);
	void MoveUp(float distance, float _timeElapsed = 1.0f);
	void MoveFront(float distance, float _timeElapsed = 1.0f);

	void Rotate(const XMFLOAT3& _axis, float _angle, float _timeElapsed = 1.0f);
	void Rotate(const XMFLOAT4& _quat);
	void Revolve(const XMFLOAT3& _axis, float _angle);
	void SynchronousRotation(const XMFLOAT3& _axis, float _angle);
	virtual void RotateMoveHorizontal(XMFLOAT3 _dir, float _angularSpeed, float _moveSpeed);

	// 월드좌표계 기준 자신의 위치를 리턴한다.
	XMFLOAT3 GetWorldPosition() const;
	XMFLOAT3 GetWorldRightVector() const;
	XMFLOAT3 GetWorldUpVector() const;
	XMFLOAT3 GetWorldLookVector() const;
	XMFLOAT4X4 GetWorldTransform() const;

	// 자신의 바운딩 박스의 래퍼런스를 리턴한다.
	const BoundingOrientedBox& GetBoundingBox() const;
	const BoundingOrientedBox& GetBaseBoundingBox() const;

	// 해당 인스턴스가 가진 오브젝트의 정보
	shared_ptr<GameObject> GetObj();
	// 위치를 강제로 이동시킨다.
	void SetLocalPosition(const XMFLOAT3& _position);
	// 특정 회전값을 대입한다.
	void SetLocalRotation(const XMFLOAT4& _rotation);
	// 특정 Scale값을 대입한다.
	void SetLocalScale(const XMFLOAT3& _scale);
	// 해당 오브젝트 프레임을 최상위 부모 ( 바운딩박스 커버 )로 설정
	void SetOOBBCover(bool _isCover);

	// 자식을 추가한다.
	void SetChild(const shared_ptr<GameObject> _pChild);
	// 메쉬를 설정한다.
	void SetMesh(const shared_ptr<Mesh>& _pMesh);
	// 메쉬의 바운딩 박스를 오브젝트로 옮긴다.
	void SetBoundingBox(const BoundingOrientedBox& _box);

	// skinnedObject인지 설정, 얻기
	void SetSkinnedObject(bool _isSkinnedObject);
	bool IsSkinnedObject();

	void UpdateLocalTransform();
	// eachTransform를 가지고 worldTransform를 업데이트 한다.
	virtual void UpdateWorldTransform();
	// 변환행렬을 적용하고 worldTransform을 업데이트 한다.

	//  OOBB 갱신
	void UpdateOOBB();

	// 오브젝트 내용 전체적으로 갱신
	virtual void UpdateObject();

	ShaderType GetShaderType() const;
	void SetShaderType(ShaderType _shaderType);
	// 충돌 체크
	// 애니메이션
	shared_ptr<GameObject> FindFrame(const string& _name);	// 이름으로 자식(자신포함)을 오브젝트를 찾는다.
	virtual void PrepareAnimate();
	virtual void Animate(float _timeElapsed);
	virtual void Remove();
	virtual bool CheckRemove() const;


	// 렌더
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void RenderAll(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void RenderInstance(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, Instancing_Data& _instanceData);

	void RenderHitBox(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, HitBoxMesh& _hitBox);
	// 월드 변환행렬을 쉐이더로 넘겨준다.
	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void UpdateHitboxShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void LoadFromFile(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const shared_ptr<GameObject>& _coverObject);
	virtual void CopyObject(const GameObject& _other);
};


///////////////////////////////////////////////////////////////////////////////
/// SkinnedGameObject
struct SkinnedWorldTransformFormat {
	array<XMFLOAT4X4, MAX_BONE> worldTransform;
};

class SkinnedGameObject : public GameObject {
protected:
	static ComPtr<ID3D12Resource> pSkinnedWorldTransformBuffer;
	static shared_ptr<SkinnedWorldTransformFormat> pMappedSkinnedWorldTransform;
public:
	static void InitSkinnedWorldTransformBuffer(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

protected:
	vector<shared_ptr<GameObject>> pBones;
	AnimationController aniController;

public:
	SkinnedGameObject();
	virtual ~SkinnedGameObject();

	virtual void LoadFromFile(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const shared_ptr<GameObject>& _coverObject);
	
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void CopyObject(const GameObject& _other);
};

class GameObjectManager {
private:
	unordered_map<string, shared_ptr<GameObject>> storage;
	unordered_map<string, ComPtr<ID3D12Resource>> instanceUploadBuffers;


public:
	shared_ptr<GameObject> GetGameObject(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	shared_ptr<GameObject> GetExistGameObject(const string& _name);
	void InitInstanceResource(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, unordered_map<string, vector<XMFLOAT4X4>>& _instanceDatas);
};

class InterpolateMoveGameObject : public GameObject {
private:
	float moveDistance;
	XMFLOAT3 prevPosition;
	XMFLOAT4 prevRotation;
	XMFLOAT3 prevScale;

	XMFLOAT3 nextPosition;
	XMFLOAT4 nextRotation;
	XMFLOAT3 nextScale;

	float t;

	shared_ptr<Sound> pFootStepSound;
public:
	InterpolateMoveGameObject();
	~InterpolateMoveGameObject();
	virtual void Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	virtual void Animate(float _timeElapsed);
	void SetNextTransform(const XMFLOAT3& _position, const XMFLOAT4& _rotation, const XMFLOAT3& _scale);
};

class InteractObject : public GameObject { 
private:

public:
	InteractObject();
	~InteractObject();
	// 서버에게 상호작용이 가능한 상태인지 질의
	virtual void QueryInteract() = 0;

	// 오브젝트와 플레이어의 상호작용
	virtual void Interact() = 0;

	virtual bool IsEnable();
	
	// 상호작용을 끝냈을 때 (일부만 사용)
	virtual void EndInteract();

};

class Door : public InteractObject {
private:
	bool isLeft;
	float openAngle;
	bool isOpen;
public:
	Door(ObjectType _type);
	~Door();
	virtual void QueryInteract();
	virtual void Interact();


	virtual void Animate(float _timeElapsed);
};

class WaterDispenser : public InteractObject {
protected:
	float coolTime;

public:
	WaterDispenser();
	~WaterDispenser();
	virtual void QueryInteract();
	virtual void Interact();
	virtual bool IsEnable();

	virtual void Animate(float _timeElapsed);

};

class Lever : public InteractObject {
private:

public:
	Lever();
	~Lever();
	virtual void QueryInteract();
	virtual void Interact();

};

class Computer : public InteractObject {
private:

	bool power;			// 전원이 켜져있는지 여부
	float hackingRate;	// 해킹률
	UINT use;			// 현재 사용중인 플레이어의 objectID (미사용시 0)
	
public:
	Computer();
	~Computer();
	virtual void QueryInteract();
	virtual void Interact();
	virtual bool IsEnable();
	void SetHackingRate(float _rate);
	void SetUse(UINT _use);
	void SetPower(bool _power);

	virtual void Animate(float _timeElapsed);
	virtual void EndInteract();
	float GetHackingRate() const;
	UINT GetUse() const;
};


////////////////// SkyBox //////////////////


class SkyBox {

	array<shared_ptr<SkyBoxMesh>, 6> pMeshs;
	array<shared_ptr<Texture>, 6> pTextures;
public:
	SkyBox(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	~SkyBox();
		
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};
