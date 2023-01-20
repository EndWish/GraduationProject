#include "stdafx.h"
#include "GameObject.h"
#include "Light.h"
#include "GameFramework.h"


unordered_map<string, Instancing_Data> GameObject::instanceDatas;
unordered_map<string, UINT> GameObject::drawInstanceCount;

//////////////////////////////////////////


void GameObject::RenderInstanceObjects(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();
	GameObjectManager& gameObjManager = gameFramework.GetGameObjectManager();

	for (auto& [name, instanceData] : instanceDatas) {
		// �ش� �ν��Ͻ��� ������Ʈ ������ �����´�.
		shared_ptr<GameObject> pGameObject = gameObjManager.GetExistGameObject(name);
		
		// �׷����� ������Ʈ�� ���� ���
		if (instanceData.activeInstanceCount > 0) {
			if(pGameObject) pGameObject->RenderInstance(_pCommandList, instanceData);
		}
	}
}

void GameObject::InitInstanceData() {
	// ������Ʈ ���� �� ���� �׷����� ������Ʈ�� ī��Ʈ�� �ʱ�ȭ ���ش�.
	for (auto& [name, data] : instanceDatas) {
		data.activeInstanceCount = 0;
	}
	for (auto& [name, count] : drawInstanceCount) {
		count = 0;
	}
}

GameObject::GameObject() {
	name = "unknown";
	worldTransform = Matrix4x4::Identity();
	localTransform = Matrix4x4::Identity();
	localPosition = XMFLOAT3(0, 0, 0);
	localRotation = XMFLOAT4(0, 0, 0, 1);
	localScale = XMFLOAT3(1, 1, 1);
	boundingBox = BoundingOrientedBox();
	isOOBBCover = false;
	instanceID = 0;
}
GameObject::~GameObject() {

}


void GameObject::Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();
	GameObject::Create();
	shared_ptr<GameObject> temp = gameFramework.GetGameObjectManager().GetGameObject(_ObjectName, _pDevice, _pCommandList);

	// �ν��Ͻ��� �ڽ����� �� ������Ʈ�� ������ ����
	if (temp) SetChild(temp);
}

void GameObject::Create() {

}

const string& GameObject::GetName() const {
	return name;
}

XMFLOAT3 GameObject::GetLocalRightVector() const {
	XMFLOAT3 rightVector = XMFLOAT3(1, 0, 0);
	rightVector = Vector3::Transform(rightVector, Matrix4x4::RotateQuaternion(localRotation));
	return rightVector;
}
XMFLOAT3 GameObject::GetLocalUpVector() const {
	XMFLOAT3 rightVector = XMFLOAT3(0, 1, 0);
	rightVector = Vector3::Transform(rightVector, Matrix4x4::RotateQuaternion(localRotation));
	return rightVector;
}
XMFLOAT3 GameObject::GetLocalLookVector() const {
	XMFLOAT3 rightVector = XMFLOAT3(0, 0, 1);
	rightVector = Vector3::Transform(rightVector, Matrix4x4::RotateQuaternion(localRotation));
	return rightVector;
}
XMFLOAT3 GameObject::GetLocalPosition() const {
	return localPosition;
}
XMFLOAT4 GameObject::GetLocalRotate() const {
	return localRotation;
}

void GameObject::MoveRight(float distance) {
	XMFLOAT3 moveVector = GetLocalRightVector();	// RightVector�� �����ͼ�
	moveVector = Vector3::Normalize(moveVector);	// �������ͷ� �ٲ���
	moveVector = Vector3::ScalarProduct(moveVector, distance);	// �̵��Ÿ���ŭ �����ش�.
	localPosition = Vector3::Add(localPosition, moveVector);
}

void GameObject::Move(const XMFLOAT3& _moveVector, float _timeElapsed) {
	localPosition = Vector3::Add(localPosition, _moveVector);
}

void GameObject::MoveUp(float distance) {
	XMFLOAT3 moveVector = GetLocalUpVector();	// UpVector�� �����ͼ�
	moveVector = Vector3::Normalize(moveVector);	// �������ͷ� �ٲ���
	moveVector = Vector3::ScalarProduct(moveVector, distance);	// �̵��Ÿ���ŭ �����ش�.
	localPosition = Vector3::Add(localPosition, moveVector);
}
void GameObject::MoveFront(float distance) {
	XMFLOAT3 moveVector = GetLocalLookVector();	// LookVector�� �����ͼ�
	moveVector = Vector3::Normalize(moveVector);	// �������ͷ� �ٲ���
	moveVector = Vector3::ScalarProduct(moveVector, distance);	// �̵��Ÿ���ŭ �����ش�.
	localPosition = Vector3::Add(localPosition, moveVector);
}
void GameObject::Rotate(const XMFLOAT3& _axis, float _angle, float _timeElapsed) {
	localRotation = Vector4::QuaternionMultiply(localRotation, Vector4::QuaternionRotation(_axis, _angle * _timeElapsed));
}

