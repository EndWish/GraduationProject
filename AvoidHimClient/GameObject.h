#pragma once
#include "Mesh.h"


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

// �׽�Ʈ ��
static int guid = 0;

struct INSTANCING_FORMAT {
	XMFLOAT4X4 transform;
};

class GameObject : public enable_shared_from_this<GameObject> {

protected:
	int gid;
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
	//friend RigidBody;

	GameObject();
	virtual ~GameObject();
	// ���� ������Ʈ ���� ������
	
	virtual void Create();
	virtual void Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	// get set �Լ�
	const string& GetName() const;
	// �θ���ǥ����� ���͵��� ��´�.
	XMFLOAT3 GetLocalRightVector() const;
	XMFLOAT3 GetLocalUpVector() const;
	XMFLOAT3 GetLocalLookVector() const;
	XMFLOAT4 GetLocalRotate() const;
	XMFLOAT3 GetLocalPosition() const;
	// ���� �̵�
	void MoveRight(float distance);
	void Move(const XMFLOAT3& _moveVector, float _timeElapsed = 1.0f);
	void MoveUp(float distance);
	void MoveFront(float distance);

	void Rotate(const XMFLOAT3& _axis, float _angle, float _timeElapsed = 1.0f);
	void Rotate(const XMFLOAT4& _quat);
	// ������ǥ�� ���� �ڽ��� ��ġ�� �����Ѵ�.
	XMFLOAT3 GetWorldPosition() const;
	XMFLOAT3 GetWorldRightVector() const;
	XMFLOAT3 GetWorldUpVector() const;
	XMFLOAT3 GetWorldLookVector() const;

	// �ڽ��� �ٿ�� �ڽ��� ���۷����� �����Ѵ�.
	const BoundingOrientedBox& GetBoundingBox() const;

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

	void UpdateLocalTransform();
	// eachTransform�� ������ worldTransform�� ������Ʈ �Ѵ�.
	virtual void UpdateWorldTransform();
	// ��ȯ����� �����ϰ� worldTransform�� ������Ʈ �Ѵ�.

	//  OOBB ����
	void UpdateOOBB();

	// ������Ʈ ���� ��ü������ ����
	void UpdateObject();

	// �浹 üũ
	void CheckCollision(const shared_ptr<GameObject>& _other);
	//bool CheckCollisionWithTerrain(shared_ptr<TerrainMap> _pTerrain, XMFLOAT3 _moveVector);
	// �ִϸ��̼�
	shared_ptr<GameObject> FindFrame(const string& _name);
	virtual void PrepareAnimate();
	virtual void Animate(double _timeElapsed);
	virtual void Animate(double _timeElapsed, const XMFLOAT3& _playerPos);
	virtual void Remove();
	virtual bool CheckRemove() const;


	// ����
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	void RenderHitBox(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, HitBoxMesh& _hitBox);
	// ���� ��ȯ����� ���̴��� �Ѱ��ش�.
	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void UpdateHitboxShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void LoadFromFile(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const shared_ptr<GameObject>& _coverObject);
	virtual void CopyObject(const GameObject& _other);
};


class GameObjectManager {
	unordered_map<string, shared_ptr<GameObject>> storage;

public:
	shared_ptr<GameObject> GetGameObject(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};

