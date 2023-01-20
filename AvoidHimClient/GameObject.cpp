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
		// 해당 인스턴스의 오브젝트 정보를 가져온다.
		shared_ptr<GameObject> pGameObject = gameObjManager.GetExistGameObject(name);
		
		// 그려지는 오브젝트가 있을 경우
		if (instanceData.activeInstanceCount > 0) {
			if(pGameObject) pGameObject->RenderInstance(_pCommandList, instanceData);
		}
	}
}

void GameObject::InitInstanceData() {
	// 오브젝트 종류 별 현재 그려지는 오브젝트의 카운트를 초기화 해준다.
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

	// 인스턴스의 자식으로 그 오브젝트의 정보를 설정
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
	XMFLOAT3 moveVector = GetLocalRightVector();	// RightVector를 가져와서
	moveVector = Vector3::Normalize(moveVector);	// 단위벡터로 바꾼후
	moveVector = Vector3::ScalarProduct(moveVector, distance);	// 이동거리만큼 곱해준다.
	localPosition = Vector3::Add(localPosition, moveVector);
}

void GameObject::Move(const XMFLOAT3& _moveVector, float _timeElapsed) {
	localPosition = Vector3::Add(localPosition, _moveVector);
}

void GameObject::MoveUp(float distance) {
	XMFLOAT3 moveVector = GetLocalUpVector();	// UpVector를 가져와서
	moveVector = Vector3::Normalize(moveVector);	// 단위벡터로 바꾼후
	moveVector = Vector3::ScalarProduct(moveVector, distance);	// 이동거리만큼 곱해준다.
	localPosition = Vector3::Add(localPosition, moveVector);
}
void GameObject::MoveFront(float distance) {
	XMFLOAT3 moveVector = GetLocalLookVector();	// LookVector를 가져와서
	moveVector = Vector3::Normalize(moveVector);	// 단위벡터로 바꾼후
	moveVector = Vector3::ScalarProduct(moveVector, distance);	// 이동거리만큼 곱해준다.
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
		cout << "메서드가 호출된 오브젝트는 인스턴스가 아닙니다.\n";
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
	// 입양할 아이가, 부모가 있을 경우 부모로 부터 독립시킨다.
	if (auto pPreParent = _pChild->pParent.lock()) {
		pPreParent->pChildren.erase(ranges::find(pPreParent->pChildren, _pChild));
	}

	// 나의 자식으로 입양
	pChildren.push_back(_pChild);

	// 자식의 부모를 나로 지정
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

	if (auto pParentLock = pParent.lock()) {	// 부모가 있을 경우
		worldTransform = Matrix4x4::Multiply(localTransform, pParentLock->worldTransform);
	}
	else {	// 부모가 없을 경우
		worldTransform = localTransform;
	}

	// 자식들도 worldTransform을 업데이트 시킨다.
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
	if (pMesh) {	// 메쉬가 있을 경우에만 렌더링을 한다.
		UpdateShaderVariable(_pCommandList);

		// 각 마테리얼에 맞는 서브메쉬를 그린다.
		for (int i = 0; i < materials.size(); ++i) {
			// 해당 서브매쉬와 매칭되는 마테리얼을 Set 해준다.

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
		// 월드행렬을 IA단계로 직접 보낸다.
		for (int i = 0; i < materials.size(); ++i) {
			// 해당 서브매쉬와 매칭되는 마테리얼을 Set 해준다.

			materials[i]->UpdateShaderVariable(_pCommandList);
			pMesh->RenderInstance(_pCommandList, i, _instanceData);
		}
	}
}

void GameObject::InputInstanceData() {
	// 현재 리소스에 들어간 같은 인스턴스의 수
	string objName = pChildren[0]->GetName();

	UINT count = drawInstanceCount[objName]++;

	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, XMMatrixTranspose(XMLoadFloat4x4(&worldTransform)));

	memcpy((instanceDatas[objName].mappedResource) + count, &world, sizeof(XMFLOAT4X4));

	instanceDatas[objName].activeInstanceCount = count + 1;
}

void GameObject::RenderHitBox(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, HitBoxMesh& _hitBox) {

	if (isOOBBCover) {	// 메쉬가 있을 경우에만 렌더링을 한다.
		UpdateHitboxShaderVariable(_pCommandList);
		// 사용할 쉐이더의 그래픽스 파이프라인을 설정한다 [수정요망]
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
	// 바운딩박스의 extents, center값 만큼 이동 후 변환 행렬 적용 ( 공전 )
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
	// 메시가 없을경우 스킵
	if (haveMesh) {

		pMesh = make_shared<Mesh>();
		pMesh->LoadFromFile(_file, _pDevice, _pCommandList, shared_from_this());

		// 마테리얼 정보
		// nMaterial (UINT)
		UINT nMaterial = 0;
		_file.read((char*)&nMaterial, sizeof(UINT));
		materials.resize(nMaterial);

		// 마테리얼들을 불러옴
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

	if (!storage.contains(_name)) {	// 처음 불러온 오브젝트일 경우
		shared_ptr<GameObject> newObject;
		
		newObject = make_shared<GameObject>();
		ifstream file("GameObject/" + _name, ios::binary);	// 파일을 연다

		if (!file) {
			cout << "GameObject Load Failed : " << _name << "\n";
			return nullptr;
		}

		// 최상위 오브젝트를 커버 OOBB로 설정
		newObject->SetOOBBCover(true);
		newObject->LoadFromFile(file, _pDevice, _pCommandList, newObject);

		// 최상위 오브젝트는 따로 파일에서 읽어옴
		BoundingOrientedBox box;
		file.read((char*)&box.Center, sizeof(XMFLOAT3));
		file.read((char*)&box.Extents, sizeof(XMFLOAT3));

		newObject->SetBoundingBox(box);
		// eachTransfrom에 맞게 각 계층의 오브젝트들의 worldTransform을 갱신
		storage[_name] = newObject;
		newObject->UpdateObject();
	}

	// 스토리지 내 오브젝트 정보와 같은 오브젝트를 복사하여 생성한다.
	shared_ptr<GameObject> Object;
	Object = make_shared<GameObject>();

	Object->CopyObject(*storage[_name]);
	Object->AddRef();

	return Object;
}

shared_ptr<GameObject> GameObjectManager::GetExistGameObject(const string& _name) {
	if (!storage.contains(_name)) {	// 처음 불러온 오브젝트일 경우
		cout << _name << " 오브젝트가 없습니다!!\n";
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
		cout << name << " 인스턴스는 최대 " << refCount << "개가 있습니다\n";
	}
	cout << instanceDatas.size() << "개입니다.\n";
}