void GameObject::Rotate(const XMFLOAT4& _quat) {

	localRotation = Vector4::QuaternionMultiply(localRotation, _quat);
}

XMFLOAT3 GameObject::GetWorldRightVector() const {
	return Vector3::Normalize(worldTransform._11, worldTransform._12, worldTransform._13);
}
XMFLOAT3 GameObject::GetWorldUpVector() const {
	return Vector3::Normalize(worldTransform._21, worldTransform._22, worldTransform._23);
}
XMFLOAT3 GameObject::GetWorldLookVector() const {
	return Vector3::Normalize(worldTransform._31, worldTransform._32, worldTransform._33);
}
XMFLOAT3 GameObject::GetWorldPosition() const {
	return XMFLOAT3(worldTransform._41, worldTransform._42, worldTransform._43);
}

const BoundingOrientedBox& GameObject::GetBoundingBox() const {
	return boundingBox;
}

shared_ptr<GameObject> GameObject::GetObj() {
	if (!pParent.lock()) return pChildren[0];
	else {
		cout << "�޼��尡 ȣ��� ������Ʈ�� �ν��Ͻ��� �ƴմϴ�.\n";
		return shared_from_this();
	}
}
void GameObject::SetLocalPosition(const XMFLOAT3& _position) {
	localPosition = _position;
}

void GameObject::SetLocalRotation(const XMFLOAT4& _rotation) {
	localRotation = _rotation;
}

void GameObject::SetLocalScale(const XMFLOAT3& _scale) {
	localScale = _scale;
}

void GameObject::SetOOBBCover(bool _isCover) {
	isOOBBCover = _isCover;
}

void GameObject::AddRef() {
	if(pMesh) pMesh->AddRef();
}

UINT GameObject::GetRef() {
	if (pMesh) return pMesh->GetRef();
	else return 0;
}

void GameObject::SetChild(const shared_ptr<GameObject> _pChild) {
	// �Ծ��� ���̰�, �θ� ���� ��� �θ�� ���� ������Ų��.
	if (auto pPreParent = _pChild->pParent.lock()) {
		pPreParent->pChildren.erase(ranges::find(pPreParent->pChildren, _pChild));
	}

	// ���� �ڽ����� �Ծ�
	pChildren.push_back(_pChild);

	// �ڽ��� �θ� ���� ����
	_pChild->pParent = shared_from_this();
}



void GameObject::SetMesh(const shared_ptr<Mesh>& _pMesh) {
	pMesh = _pMesh;
}

void GameObject::SetBoundingBox(const BoundingOrientedBox& _box) {
	baseOrientedBox = _box;
}

void GameObject::UpdateLocalTransform() {
	localTransform = Matrix4x4::Identity();
	// S

	localTransform._11 = localScale.x;
	localTransform._22 = localScale.y;
	localTransform._33 = localScale.z;
	// SxR
	localTransform = Matrix4x4::Multiply(localTransform, Matrix4x4::RotateQuaternion(localRotation));
	// xT
	localTransform._41 = localPosition.x;
	localTransform._42 = localPosition.y;
	localTransform._43 = localPosition.z;

}
void GameObject::UpdateWorldTransform() {
	//UpdateLocalTransform();

	if (auto pParentLock = pParent.lock()) {	// �θ� ���� ���
		worldTransform = Matrix4x4::Multiply(localTransform, pParentLock->worldTransform);
	}
	else {	// �θ� ���� ���
		worldTransform = localTransform;
	}

	// �ڽĵ鵵 worldTransform�� ������Ʈ ��Ų��.
	for (auto& pChild : pChildren) {
		pChild->UpdateWorldTransform();
	}
}

void GameObject::UpdateOOBB() {

	if (isOOBBCover) {
		baseOrientedBox.Transform(boundingBox, XMLoadFloat4x4(&worldTransform));

		XMStoreFloat4(&boundingBox.Orientation, XMQuaternionNormalize(XMLoadFloat4(&boundingBox.Orientation)));
	}

	for (const auto& pChild : pChildren) {
		pChild->UpdateOOBB();
	}
}


void GameObject::UpdateObject() {
	UpdateLocalTransform();
	UpdateWorldTransform();
	UpdateOOBB();
}


