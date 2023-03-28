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
class TextBox;

struct Instancing_Data {
	UINT activeInstanceCount;
	ComPtr<ID3D12Resource> resource;
	D3D12_VERTEX_BUFFER_VIEW bufferView;
};

class GameObject : public enable_shared_from_this<GameObject> {
protected:
	static unordered_map<string, Instancing_Data> instanceDatas;
public:
	static unordered_map<string, Instancing_Data>& GetInstanceDatas() { return instanceDatas; };
	static void RenderInstanceObjects(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	static void RenderShadowInstanceObjects(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
protected:
	ShaderType shaderType;
	UINT objectClass;	// 0�� GameObject, 1�� SkinnedObject, 2�� Effect
	UINT id;
	string name;

	XMFLOAT4X4 worldTransform;

	// �θ���ǥ�� ����
	XMFLOAT4X4 localTransform;
	XMFLOAT3 localPosition;
	XMFLOAT3 localScale;
	XMFLOAT4 localRotation;

	// ��ü�� ������ �ִ� ���� ������
	shared_ptr<Light> pLight;

	BoundingOrientedBox boundingBox;

	// true�ϰ�� ���� ������Ʈ���� ��� �����ϴ� �ٿ���ڽ��� ����
	bool isOOBBCover;
	BoundingOrientedBox baseOrientedBox;


	weak_ptr<GameObject> pParent;
	vector<shared_ptr<GameObject>> pChildren;

	shared_ptr<Mesh> pMesh;

	vector<shared_ptr<Material>> materials;

public:

	GameObject();
	virtual ~GameObject();
	// ���� ������Ʈ ���� ������

	virtual void Create();
	virtual void Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	// get set �Լ�
	void SetID(UINT _id) { id = _id; }
	UINT GetID() const { return id; }

	const string& GetName() const;

	// �θ���ǥ����� ���͵��� ��´�.
	XMFLOAT3 GetLocalRightVector() const;
	XMFLOAT3 GetLocalUpVector() const;
	XMFLOAT3 GetLocalLookVector() const;
	XMFLOAT4 GetLocalRotate() const;
	XMFLOAT3 GetLocalPosition() const;
	XMFLOAT3 GetLocalScale() const;

	// ���� �̵�
	void Move(const XMFLOAT3& _moveVector, float _timeElapsed = 1.0f);
	void MoveRight(float distance, float _timeElapsed = 1.0f);
	void MoveUp(float distance, float _timeElapsed = 1.0f);
	virtual void MoveFront(float distance, float _timeElapsed = 1.0f);

	void Rotate(const XMFLOAT3& _axis, float _angle, float _timeElapsed = 1.0f);
	void Rotate(const XMFLOAT4& _quat);
	void Revolve(const XMFLOAT3& _axis, float _angle);
	void SynchronousRotation(const XMFLOAT3& _axis, float _angle);
	virtual void RotateMoveHorizontal(XMFLOAT3 _dir, float _angularSpeed, float _moveSpeed);

	// ������ǥ�� ���� �ڽ��� ��ġ�� �����Ѵ�.
	XMFLOAT3 GetWorldPosition() const;
	XMFLOAT3 GetWorldRightVector() const;
	XMFLOAT3 GetWorldUpVector() const;
	XMFLOAT3 GetWorldLookVector() const;
	XMFLOAT4X4 GetWorldTransform() const;

	// �ڽ��� �ٿ�� �ڽ��� ���۷����� �����Ѵ�.
	const BoundingOrientedBox& GetBoundingBox() const;
	const BoundingOrientedBox& GetBaseBoundingBox() const;

	// �ش� �ν��Ͻ��� ���� ������Ʈ�� ����
	shared_ptr<GameObject> GetObj();
	// ��ġ�� ������ �̵���Ų��.
	void SetLocalPosition(const XMFLOAT3& _position);
	// Ư�� ȸ������ �����Ѵ�.
	void SetLocalRotation(const XMFLOAT4& _rotation);
	// Ư�� Scale���� �����Ѵ�.
	void SetLocalScale(const XMFLOAT3& _scale);
	// �ش� ������Ʈ �������� �ֻ��� �θ� ( �ٿ���ڽ� Ŀ�� )�� ����
	void SetOOBBCover(bool _isCover);

	// �ڽ��� �߰��Ѵ�.
	void SetChild(const shared_ptr<GameObject> _pChild);
	// �޽��� �����Ѵ�.
	void SetMesh(const shared_ptr<Mesh>& _pMesh);
	// �޽��� �ٿ�� �ڽ��� ������Ʈ�� �ű��.
	void SetBoundingBox(const BoundingOrientedBox& _box);

	// skinnedObject���� ����, ���
	void SetObjectClass(UINT _objectClass);
	UINT GetObjectClass();

	virtual void UpdateLocalTransform();
	// eachTransform�� ������ worldTransform�� ������Ʈ �Ѵ�.
	virtual void UpdateWorldTransform();
	// ��ȯ����� �����ϰ� worldTransform�� ������Ʈ �Ѵ�.

	//  OOBB ����
	void UpdateOOBB();


	// ������Ʈ ���� ��ü������ ����
	virtual void UpdateObject();

	ShaderType GetShaderType() const;
	void SetShaderType(ShaderType _shaderType);
	// �浹 üũ
	// �ִϸ��̼�
	shared_ptr<GameObject> FindFrame(const string& _name);	// �̸����� �ڽ�(�ڽ�����)�� ������Ʈ�� ã�´�.

	virtual void Animate(float _timeElapsed);


	// ����
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void RenderAll(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void RenderInstance(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, Instancing_Data& _instanceData);

	void RenderHitBox(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, HitBoxMesh& _hitBox);
	// ���� ��ȯ����� ���̴��� �Ѱ��ش�.
	virtual void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void UpdateHitboxShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void LoadFromFile(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const shared_ptr<GameObject>& _coverObject);
	virtual void CopyObject(const GameObject& _other);
};

///////////////////////////////////////////////////////////////////////////////
/// EffectGameObject

class Effect : public GameObject {
protected:
	UINT nIndex, row, col;	// ������ ����, ��, ��
	float curIndexTime, maxIndexTime;
	float lifeTime;

public:
	Effect();
	virtual ~Effect();

	virtual void LoadFromFile(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const shared_ptr<GameObject>& _coverObject);
	virtual void CopyObject(const GameObject& _other);

	virtual void Animate(float _timeElapsed);
	virtual void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	void SetMaxIndexTime(float _maxIndexTime) { maxIndexTime = _maxIndexTime; }
	void SetLifeTime(float _lifeTime) { lifeTime = _lifeTime; }
	float GetMaxIndexTime() const { return maxIndexTime; };
};
///////////////////////////////////////////////////////////////////////////////
/// Attack
class Attack : public GameObject {

protected:
	float lifeTime;
	AttackType attackType; // 1 = swing, 2 = throw
	UINT playerObjectID;	// ������ ���� �÷��̾��� ������Ʈ ID
	float damage;
	bool isRemove;
public:
	Attack();
	Attack(UINT _playerObjectID);
	~Attack();


	virtual void Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void Animate(float _timeElapsed);
	void Remove() { isRemove = true; };
	bool GetIsRemove() const { return isRemove; };
	AttackType GetAttackType() const { return attackType; };
	float GetDamage() const { return damage; };
	UINT GetPlayerObjectID() const { return playerObjectID; };
};

class SwingAttack : public Attack {
protected:

public:
	SwingAttack();
	SwingAttack(UINT _playerObjectID);
	~SwingAttack();
	virtual void Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void Animate(float _timeElapsed);
};

class ThrowAttack : public Attack {
protected:
	bool isStuck;
	XMFLOAT3 velocity;
	float rotateXSpeed;
	float acc;

public:
	ThrowAttack();
	ThrowAttack(UINT _playerObjectID, const XMFLOAT3& _lookVector);
	~ThrowAttack();
	virtual void Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void Animate(float _timeElapsed);
	void SetIsStuck(bool _isStuck);
	bool GetIsStuck() const;
};


///////////////////////////////////////////////////////////////////////////////
/// SkinnedGameObject
struct SkinnedWorldTransformFormat {
	array<XMFLOAT4X4, MAX_BONE> worldTransform;
};

class SkinnedGameObject : public GameObject {
protected:

	bool isTransparent; // ���� ������ ��� true
	float transparentTime; // ���� ���ӽð�
	ComPtr<ID3D12Resource> pSkinnedWorldTransformBuffer;
	shared_ptr<SkinnedWorldTransformFormat> pMappedSkinnedWorldTransform;
	vector<shared_ptr<GameObject>> pBones;
	shared_ptr<AnimationController> pAniController;

public:
	SkinnedGameObject();
	virtual ~SkinnedGameObject();

	virtual void Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void LoadFromFile(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const shared_ptr<GameObject>& _coverObject);

	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void CopyObject(const GameObject& _other, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void SetTransparent(bool _isTransparent) { isTransparent = _isTransparent; }
	bool GetTransparent() const { return isTransparent; }
	virtual void Animate(float _timeElapsed);

	void SetTransparentTime(float _transparentTime) { transparentTime = _transparentTime; }
	float GetTransparentTime() const { return transparentTime; }

	shared_ptr<AnimationController> GetAniController() { return pAniController; }

};

///////////////////////////////////////////////////////////////////////////////
/// InterpolateMoveGameObject ( �� ������Ʈ )

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
	float hp;
	shared_ptr<Sound> pFootStepSound;

	bool imprisoned;
	bool visible;

	shared_ptr<Camera> pCamera;
	shared_ptr<TextBox> nickname;

	weak_ptr<GameObject> wpHandObject;
	weak_ptr<AnimationController> wpAniController;

public:
	InterpolateMoveGameObject();
	~InterpolateMoveGameObject();
	virtual void Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	virtual void Animate(float _timeElapsed);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	
	void SetNextTransform(const XMFLOAT3& _position, const XMFLOAT4& _rotation, const XMFLOAT3& _scale);

	void SetNickname(wstring _name, bool _isProfessor);
	shared_ptr<TextBox> GetNickname()  { return nickname; };

	float GetHP() const { return hp; };
	void SetHP(float _hp) { hp = _hp; };
	void AddHP(float _hp) { hp += _hp; };

	void SetTransparent(bool _isTransparent);

	bool GetImprisoned() const { return imprisoned; }
	void SetImprisoned(bool _imprisoned) { imprisoned = _imprisoned; }

	void SetCamera(const shared_ptr<Camera>& _pCamera) { pCamera = _pCamera; }

	void SetVisible(bool _visible) { visible = _visible; };
	bool GetVisible() const { return visible; };

	bool GetTransparent();
	shared_ptr<GameObject> GetHandObject();
	shared_ptr<AnimationController> GetAniController();
};

///////////////////////////////////////////////////////////////////////////////
/// InteractObject
class InteractObject : public GameObject {
protected:
	ObjectType type;
public:
	InteractObject();
	~InteractObject();
	// �������� ��ȣ�ۿ��� ������ �������� ����
	virtual void QueryInteract() = 0;

	// ������Ʈ�� �÷��̾��� ��ȣ�ۿ�
	virtual void Interact() = 0;

	virtual bool IsEnable();

	// ��ȣ�ۿ��� ������ �� (�Ϻθ� ���)
	virtual void EndInteract();

	// �÷��̾ ��ȣ�ۿ� �õ��� �� �� �ִ� ������Ʈ���� Ȯ��
	virtual bool IsInteractable() = 0;

	ObjectType GetObjectType() const;

};
class Door : public InteractObject {
private:
	bool isPrison;
	bool isLeft;
	float openAngle;
	bool isOpen;
public:
	Door(ObjectType _type);
	~Door();
	virtual void QueryInteract();
	virtual void Interact();


	virtual void Animate(float _timeElapsed);
	virtual bool IsInteractable();
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
	virtual bool IsInteractable();
};
class Lever : public InteractObject {
private:
	bool power;
	float openAngle;
public:
	Lever();
	~Lever();
	virtual void QueryInteract();
	virtual void Interact();
	virtual bool IsInteractable();
	virtual void Animate(float _timeElapsed);
};
class Computer : public InteractObject {
private:

	bool power;			// ������ �����ִ��� ����
	float hackingRate;	// ��ŷ��
	UINT use;			// ���� ������� �÷��̾��� objectID (�̻��� 0)

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
	virtual bool IsInteractable();
};

////////////////// Item //////////////////

class Item : public GameObject {
protected:

	UINT index;	// �������� �����Ȱ��� �ε���
	bool isRemove;
	// ������ �Ǵ� ��ġ
	XMFLOAT3 basePosition;
	float cycle;
	ObjectType itemType;
	// ������ �ִϸ��̼ǿ� ���� ����

	float timeElapsed;
	float rotateSpeed;
	float moveDistance;

public:
	Item(ObjectType _objectType);
	~Item();
	virtual void Animate(float _timeElapsed);
	virtual void Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	//virtual void Use() = 0;
	void Remove() { isRemove = true; };
	bool GetIsRemove() const { return isRemove; };
	ObjectType GetType() const { return itemType; };
	void SetIndex(UINT _index) { index = _index; };
	UINT GetIndex() const { return index; };
	void SetBasePosition(const XMFLOAT3 _basePosition) { basePosition = _basePosition; };
};

class Trap : public GameObject {
protected:
	bool isRemove;
	float slowRate;
	float slowTime;
public:
	Trap();
	~Trap();
	float GetSlowRate() const { return slowRate; };
	float GetSlowTime() const { return slowTime; };
	void Remove() { isRemove = true; };
	bool GetIsRemove() const { return isRemove; };
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

//////////////// FullScreenObject //////////////
class FullScreenObject {
public:

	ComPtr<ID3D12Resource> pPositionBuffer;	// ȭ���� ��ä��� �ﰢ�� �ΰ��θ� �Ǿ��ִ�.
	ComPtr<ID3D12Resource> pPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW positionBufferView;

	ComPtr<ID3D12Resource> pTexCoordBuffer;	// ȭ���� ��ä��� �ﰢ�� �ΰ��θ� �Ǿ��ִ�.
	ComPtr<ID3D12Resource> pTexCoordUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW texCoordBufferView;

	FullScreenObject(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	~FullScreenObject();
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

private:

};




////////////////// GameObjectManager //////////////////

class GameObjectManager {
private:
	unordered_map<string, shared_ptr<GameObject>> storage;
	unordered_map<string, ComPtr<ID3D12Resource>> instanceUploadBuffers;


public:
	bool LoadGameObject(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	shared_ptr<GameObject> GetGameObject(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	shared_ptr<GameObject> GetExistGameObject(const string& _name);
	void InitInstanceResource(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, unordered_map<string, vector<XMFLOAT4X4>>& _instanceDatas);
};