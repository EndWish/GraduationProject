#include "stdafx.h"
#include "GameObject.h"
#include "Light.h"
#include "GameFramework.h"


unordered_map<string, Instancing_Data> GameObject::instanceDatas;

//////////////////////////////////////////


void GameObject::RenderInstanceObjects(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();
	gameFramework.GetShader("InstancingShader")->PrepareRender(_pCommandList);
	GameObjectManager& gameObjManager = gameFramework.GetGameObjectManager();

	for (auto& [name, instanceData] : instanceDatas) {
		// 해당 인스턴스의 오브젝트 정보를 가져온다.
		shared_ptr<GameObject> pGameObject = gameObjManager.GetExistGameObject(name);
		
		if(pGameObject) pGameObject->RenderInstance(_pCommandList, instanceData);
		
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
	id = 0;
	objectClass = 0;
	shaderType = ShaderType::none;
}
GameObject::~GameObject() {

}


void GameObject::Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();
	GameObject::Create();
	shared_ptr<GameObject> temp = gameFramework.GetGameObjectManager().GetGameObject(_ObjectName, _pDevice, _pCommandList);

	// 인스턴스의 자식으로 그 오브젝트의 정보를 설정

	if (temp) {
		// 인스턴스가 CoverOOBB를 갖게 된다.
		name = temp->GetName() + "_Instance";
		isOOBBCover = true;
		baseOrientedBox = temp->GetBaseBoundingBox();
		SetChild(temp);
	}
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
XMFLOAT3 GameObject::GetLocalScale() const {
	return localScale;
}
XMFLOAT4 GameObject::GetLocalRotate() const {
	return localRotation;
}



void GameObject::Move(const XMFLOAT3& _moveVector, float _timeElapsed) {
	localPosition = Vector3::Add(localPosition, Vector3::ScalarProduct(_moveVector, _timeElapsed));
}

void GameObject::MoveRight(float distance, float _timeElapsed) {
	XMFLOAT3 moveVector = GetLocalRightVector();	// RightVector를 가져와서
	moveVector = Vector3::Normalize(moveVector);	// 단위벡터로 바꾼후
	moveVector = Vector3::ScalarProduct(moveVector, distance * _timeElapsed);	// 이동거리만큼 곱해준다.
	localPosition = Vector3::Add(localPosition, moveVector);
}

void GameObject::MoveUp(float distance, float _timeElapsed) {
	XMFLOAT3 moveVector = GetLocalUpVector();	// UpVector를 가져와서
	moveVector = Vector3::Normalize(moveVector);	// 단위벡터로 바꾼후
	moveVector = Vector3::ScalarProduct(moveVector, distance * _timeElapsed);	// 이동거리만큼 곱해준다.
	localPosition = Vector3::Add(localPosition, moveVector);
}

void GameObject::MoveFront(float distance, float _timeElapsed) {
	XMFLOAT3 moveVector = GetLocalLookVector();	// LookVector를 가져와서
	moveVector = Vector3::Normalize(moveVector);	// 단위벡터로 바꾼후
	moveVector = Vector3::ScalarProduct(moveVector, distance * _timeElapsed);	// 이동거리만큼 곱해준다.
	localPosition = Vector3::Add(localPosition, moveVector);
}

void GameObject::Rotate(const XMFLOAT3& _axis, float _angle, float _timeElapsed) {
	localRotation = Vector4::QuaternionMultiply(localRotation, Vector4::QuaternionRotation(_axis, _angle * _timeElapsed));
}

void GameObject::Rotate(const XMFLOAT4& _quat) {

	localRotation = Vector4::QuaternionMultiply(localRotation, _quat);
}

void GameObject::Revolve(const XMFLOAT3& _axis, float _angle) {
	XMFLOAT4X4 rotationMatrix = Matrix4x4::RotationAxis(_axis, _angle);
	localPosition = Vector3::Transform(localPosition, rotationMatrix);
}
void GameObject::SynchronousRotation(const XMFLOAT3& _axis, float _angle) {
	Revolve(_axis, _angle);
	localRotation = Vector4::QuaternionMultiply(localRotation, Vector4::QuaternionRotation(_axis, _angle));
}
void GameObject::RotateMoveHorizontal(XMFLOAT3 _dir, float _angularSpeed, float _moveSpeed) {
	// 룩벡터와 타겟벡터를 xz평면에 투영한다.
	_dir.y = 0;
	XMFLOAT3 origin = GetLocalLookVector();
	origin.y = 0;
	XMFLOAT3 axis = Vector3::CrossProduct(origin, _dir);
	float minAngle = Vector3::Angle(origin, _dir);
	if (abs(axis.y) <= FLT_EPSILON) {	// 외적이 불가능한 경우 (두 벡터가 평행한 경우)
		axis = XMFLOAT3(0, 1, 0);
	}

	localRotation = Vector4::QuaternionMultiply(localRotation, Vector4::QuaternionRotation(axis, min(_angularSpeed, minAngle)));
	MoveFront(_moveSpeed);
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
XMFLOAT4X4 GameObject::GetWorldTransform() const {
	return worldTransform;
}
XMFLOAT3 GameObject::GetWorldPosition() const {
	return XMFLOAT3(worldTransform._41, worldTransform._42, worldTransform._43);
}

const BoundingOrientedBox& GameObject::GetBoundingBox() const {
	return boundingBox;
}

const BoundingOrientedBox& GameObject::GetBaseBoundingBox() const {
	return baseOrientedBox;
}

shared_ptr<GameObject> GameObject::GetObj() {
	if (pChildren.size() > 0) return pChildren[0];
	else {
		cout << "메서드가 호출된 오브젝트는 인스턴스가 아닙니다.\n";
		return nullptr;
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

void GameObject::SetObjectClass(UINT _objectClass) {
	objectClass = _objectClass;
}
UINT GameObject::GetObjectClass() {
	return objectClass;
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


ShaderType GameObject::GetShaderType() const {
	return shaderType;
}

void GameObject::SetShaderType(ShaderType _shaderType) {
	shaderType = _shaderType;
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

void GameObject::Animate(float _timeElapsed) {
	for (const auto& pChild : pChildren) {
		pChild->Animate(_timeElapsed);
	}
}

void GameObject::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	if (pMesh) {	// 메쉬가 있을 경우에만 렌더링을 한다.
		GameFramework& gameFramework = GameFramework::Instance();
		UpdateShaderVariable(_pCommandList);

		// 각 마테리얼에 맞는 서브메쉬를 그린다.
		for (int i = 0; i < materials.size(); ++i) {
			// 해당 서브매쉬와 매칭되는 마테리얼을 Set 해준다.

			materials[i]->UpdateShaderVariable(_pCommandList);
			pMesh->Render(_pCommandList, i);
		}

	}
}

void GameObject::RenderAll(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	if (pMesh) {	// 메쉬가 있을 경우에만 렌더링을 한다.
		GameFramework& gameFramework = GameFramework::Instance();

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
	
	XMFLOAT4X4 world = Matrix4x4::ScaleTransform(Vector3::ScalarProduct(baseOrientedBox.Extents, 2.0f));
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
	// ShaderType(char)
	_file.read((char*)&shaderType, sizeof(ShaderType));

	// localTransform(float3, 3, 4)
	_file.read((char*)&localPosition, sizeof(XMFLOAT3));
	_file.read((char*)&localScale, sizeof(XMFLOAT3));
	_file.read((char*)&localRotation, sizeof(XMFLOAT4));
	UpdateLocalTransform();

	
	int haveMesh;
	// haveMesh(int)


	_file.read((char*)&haveMesh, sizeof(int));
	// 메시가 없을경우 스킵
	if (0 < haveMesh) {
		if(haveMesh == 1)
			pMesh = make_shared<Mesh>();
		else
		{
			pMesh = make_shared<SkinnedMesh>();
		}
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


	UINT nChildren = 0;
	_file.read((char*)&nChildren, sizeof(UINT));
	pChildren.reserve(nChildren);
		
	for (int i = 0; i < (int)nChildren; ++i) {
		UINT objectType;
		_file.read((char*)&objectType, sizeof(UINT));

		shared_ptr<GameObject> newObject;
		if (objectType == 1) {
			newObject = make_shared<SkinnedGameObject>();
		}
		else if (objectType == 2) {
			newObject = make_shared<Effect>();
		}
		else {
			newObject = make_shared<GameObject>();
		}

		newObject->LoadFromFile(_file, _pDevice, _pCommandList, _coverObject);
		SetChild(newObject);
	}
}

void GameObject::CopyObject(const GameObject& _other) {
	GameFramework& gameFramework = GameFramework::Instance();

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
	shaderType = _other.shaderType;

	switch (shaderType) {
	case ShaderType::basic: {
		// 인스턴싱을 사용하지 않고, 불투명한 오브젝트를 그리는 쉐이더
		gameFramework.GetShader("BasicShader")->AddObject(shared_from_this());
		break;
	}
	case ShaderType::blending: {
		// 인스턴싱을 사용하지 않고, 반투명한 오브젝트를 그리는 쉐이더
		gameFramework.GetShader("BlendingShader")->AddObject(shared_from_this());
		break;
	}
	case ShaderType::skinned: {
		gameFramework.GetShader("SkinnedShader")->AddObject(shared_from_this());
		break;
	}
	case ShaderType::effect: {
		gameFramework.GetShader("EffectShader")->AddObject(shared_from_this());
		break;
	}
	}


	for (int i = 0; i < _other.pChildren.size(); ++i) {
		shared_ptr<GameObject> child;
		if(_other.pChildren[i]->GetObjectClass() == 1)
			child = make_shared<SkinnedGameObject>();
		else if(_other.pChildren[i]->GetObjectClass() == 2)
			child = make_shared<Effect>();
		else
			child = make_shared<GameObject>();

		child->CopyObject(*_other.pChildren[i]);
		SetChild(child);
	}
}

///////////////////////////////////////////////////////////////////////////////
/// EffectGameObject
Effect::Effect() {
	objectClass = 2;
	nIndex = 1; row = 1; col = 1;
	curIndexTime = 0.f;
	maxIndexTime = 0.f;
	lifeTime = 1.f;
}
Effect::~Effect()  {

}

void Effect::Animate(float _timeElapsed) {
	lifeTime -= _timeElapsed;

	curIndexTime += _timeElapsed;
	while (maxIndexTime <= curIndexTime) {
		curIndexTime -= maxIndexTime;
	}
		
	GameObject::Animate(_timeElapsed);
}

void Effect::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	XMINT3 indexInfo;
	indexInfo.x = (UINT)(nIndex * curIndexTime / maxIndexTime);
	indexInfo.y = row;
	indexInfo.z = col;
	_pCommandList->SetGraphicsRoot32BitConstants(8, 3, &indexInfo, 0);
	GameObject::UpdateShaderVariable(_pCommandList);
}

void Effect::LoadFromFile(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const shared_ptr<GameObject>& _coverObject) {
	_file.read((char*)&nIndex, sizeof(UINT));
	_file.read((char*)&row, sizeof(UINT));
	_file.read((char*)&col, sizeof(UINT));
	_file.read((char*)&maxIndexTime, sizeof(float));
	_file.read((char*)&lifeTime, sizeof(float));

	GameObject::LoadFromFile(_file, _pDevice, _pCommandList, _coverObject);
}

void Effect::CopyObject(const GameObject& _other) {
	const Effect& other = dynamic_cast<const Effect&>(_other);

	nIndex = other.nIndex;
	row = other.row;
	col = other.col;
	curIndexTime = other.curIndexTime;
	maxIndexTime = other.maxIndexTime;
	lifeTime = other.lifeTime;
	GameObject::CopyObject(_other);
}

/////////////////////////// SkinnedGameObject /////////////////////
ComPtr<ID3D12Resource> SkinnedGameObject::pSkinnedWorldTransformBuffer;
shared_ptr<SkinnedWorldTransformFormat> SkinnedGameObject::pMappedSkinnedWorldTransform;

void SkinnedGameObject::InitSkinnedWorldTransformBuffer(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	ComPtr<ID3D12Resource> temp;
	UINT ncbElementBytes = ((sizeof(SkinnedWorldTransformFormat) + 255) & ~255); //256의 배수
	pSkinnedWorldTransformBuffer = ::CreateBufferResource(_pDevice, _pCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, temp);
	pSkinnedWorldTransformBuffer->Map(0, NULL, (void**)&pMappedSkinnedWorldTransform);
}

SkinnedGameObject::SkinnedGameObject() {
	objectClass = true;
	aniController = AnimationController();
}
SkinnedGameObject::~SkinnedGameObject() {

}

void SkinnedGameObject::LoadFromFile(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const shared_ptr<GameObject>& _coverObject) {
	UINT nBone = 0;

	_file.read((char*)&nBone, sizeof(UINT));

	vector<string> boneNames(nBone);
	for (UINT i = 0; i < nBone; ++i) {
		ReadStringBinary(boneNames[i], _file);
	}

	aniController.LoadFromFile(_file, nBone);

	GameObject::LoadFromFile(_file, _pDevice, _pCommandList, _coverObject);
	// ShaderType은 강제로 SkinnedShader로 바꾸어준다.

	// 이름을 가지고 뼈를 찾는다.
	pBones.assign(nBone, nullptr);
	for (UINT i = 0; i < nBone; ++i) {
		pBones[i] = FindFrame(boneNames[i]);
	}

}

void SkinnedGameObject::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	if (pMesh) {
		GameFramework& gameFramework = GameFramework::Instance();
		// 메쉬가 있을 경우에만 렌더링을 한다.
		// 애니메이션에 따라 본들의 행렬을 변경하고 Update한다.	[이 부분은 추후 Animation에서 하도록 수정한다.]
		aniController.AddTime(1.f / 300.f);
		aniController.UpdateBoneLocalTransform(pBones);
		UpdateWorldTransform();
		UpdateShaderVariable(_pCommandList);

		// 본의 월드 변환 행렬들을 리소스에 담아 루트시그니처에 연결한다.
		for (int i = 0; auto & pBone : pBones) {
			XMFLOAT4X4 world = pBone->GetWorldTransform();
			XMStoreFloat4x4(&world, XMMatrixTranspose(XMLoadFloat4x4(&world)));
			pMappedSkinnedWorldTransform->worldTransform[i++] = world;
		}

		D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = pSkinnedWorldTransformBuffer->GetGPUVirtualAddress();
		_pCommandList->SetGraphicsRootConstantBufferView(7, gpuVirtualAddress);

		// 각 마테리얼에 맞는 서브메쉬를 그린다.
		for (int i = 0; i < materials.size(); ++i) {
			// 해당 서브매쉬와 매칭되는 마테리얼을 Set 해준다.
			materials[i]->UpdateShaderVariable(_pCommandList);
			pMesh->Render(_pCommandList, i);	// 오프셋 행렬을 루트 시그니처에 연결하는 것은 메쉬에서 한다.
		}
	}
	for (const auto& pChild : pChildren) {
		pChild->Render(_pCommandList);
	}
}

void SkinnedGameObject::CopyObject(const GameObject& _other) {
	const SkinnedGameObject& other = dynamic_cast<const SkinnedGameObject&>(_other);

	aniController = other.aniController;
	GameObject::CopyObject(_other);

	pBones.assign(other.pBones.size(), {});
	for (int i = 0; i < pBones.size(); ++i) {
		if(other.pBones[i])
			pBones[i] = FindFrame(other.pBones[i]->GetName());
	}
}

/////////////////////////// GameObjectManager /////////////////////
bool GameObjectManager::LoadGameObject(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	ifstream file("GameObject/" + _name, ios::binary);	// 파일을 연다
	if (!file) {
		cout << "GameObject Load Failed : " << _name << "\n";
		return false;
	}

	//ObjectType(uint)	: 0은 GameObject, 1은 스킨드 오브젝트(nBone과 boneName, 애니메이션 정보가 추가됨), 2는 이펙트
	UINT objectType;
	file.read((char*)&objectType, sizeof(UINT));

	shared_ptr<GameObject> newObject;

	if (objectType == 1) {	// 스킨드 오브젝트
		newObject = make_shared<SkinnedGameObject>();

		//newObject->SetSkinnedObject(true);
	}
	else if (objectType == 2) {	// 이펙트일 경우
		newObject = make_shared<Effect>();

	}
	else {
		newObject = make_shared<GameObject>();
	}

	// 최상위 오브젝트를 커버 OOBB로 설정
	newObject->LoadFromFile(file, _pDevice, _pCommandList, newObject);

	// 최상위 오브젝트는 따로 파일에서 읽어옴
	BoundingOrientedBox box;
	file.read((char*)&box.Center, sizeof(XMFLOAT3));
	file.read((char*)&box.Extents, sizeof(XMFLOAT3));

	newObject->SetBoundingBox(box);
	// eachTransfrom에 맞게 각 계층의 오브젝트들의 worldTransform을 갱신
	storage[_name] = newObject;
	newObject->UpdateObject();

	return true;
}

shared_ptr<GameObject> GameObjectManager::GetGameObject(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	if (!storage.contains(_name)) {	// 처음 불러온 오브젝트일 경우
		int result = LoadGameObject(_name, _pDevice, _pCommandList);
		if (!result) {
			cout << "GameObject Load Failed : " << _name << "\n";
			return nullptr;
		}
	}

	// 스토리지 내 오브젝트 정보와 같은 오브젝트를 복사하여 생성한다.
	shared_ptr<GameObject> Object;
	if (storage[_name]->GetObjectClass() == 1) {
		Object = make_shared<SkinnedGameObject>();
	}
	else if (storage[_name]->GetObjectClass() == 2) {
		Object = make_shared<Effect>();
	}
	else {
		Object = make_shared<GameObject>();
	}

	Object->CopyObject(*storage[_name]);
	return Object;
}

shared_ptr<GameObject> GameObjectManager::GetExistGameObject(const string& _name) {
	if (!storage.contains(_name)) {	// 처음 불러온 오브젝트일 경우
		cout << _name << " 오브젝트가 없습니다!!\n";
		return nullptr;
	}
	return storage[_name];
}

void GameObjectManager::InitInstanceResource(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, unordered_map<string, vector<XMFLOAT4X4>>& _instanceDatas) {
	UINT refCount = 0;
	ComPtr<ID3D12Resource> temp;
	auto& instanceDatas = GameObject::GetInstanceDatas();

	for (auto& [name, transform] : _instanceDatas) {
		Instancing_Data data;
		// 현재 이 이름을 가진 오브젝트를 사용하는 인스턴스의 수만큼 리소스를 생성한다.
		refCount = (UINT)_instanceDatas[name].size();
		data.activeInstanceCount = refCount;

		data.resource = CreateBufferResource(_pDevice, _pCommandList, _instanceDatas[name].data(), sizeof(XMFLOAT4X4) * refCount, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, instanceUploadBuffers[name]);
		data.bufferView.BufferLocation = data.resource->GetGPUVirtualAddress();
		data.bufferView.StrideInBytes = sizeof(XMFLOAT4X4);
		data.bufferView.SizeInBytes = sizeof(XMFLOAT4X4) * refCount;

		instanceDatas[name] = data;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// InterpolateMoveGameObject

InterpolateMoveGameObject::InterpolateMoveGameObject() {
	prevPosition = XMFLOAT3();
	prevRotation = Vector4::QuaternionIdentity();
	prevScale = XMFLOAT3(1,1,1);
	
	nextPosition = XMFLOAT3();
	nextRotation = Vector4::QuaternionIdentity();
	nextScale = XMFLOAT3(1,1,1);
	t = 0;
	hp = 100.0f;
	moveDistance = 0.f;

	imprisoned = false;
}

void InterpolateMoveGameObject::Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();
	GameObject::Create(_ObjectName, _pDevice, _pCommandList);
	name = "OtherPlayer";
	pFootStepSound = gameFramework.GetSoundManager().LoadFile("step");
}

InterpolateMoveGameObject::~InterpolateMoveGameObject() {
}

void InterpolateMoveGameObject::Animate(float _timeElapsed) {
	// 서버의 다음 주기가 돌때를 t = 1로 잡고 보간한다
	XMFLOAT3 prevPosition = GetWorldPosition();
	t += _timeElapsed / SERVER_PERIOD; 
	t = min(t, 1.f);
	localPosition = Vector3::Lerp(prevPosition, nextPosition, t);
	localRotation = Vector4::QuaternionSlerp(prevRotation, nextRotation, t);
	localScale = Vector3::Lerp(prevScale, nextScale, t);

	UpdateObject();

	XMFLOAT3 position = GetWorldPosition();
	pFootStepSound->SetPosition(position);
	if (position.y == prevPosition.y) {
		moveDistance += Vector3::Length(Vector3::Subtract(prevPosition, position));
	}
	if (moveDistance > (1.0f * 2)) {
		pFootStepSound->Play();
		moveDistance = 0.f;
	}

}

void InterpolateMoveGameObject::SetNextTransform(const XMFLOAT3& _position, const XMFLOAT4& _rotation, const XMFLOAT3& _scale) {
	// 기존에 보간으로 이동중이던 목표위치로 순간이동 시킨다.
	localPosition = nextPosition;
	localRotation = nextRotation;
	localScale = nextScale;

	// 시작 위치를 갱신해준다.
	prevPosition = nextPosition;
	prevRotation = nextRotation;
	prevScale = nextScale;

	// 도착 위치를 새로 도착한 패킷의 정보로 갱신해준다.
	nextPosition = _position;
	nextRotation = _rotation;
	nextScale = _scale;

	UpdateObject();
	
	t = 0;
}

///////////////////////////////////////////////////////////////////////////////
/// InteractObject

InteractObject::InteractObject() {
}

InteractObject::~InteractObject() {
}

void InteractObject::Interact()
{
}

void InteractObject::EndInteract() {

}

bool InteractObject::IsEnable() {
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///

Door::Door(ObjectType _type) {
	isLeft = (_type == ObjectType::Ldoor || _type == ObjectType::exitLDoor) ? true : false;
	openAngle = 0.f;
	isOpen = false;
	isPrison = (_type == ObjectType::prisonDoor) ? true : false;
}

Door::~Door() {
}

void Door::QueryInteract() {
	CS_TOGGLE_DOOR packet;
	packet.cid = cid;
	packet.objectID = id;
	SendFixedPacket(packet);

}

void Door::Interact() {
	isOpen = !isOpen;
	cout << "열려라 참깨\n";
}

void Door::Animate(float _timeElapsed) {
	float angle = 0.f;
	float dir = isLeft ? 1.f : -1.f;
	
	if (isOpen && openAngle < 90.f) {
		// 열리는곳
		angle = min(180.0f, (90.0f - openAngle) / _timeElapsed);
		Rotate(XMFLOAT3(0, 1, 0), -angle * dir, _timeElapsed);
	}

	if (!isOpen && openAngle > 0.f) {
		angle = -min(180.0f, openAngle / _timeElapsed);
		Rotate(XMFLOAT3(0, 1, 0), -angle * dir, _timeElapsed);
	}
	openAngle += (angle * _timeElapsed);
	UpdateObject();
}

bool Door::IsInteractable(bool _isPlayerProfessor) {
	// 학생은 모든 문을 열 수 있지만, 교수는 감옥 문을 열지 못한다.
	return !(isPrison && _isPlayerProfessor);
}


///////////////////////////////////////////////////////////////////////////////
///

WaterDispenser::WaterDispenser() {
	coolTime = 0.f;
}

WaterDispenser::~WaterDispenser() {
}

void WaterDispenser::QueryInteract() {
	if (coolTime <= 0) {
		CS_USE_WATER_DISPENSER sendPacket;
		sendPacket.cid = cid;
		sendPacket.playerObjectID = myObjectID;
		sendPacket.objectID = id;
		SendFixedPacket(sendPacket);
	}
	else {
		cout << "정수기 재사용 대기시간 : " << coolTime << "\n";
	}
}

void WaterDispenser::Interact() {
	coolTime = WATER_DISPENSER_COOLTIME;
}

void WaterDispenser::Animate(float _timeElapsed) {
	if (0 < coolTime) {
		coolTime -= _timeElapsed;
	}
}

bool WaterDispenser::IsInteractable(bool _isPlayerProfessor) {
	// 학생만 할 수 있다.
	return true;
}

bool WaterDispenser::IsEnable() {
	return coolTime <= 0;
}

///////////////////////////////////////////////////////////////////////////////
///

Lever::Lever() {
}

Lever::~Lever() {
}

void Lever::QueryInteract() {

}

void Lever::Interact() {
}

bool Lever::IsInteractable(bool _isPlayerProfessor) {
	// 누구나 할 수 있다.
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///

Computer::Computer() {
	power = true;
	hackingRate = 0.f;
	use = false;
}
Computer::~Computer() {
	
}

void Computer::QueryInteract() {
	if (IsEnable()) {
		CS_QUERY_USE_COMPUTER sendPacket;
		sendPacket.cid = cid;
		sendPacket.playerObjectID = myObjectID;
		sendPacket.computerObjectID = id;
		SendFixedPacket(sendPacket);
	}
}

void Computer::Interact() {
	cout << " 컴퓨터 해킹 시작 \n";
}

bool Computer::IsEnable() {
	// 전원이 들어와있으며 아무도 사용하지 않고, 해독완료되지 않은 컴퓨터
	return (use == 0 && power && hackingRate < 100.f);
}

void Computer::SetHackingRate(float _rate) {
	hackingRate = _rate;
}

void Computer::SetUse(UINT _use) {
	use = _use;
}

void Computer::SetPower(bool _power) {
	power = _power;
}


void Computer::Animate(float _timeElapsed) {
	float hackingSpeed = 5.0f;
	// 내가 컴퓨터를 사용중일 경우
	if (hackingRate < 100.0f && use == myObjectID && power) {
		cout << hackingRate << " % 해킹완료\n";
		hackingRate += 5.0f * _timeElapsed;
	}
	else if (hackingRate >= 100.0f && use == myObjectID) {
		use = 0;
		CS_HACKING_RATE sendPacket;
		sendPacket.cid = cid;
		sendPacket.computerObjectID = id;
		sendPacket.rate = hackingRate;
		SendFixedPacket(sendPacket);
		cout << "해킹 완료.\n";
	}
}
void Computer::EndInteract() {
	if (use == myObjectID) {
		use = 0;
		CS_HACKING_RATE sendPacket;
		sendPacket.cid = cid;
		sendPacket.computerObjectID = id;
		sendPacket.rate = hackingRate;
		SendFixedPacket(sendPacket);
		cout << "해킹 취소 \n ";
	}
}

float Computer::GetHackingRate() const {
	return hackingRate;
}

UINT Computer::GetUse() const {
	return use;
}
bool Computer::IsInteractable(bool _isPlayerProfessor) {
	// 학생만 할 수 있다.
	return !_isPlayerProfessor;
}
///////////////////////////////////////////////////////////////////////////////
///

SkyBox::SkyBox(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	// 큐브 오브젝트 리소스 생성
	for (int i = 0; i < 6; i++) {
		pMeshs[i] = make_shared<SkyBoxMesh>(i, _pDevice, _pCommandList);
		pTextures[i] = make_shared<Texture>(1, RESOURCE_TEXTURE2D, 0, 1);
	}
	// 앞뒤, 상하, 왼오
	pTextures[0]->LoadFromFile("SkyBox_Back_0", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 4);
	pTextures[1]->LoadFromFile("SkyBox_Front_0", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 4);
	pTextures[2]->LoadFromFile("SkyBox_Top_0", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 4);
	pTextures[3]->LoadFromFile("SkyBox_Bottom_0", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 4);
	pTextures[4]->LoadFromFile("SkyBox_Left_0", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 4);
	pTextures[5]->LoadFromFile("SkyBox_Right_0", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 4);


	// SRV 적재
	for (int i = 0; i < 6; i++) {
		Shader::CreateShaderResourceViews(_pDevice, pTextures[i], 0, 4);
	}

}




SkyBox::~SkyBox() {
}

void SkyBox::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	for (int i = 0; i < 6; ++i) {
		if (pTextures[i]) {
			pTextures[i]->UpdateShaderVariable(_pCommandList);
			if (pMeshs[i])
				pMeshs[i]->Render(_pCommandList);
		}
	}
}



Attack::Attack() {
	lifeTime = FLT_MAX;
	attackType = AttackType::none;
	playerObjectID = 0;
	damage = 0;
	isRemove = false;
}

Attack::Attack(UINT _playerObjectID) : Attack() {
	playerObjectID = _playerObjectID;
}

Attack::~Attack() {

}

void Attack::Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameObject::Create(_ObjectName, _pDevice, _pCommandList);
	if (GetObj()->GetObjectClass() != 2) {
		cout << _ObjectName << " 공격 생성 에러 : Effect가 아닙니다.\n";
		// 오브젝트는 항상 Effect여야 한다.
		return;
	}
}

void Attack::Animate(float _timeElapsed) {
	GameObject::Animate(_timeElapsed);

}


SwingAttack::SwingAttack() {
	attackType = AttackType::swingAttack;
	damage = 20.0f;
}

SwingAttack::SwingAttack(UINT _playerObjectID) : SwingAttack() {
	playerObjectID = _playerObjectID;
}

SwingAttack::~SwingAttack() {

}

void SwingAttack::Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	Attack::Create(_ObjectName, _pDevice, _pCommandList);
	auto ObjInfo = dynamic_pointer_cast<Effect>(GetObj());
	
	lifeTime = ObjInfo->GetMaxIndexTime();
}

void SwingAttack::Animate(float _timeElapsed) {
	Attack::Animate(_timeElapsed);
	lifeTime -= _timeElapsed;
	if (lifeTime < 0) {
		isRemove = true;
	}
}

ThrowAttack::ThrowAttack() {
	isStuck = false;
	lifeTime = 5.0f;
	attackType = AttackType::throwAttack;
	velocity = XMFLOAT3(0, 0, 0);
	damage = 20.0f;
	acc = 0.f;
	rotateXSpeed = 1080.0f;
}

ThrowAttack::ThrowAttack(UINT _playerObjectID, const XMFLOAT3& _lookVector) : ThrowAttack() {
	playerObjectID = _playerObjectID;
	velocity = Vector3::ScalarProduct(_lookVector, 25.0f);
}

ThrowAttack::~ThrowAttack() {
}

void ThrowAttack::Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	Attack::Create(_ObjectName, _pDevice, _pCommandList);
	damage = 20.0f;
}

void ThrowAttack::Animate(float _timeElapsed) {
	Attack::Animate(_timeElapsed);
	if (!isStuck) {
		Move(velocity, _timeElapsed);
		Rotate(GetLocalRightVector(), rotateXSpeed, _timeElapsed);
		acc += GRAVITY * _timeElapsed;
		velocity.y -= acc * _timeElapsed;
		UpdateObject();
	}

	lifeTime -= _timeElapsed;
	if (lifeTime < 0) {
		isRemove = true;
	}

}

void ThrowAttack::SetIsStuck(bool _isStuck) {
	isStuck = _isStuck;
}

bool ThrowAttack::GetIsStuck() const {
	return isStuck;
}