shared_ptr<GameObject> GameObject::FindFrame(const string& _name) {
	if (name == _name) {
		return shared_from_this();
	}
	for (const auto& pChild : pChildren) {
		if (auto pFound = pChild->FindFrame(_name)) {
			return pFound;
		}
	}
	return nullptr;
}

void GameObject::PrepareAnimate() {

}

void GameObject::Animate(double _timeElapsed) {
	for (const auto& pChild : pChildren) {
		pChild->Animate(_timeElapsed);
	}
}

void GameObject::Animate(double _timeElapsed, const XMFLOAT3& _playerPos) {

};
void GameObject::Remove() {
};
bool GameObject::CheckRemove() const {
	return false;
}

void GameObject::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	if (pMesh) {	// �޽��� ���� ��쿡�� �������� �Ѵ�.
		UpdateShaderVariable(_pCommandList);

		// �� ���׸��� �´� ����޽��� �׸���.
		for (int i = 0; i < materials.size(); ++i) {
			// �ش� ����Ž��� ��Ī�Ǵ� ���׸����� Set ���ش�.

			materials[i]->UpdateShaderVariable(_pCommandList);
			pMesh->Render(_pCommandList, i);
		}
	}
	for (const auto& pChild : pChildren) {
		pChild->Render(_pCommandList);
	}
}

void GameObject::RenderInstance(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, Instancing_Data& _instanceData) {
	if (pMesh) {
		// ��������� IA�ܰ�� ���� ������.
		for (int i = 0; i < materials.size(); ++i) {
			// �ش� ����Ž��� ��Ī�Ǵ� ���׸����� Set ���ش�.

			materials[i]->UpdateShaderVariable(_pCommandList);
			pMesh->RenderInstance(_pCommandList, i, _instanceData);
		}
	}
}

void GameObject::InputInstanceData() {
	// ���� ���ҽ��� �� ���� �ν��Ͻ��� ��
	string objName = pChildren[0]->GetName();

	UINT count = drawInstanceCount[objName]++;

	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, XMMatrixTranspose(XMLoadFloat4x4(&worldTransform)));

	memcpy((instanceDatas[objName].mappedResource) + count, &world, sizeof(XMFLOAT4X4));

	instanceDatas[objName].activeInstanceCount = count + 1;
}

void GameObject::RenderHitBox(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, HitBoxMesh& _hitBox) {

	if (isOOBBCover) {	// �޽��� ���� ��쿡�� �������� �Ѵ�.
		UpdateHitboxShaderVariable(_pCommandList);
		// ����� ���̴��� �׷��Ƚ� ������������ �����Ѵ� [�������]
		_hitBox.Render(_pCommandList);
	}
	for (const auto& pChild : pChildren) {
		pChild->RenderHitBox(_pCommandList, _hitBox);
	}
}

void GameObject::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, XMMatrixTranspose(XMLoadFloat4x4(&worldTransform)));
	_pCommandList->SetGraphicsRoot32BitConstants(1, 16, &world, 0);
}


void GameObject::UpdateHitboxShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	XMFLOAT4X4 world = Matrix4x4::ScaleTransform(Vector3::ScalarProduct(baseOrientedBox.Extents, 1.0f));
	XMFLOAT4X4 translate = Matrix4x4::Identity();
	translate._41 += baseOrientedBox.Center.x;
	translate._42 += baseOrientedBox.Center.y;
	translate._43 += baseOrientedBox.Center.z;
	// �ٿ���ڽ��� extents, center�� ��ŭ �̵� �� ��ȯ ��� ���� ( ���� )
	world = Matrix4x4::Multiply(Matrix4x4::Multiply(world, translate), worldTransform);

	XMStoreFloat4x4(&world, XMMatrixTranspose(XMLoadFloat4x4(&world)));
	//world = Matrix4x4::Identity();
	_pCommandList->SetGraphicsRoot32BitConstants(1, 16, &world, 0);

}



void GameObject::LoadFromFile(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const shared_ptr<GameObject>& _coverObject) {
	GameFramework& gameFramework = GameFramework::Instance();

	// nameSize (UINT) / name(string)
	ReadStringBinary(name, _file);

	// localTransform(float3, 3, 4)
	_file.read((char*)&localPosition, sizeof(XMFLOAT3));
	_file.read((char*)&localScale, sizeof(XMFLOAT3));
	_file.read((char*)&localRotation, sizeof(XMFLOAT4));
	UpdateLocalTransform();

	int haveMesh;
	// haveMesh(bool)

	_file.read((char*)&haveMesh, sizeof(int));
	// �޽ð� ������� ��ŵ
	if (haveMesh) {

		pMesh = make_shared<Mesh>();
		pMesh->LoadFromFile(_file, _pDevice, _pCommandList, shared_from_this());

		// ���׸��� ����
		// nMaterial (UINT)
		UINT nMaterial = 0;
		_file.read((char*)&nMaterial, sizeof(UINT));
		materials.resize(nMaterial);

		// ���׸������ �ҷ���
		for (auto& mat : materials) {
			mat = make_shared<Material>();
			mat->LoadMaterial(_file, _pDevice, _pCommandList);
		}
	}
	UINT nChildren;
	_file.read((char*)&nChildren, sizeof(UINT));
	pChildren.reserve(nChildren);
		
	for (int i = 0; i < nChildren; ++i) {
		shared_ptr<GameObject> newObject = make_shared<GameObject>();
		newObject->LoadFromFile(_file, _pDevice, _pCommandList, _coverObject);
		SetChild(newObject);
	}
}

void GameObject::CopyObject(const GameObject& _other) {
	name = _other.name;
	worldTransform = _other.worldTransform;
	localTransform = _other.localTransform;
	boundingBox = _other.boundingBox;
	baseOrientedBox = _other.baseOrientedBox;
	isOOBBCover = _other.isOOBBCover;
	localPosition = _other.localPosition;
	localScale = _other.localScale;
	localRotation = _other.localRotation;
	pMesh = _other.pMesh;
	materials = _other.materials;

	for (int i = 0; i < _other.pChildren.size(); ++i) {
		shared_ptr<GameObject> child = make_shared<GameObject>();
		child->CopyObject(*_other.pChildren[i]);
		SetChild(child);
	}
}

/////////////// FullScreenObject


/////////////////////////// GameObjectManager /////////////////////
shared_ptr<GameObject> GameObjectManager::GetGameObject(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	if (!storage.contains(_name)) {	// ó�� �ҷ��� ������Ʈ�� ���
		shared_ptr<GameObject> newObject;
		
		newObject = make_shared<GameObject>();
		ifstream file("GameObject/" + _name, ios::binary);	// ������ ����

		if (!file) {
			cout << "GameObject Load Failed : " << _name << "\n";
			return nullptr;
		}

		// �ֻ��� ������Ʈ�� Ŀ�� OOBB�� ����
		newObject->SetOOBBCover(true);
		newObject->LoadFromFile(file, _pDevice, _pCommandList, newObject);

		// �ֻ��� ������Ʈ�� ���� ���Ͽ��� �о��
		BoundingOrientedBox box;
		file.read((char*)&box.Center, sizeof(XMFLOAT3));
		file.read((char*)&box.Extents, sizeof(XMFLOAT3));

		newObject->SetBoundingBox(box);
		// eachTransfrom�� �°� �� ������ ������Ʈ���� worldTransform�� ����
		storage[_name] = newObject;
		newObject->UpdateObject();
	}

	// ���丮�� �� ������Ʈ ������ ���� ������Ʈ�� �����Ͽ� �����Ѵ�.
	shared_ptr<GameObject> Object;
	Object = make_shared<GameObject>();

	Object->CopyObject(*storage[_name]);
	Object->AddRef();

	return Object;
}

shared_ptr<GameObject> GameObjectManager::GetExistGameObject(const string& _name) {
	if (!storage.contains(_name)) {	// ó�� �ҷ��� ������Ʈ�� ���
		cout << _name << " ������Ʈ�� �����ϴ�!!\n";
		return nullptr;
	}
	return storage[_name];
}

void GameObjectManager::InitInstanceResource(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	UINT refCount = 0;
	ComPtr<ID3D12Resource> temp;
	auto& instanceDatas = GameObject::GetInstanceDatas();
	for (auto& [name, pObject] : storage) {
		Instancing_Data data;
		refCount = pObject->GetRef();
		
		data.resource = CreateBufferResource(_pDevice, _pCommandList, NULL, sizeof(XMFLOAT4X4) * refCount, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, temp);
		data.bufferView.BufferLocation = data.resource->GetGPUVirtualAddress();
		data.bufferView.StrideInBytes = sizeof(XMFLOAT4X4);
		data.bufferView.SizeInBytes = sizeof(XMFLOAT4X4) * refCount;

		instanceDatas[name] = data;
		instanceDatas[name].resource->Map(0, NULL, (void**)&instanceDatas[name].mappedResource);
		cout << name << " �ν��Ͻ��� �ִ� " << refCount << "���� �ֽ��ϴ�\n";
	}
	cout << instanceDatas.size() << "���Դϴ�.\n";
}
