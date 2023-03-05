#include "stdafx.h"
#include "Zone.h"
#include "GameFramework.h"



///////////////////////////////////////////////////////////////////////////////
/// Sector
Sector::Sector() {
	pGameObjectLayers.assign((UINT)SectorLayer::num, {});
}
Sector::~Sector() {

}

void Sector::AddObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pGameObject) {
	Layer& pGameObjectLayer = pGameObjectLayers[(int)_sectorLayer];
	auto it = pGameObjectLayer.find(_objectID);

	if (it == pGameObjectLayer.end()) {		// ��ü�� �������� ���� ���
		pGameObjectLayer[_objectID] = _pGameObject;	// �߰��Ѵ�.
	}
	else {
		cout << _objectID << "����! ";
		cout << format("���� : �ߺ��Ǵ� id���� �����մϴ� : {} vs {} \n", (*it).second->GetName(), _pGameObject->GetName());
	}
}
void Sector::RemoveObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pGameObject) {
	if (_pGameObject == nullptr) {
		cout << format("���� : ������Ʈ �����Ͱ� NULL�Դϴ�.\n");
		return;
	}

	Layer& pGameObjectLayer = pGameObjectLayers[(int)_sectorLayer];
	auto it = pGameObjectLayer.find(_objectID);

	if (it != pGameObjectLayer.end()) {		// ��ü�� ������ ���
		pGameObjectLayer.erase(it);
	}
	else {
		cout << format("���� : �ش� ID�� ������ ������Ʈ�� �����ϴ�.\n");
	}
}

void Sector::AddInteractObject(UINT _objectID, shared_ptr<GameObject> _pGameObject) {
	auto it = pInteractionObjects.find(_objectID);

	if (it == pInteractionObjects.end()) {		// ��ü�� �������� ���� ���
		pInteractionObjects[_objectID] = dynamic_pointer_cast<InteractObject>(_pGameObject);	// �߰��Ѵ�.
	}
	else {
		cout << _objectID << "����! ";
		cout << format("���� : �ߺ��Ǵ� id���� �����մϴ� : {} vs {} \n", (*it).second->GetName(), _pGameObject->GetName());
	}
}

void Sector::RemoveInteractObject(UINT _objectID, shared_ptr<GameObject> _pGameObject) {
	if (_pGameObject == nullptr) {
		cout << format("���� : ������Ʈ �����Ͱ� NULL�Դϴ�.\n");
		return;
	}

	auto it = pInteractionObjects.find(_objectID);

	if (it != pInteractionObjects.end()) {		// ��ü�� ������ ���
		pInteractionObjects.erase(it);
	}
	else {
		cout << format("���� : �ش� ID�� ������ ������Ʈ�� �����ϴ�.\n");
	}
}

shared_ptr<GameObject> Sector::FindObject(SectorLayer _sectorLayer, UINT _objectID) {
	Layer pGameObjectLayer = pGameObjectLayers[(int)_sectorLayer];
	auto it = pGameObjectLayer.find(_objectID);

	if (it != pGameObjectLayer.end()) {		// ��ü�� ������ ���
		return it->second;
	}
	else {		// �������� ���� ���
		cout << "ã������ ������Ʈ�� �����ϴ�.\n";
		return NULL;
	}
}

void Sector::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	for (auto pGameObjectLayer : pGameObjectLayers) {
		for (auto [gid, pGameObject] : pGameObjectLayer) {
			pGameObject->Render(_pCommandList);
		}
	}

}

void Sector::RenderHitBox(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, HitBoxMesh& _mesh) {

	for (auto pGameObjectLayer : pGameObjectLayers) {
		for (auto [gid, pGameObject] : pGameObjectLayer) {
			pGameObject->RenderHitBox(_pCommandList, _mesh);
		}
	}
}

void Sector::SetBoundingBox(const BoundingBox& _boundingBox) {
	boundingBox = _boundingBox;
}

const BoundingBox& Sector::GetBoundingBox() const {
	return boundingBox;
}

vector<shared_ptr<GameObject>>  Sector::CheckCollisionRotate(BoundingOrientedBox& _boundingBox, shared_ptr<GameObject> _pFloor) {

	vector<shared_ptr<GameObject>> result;
	for (auto [gid, pGameObject] : pGameObjectLayers[(UINT)SectorLayer::obstacle]) {
		if (_pFloor && _pFloor == pGameObject) continue;
		if (pGameObject->GetBoundingBox().Intersects(_boundingBox)) {
			result.push_back(pGameObject);
		}
	}
	return result;
}

shared_ptr<GameObject> Sector::CheckCollisionHorizontal(BoundingOrientedBox& _boundingBox, shared_ptr<Player> _pPlayer, shared_ptr<GameObject> _pFloor) {

	// �������� �ö� �� �ִ� �ִ� ���̰�
	float bias = 0.2f;

	for (auto [gid, pGameObject] : pGameObjectLayers[(UINT)SectorLayer::obstacle]) {
		if (_pFloor && _pFloor == pGameObject) continue;
		BoundingOrientedBox boundingBox = pGameObject->GetBoundingBox();

		if (boundingBox.Intersects(_boundingBox)) {
			// �ε������� ����� �ö󰥸��� ������ ���
			float heightGap = boundingBox.Extents.y + boundingBox.Center.y + _boundingBox.Extents.y - _boundingBox.Center.y;
			if (heightGap < bias) {
				// �÷��̾ �� ���̸�ŭ �̵�
				_pPlayer->MoveUp(bias);
			}
			else return pGameObject;
		}
	}
	return nullptr;
}

shared_ptr<GameObject> Sector::CheckCollisionVertical(BoundingOrientedBox& _boundingBox, shared_ptr<Player> _pPlayer, float _timeElapsed) {

	XMFLOAT3 vel = _pPlayer->GetVelocity();
	float displacement = vel.y * _timeElapsed;
	for (auto [gid, pGameObject] : pGameObjectLayers[(UINT)SectorLayer::obstacle]) {
		BoundingOrientedBox boundingBox = pGameObject->GetBoundingBox();
		if (boundingBox.Intersects(_boundingBox)) {
			// �̵� �ϱ��� �ٿ���ڽ��� ��ü�� ���ʿ� �ִ������� Ȯ��
			if (_boundingBox.Center.y - _boundingBox.Extents.y - displacement >= boundingBox.Center.y + boundingBox.Extents.y) {
				return pGameObject;
			}
			// õ�忡�� �ε��� ���
			if (boundingBox.Center.y - boundingBox.Extents.y > _boundingBox.Center.y + _boundingBox.Extents.y - displacement) {
				_pPlayer->SetVelocity(XMFLOAT3(vel.x, 0.f, vel.z));
			}
		}
	}
	return nullptr;
}

void Sector::CheckCollisionWithAttack(shared_ptr<Student> _pPlayer) {
	BoundingOrientedBox playerOOBB = _pPlayer->GetBoundingBox();
	for (auto [gid, pGameObject] : pGameObjectLayers[(UINT)SectorLayer::attack]) {

		shared_ptr<Attack> pAttack = dynamic_pointer_cast<Attack>(pGameObject);
		// ������ ������ �����ϰ�� ����
		if (pAttack->GetPlayerObjectID() == myObjectID) continue;

		BoundingOrientedBox boundingBox = pGameObject->GetBoundingBox();
		if (boundingBox.Intersects(playerOOBB)) {
			_pPlayer->AddHP(-pAttack->GetDamage());

			CS_ATTACK_HIT sendPacket;
			sendPacket.attackObjectID = gid;
			sendPacket.attackType = pAttack->GetAttackType();
			sendPacket.hitPlayerObjectID = myObjectID;
			sendPacket.cid = cid;

			SendFixedPacket(sendPacket);
			// �÷��̾�� �����ð��� ��� ����
			pAttack->Remove();
		}
	}
}

bool Sector::CheckCollisionProjectileWithObstacle(const BoundingOrientedBox& _boundingBox) {
	for (auto [gid, pObstacle] : pGameObjectLayers[(UINT)SectorLayer::obstacle]) {
		if (pObstacle->GetBoundingBox().Intersects(_boundingBox)) {
			return true;
		}
	}
	return false;
}


bool Sector::CheckObstacleBetweenPlayerAndCamera(const XMVECTOR& _origin, const XMVECTOR& _direction, float _curDistance) {
	for (auto [gid, pGameObject] : pGameObjectLayers[(UINT)SectorLayer::obstacle]) {
		BoundingOrientedBox boundingBox = pGameObject->GetBoundingBox();
		float dist;
		if (boundingBox.Intersects(_origin, _direction, dist) && dist <= _curDistance) {
			return true;
		}
	}
	return false;
}

pair<float, shared_ptr<InteractObject>> Sector::GetNearestInteractObject(const shared_ptr<Player>& _pPlayer, bool _isPlayerProfessor) {
	float minDist = 1.0f;
	float dist = 0.f;
	shared_ptr<InteractObject> pNearestObject;
	for (auto [gid, pGameObject] : pInteractionObjects) {
		// �� �÷��̾ ����� �� ���� ������Ʈ�� ��� �ǳʶڴ�.
		if(!pGameObject->IsInteractable(_isPlayerProfessor)) continue;

		BoundingOrientedBox boundingBox = pGameObject->GetBoundingBox();
		BoundingOrientedBox playerBoundingBox = _pPlayer->GetBoundingBox();
		// �Ÿ����� �ٻ�. �Ÿ��� ���� ������� �浹üũ�� ���� �ʴ´�.
		if (Vector3::Length(boundingBox.Center, playerBoundingBox.Center) - boundingBox.Extents.x - boundingBox.Extents.z > minDist) {
			continue;
		}
		// �÷��̾ �ٶ󺸴� �������� �ٿ�� �ڽ��� �̵����Ѻ���.
		playerBoundingBox.Center = Vector3::Add(playerBoundingBox.Center, _pPlayer->GetWorldLookVector(), playerBoundingBox.Extents.z * 2);
		if (playerBoundingBox.Intersects(boundingBox)) {
			minDist = dist;
			pNearestObject = pGameObject;
		}
	}
	return make_pair(dist, pNearestObject);
}


///////////////////////////////////////////////////////////////////////////////
/// Zone

// ������, �Ҹ���
Zone::Zone() {
	startPoint = XMFLOAT3(0, 0, 0);
	size = XMFLOAT3(0, 0, 0);
	div = XMINT3(0, 0, 0);
	sectorSize = XMFLOAT3(0, 0, 0);
	pindex = XMINT3(0, 0, 0);
	pid = 0;
}
Zone::Zone(const XMFLOAT3& _size, const XMINT3& _div, shared_ptr<PlayScene> _pScene) : size(_size), div(_div), pScene(_pScene) {
	sectors.assign(div.x, vector<vector<Sector>>(div.y, vector<Sector>(div.z, Sector())));
	sectorSize = Vector3::Division(size, div);
	startPoint = XMFLOAT3(-50, -50, -50);
	pindex = XMINT3(0, 0, 0);
	pid = 0;

	BoundingBox boundingBox;
	boundingBox.Extents = Vector3::ScalarProduct(sectorSize, 0.5f);
	for (int x = 0; x < _div.x; ++x) {
		for (int y = 0; y < _div.y; ++y) {
			for (int z = 0; z < _div.z; ++z) {
				// startPoint + (x,y,z = �ε���) * (sectorSize) + Extents;
				boundingBox.Center = Vector3::Add(Vector3::Add(startPoint, Vector3::Multiple(XMFLOAT3(x, y, z), sectorSize)), boundingBox.Extents);
				sectors[x][y][z].SetBoundingBox(boundingBox);
			}
		}
	}
}
Zone::~Zone() {
}


// ��ġ�� ���� �ε��� ���
XMINT3 Zone::GetIndex(const XMFLOAT3& _pos) {
	XMFLOAT3 indexFloat = Vector3::Division(Vector3::Subtract(_pos, startPoint), sectorSize);
	XMINT3 index((int)indexFloat.x, (int)indexFloat.y, (int)indexFloat.z);
	index = Vector3::Clamp(index, XMINT3(0, 0, 0), XMINT3(div.x - 1, div.y - 1, div.z - 1));
	return index;
}

// ���� ���
Sector* Zone::GetSector(const XMFLOAT3& _pos) {
	XMINT3 index = GetIndex(_pos);
	return &sectors[index.x][index.y][index.z];
}
Sector* Zone::GetSector(const XMINT3& _index) {
	return &sectors[_index.x][_index.y][_index.z];
}

// ������Ʈ �߰�
void Zone::AddObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMFLOAT3& _pos) {
	Sector* sector = GetSector(_pos);
	sector->AddObject(_sectorLayer, _objectID, _pObject);
}
void Zone::AddObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMINT3& _index) {
	Sector* sector = GetSector(_index);
	sector->AddObject(_sectorLayer, _objectID, _pObject);
}

void Zone::AddInteractObject(UINT _objectID, shared_ptr<GameObject> _pObject, const XMFLOAT3& _pos) {
	Sector* sector = GetSector(_pos);
	sector->AddInteractObject(_objectID, _pObject);
}
void Zone::AddInteractObject(UINT _objectID, shared_ptr<GameObject> _pObject, const XMINT3& _index) {
	Sector* sector = GetSector(_index);
	sector->AddInteractObject(_objectID, _pObject);
}

// ������Ʈ ����
void Zone::RemoveObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMFLOAT3& _pos) {
	Sector* sector = GetSector(_pos);
	sector->RemoveObject(_sectorLayer, _objectID, _pObject);
}
void Zone::RemoveObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMINT3& _index) {
	Sector* sector = GetSector(_index);
	sector->RemoveObject(_sectorLayer, _objectID, _pObject);
}

void Zone::RemoveInteractObject(UINT _objectID, shared_ptr<GameObject> _pObject, const XMFLOAT3& _pos) {
	Sector* sector = GetSector(_pos);
	sector->RemoveInteractObject(_objectID, _pObject);
}

void Zone::RemoveInteractObject(UINT _objectID, shared_ptr<GameObject> _pObject, const XMINT3& _index) {
	Sector* sector = GetSector(_index);
	sector->RemoveInteractObject(_objectID, _pObject);
}

// ������Ʈ�� �ٸ� ���ͷ� �̵�
void Zone::HandOffObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMFLOAT3& _prePos, const XMFLOAT3& _nextPos) {
	AddObject(_sectorLayer, _objectID, _pObject, _nextPos);		// �����߰��ϰ�
	RemoveObject(_sectorLayer, _objectID, _pObject, _prePos);	// ������ġ������ ����
}
void Zone::HandOffObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMINT3& _preIndex, const XMINT3& _nextIndex) {
	AddObject(_sectorLayer, _objectID, _pObject, _nextIndex);		// �����߰��ϰ�
	RemoveObject(_sectorLayer, _objectID, _pObject, _preIndex);	// ������ġ������ ����
}
// ���� ���������� �����ϴ� �Լ�
bool Zone::IsSameSector(const XMFLOAT3& _pos1, const XMFLOAT3& _pos2) {
	XMINT3 index1 = GetIndex(_pos1);
	XMINT3 index2 = GetIndex(_pos2);
	return index1.x == index2.x && index1.y == index2.y && index1.z == index2.z;
}

// �ֺ� ���� ���
vector<Sector*> Zone::GetAroundSectors(const XMINT3& _index) {
	vector<Sector*> sectors;
	for (int x = _index.x - 1; x <= _index.x + 1; ++x) {
		for (int y = _index.y - 1; y <= _index.y + 1; ++y) {
			for (int z = _index.z - 1; z <= _index.z + 1; ++z) {
				XMINT3 aroundIndex = XMINT3(x, y, z);
				if (IndexInside(aroundIndex)) {
					sectors.push_back(GetSector(aroundIndex));
				}
			}
		}
	}
	return sectors;
}


// ���������Ұ� �浹�ϴ� ���� ���
vector<Sector*> Zone::GetFrustumSectors(const BoundingFrustum& _frustum) {
	// ���������� �̿��ϸ� ��������
	vector<Sector*> result;
	for (int x = 0; x < div.x; ++x) {
		for (int y = 0; y < div.y; ++y) {
			for (int z = 0; z < div.z; ++z) {
				XMINT3 index = XMINT3(x, y, z);
				XMFLOAT3 extents = Vector3::ScalarProduct(sectorSize, 0.5f);
				//center = startPoint + index * sectorSize + extent;
				XMFLOAT3 center = Vector3::Add(Vector3::Add(startPoint, Vector3::Multiple(sectorSize, index)), extents);

				BoundingBox boundingBox(center, extents);
				if (_frustum.Intersects(boundingBox)) {
					result.push_back(GetSector(index));
				}
			}
		}
	}
	return result;
}

void Zone::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, shared_ptr<BoundingFrustum> _pBoundingFrustum) {
	GameFramework& gameFramework = GameFramework::Instance();



#ifdef USING_INSTANCING
	//gameFramework.GetShader("SkinnedShader")->Render(_pCommandList);
	gameFramework.GetShader("BasicShader")->Render(_pCommandList);
	GameObject::RenderInstanceObjects(_pCommandList);

	// �������� ������Ʈ�� �������� �׸���.
	gameFramework.GetShader("BlendingShader")->Render(_pCommandList);

#else
	gameFramework.GetShader("BasicShader")->PrepareRender(_pCommandList);
	for (auto& divx : sectors) {
		for (auto& divy : divx) {
			for (auto& sector : divy) {
				sector.Render(_pCommandList);
			}
		}
	}

	/*auto sectors = GetFrustumSectors(*_pBoundingFrustum);

	for (auto& sector : sectors) {
		sector->Render(_pCommandList);
	}*/



#endif
#ifdef DRAW_BOUNDING
	//HitBoxMesh& hitBoxMesh = gameFramework.GetHitBoxMesh();
	//gameFramework.GetShader("BoundingMeshShader")->PrepareRender(_pCommandList);


	//for (auto& divx : sectors) {
	//	for (auto& divy : divx) {
	//		for (auto& sector : divy) {
	//			sector.RenderHitBox(_pCommandList, hitBoxMesh);
	//		}
	//	}
	//}

#endif

}



void Zone::LoadZoneFromFile(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const array<UINT, MAX_PARTICIPANT>& _enableComputers) {
	GameFramework& gameFramework = GameFramework::Instance();

	ifstream file("Map", ios::binary);

	if (!file) {
		cout << "Zone File Load Failed!\n";
		return;
	}

	// ���ҽ��� ����� ���� �ӽ÷� ���� �ν��Ͻ��� ���� ��� ������ �����صд�.
	unordered_map<string, vector<XMFLOAT4X4>> instanceDatas;

	// �ʳ� ��ü �ν��Ͻ��� ����
	UINT nInstance;

	string objName;
	SectorLayer objLayer;
	ObjectType objType;

	XMFLOAT4X4 world, temp;

	XMFLOAT3 position, scale;
	XMFLOAT4 rotation;

	bool activeComputer;
	// nInstance (UINT)
	file.read((char*)&nInstance, sizeof(UINT));

	for (UINT objectID = 1; objectID <= nInstance; ++objectID) {
		// nameSize(UINT) / fileName (string)
		activeComputer = false;
		ReadStringBinary(objName, file);
		// SectorLayer(char)
		file.read((char*)&objLayer, sizeof(SectorLayer));
		// ObjectType(char)
		file.read((char*)&objType, sizeof(ObjectType));

		// position(float * 3) / scale(float * 3) / rotation(float * 3)
		file.read((char*)&position, sizeof(XMFLOAT3));
		file.read((char*)&scale, sizeof(XMFLOAT3));
		file.read((char*)&rotation, sizeof(XMFLOAT4));

		shared_ptr<GameObject> pGameObject;
		switch (objLayer) {
		case SectorLayer::obstacle: {
			switch (objType) {
			case ObjectType::Rdoor:
			case ObjectType::Ldoor:
			case ObjectType::exitRDoor:
			case ObjectType::exitLDoor: {
				pGameObject = make_shared<Door>(objType);
				AddInteractObject(objectID, pGameObject, GetIndex(position));
				pInteractObjTable[objectID] = dynamic_pointer_cast<InteractObject>(pGameObject);
				break;
			}
			case ObjectType::lever: {
				pGameObject = make_shared<Lever>();
				AddInteractObject(objectID, pGameObject, GetIndex(position));
				pInteractObjTable[objectID] = dynamic_pointer_cast<InteractObject>(pGameObject);
				break;
			}
			case ObjectType::waterDispenser: {
				pGameObject = make_shared<WaterDispenser>();
				AddInteractObject(objectID, pGameObject, GetIndex(position));
				pInteractObjTable[objectID] = dynamic_pointer_cast<InteractObject>(pGameObject);
				break;
			}
			case ObjectType::computer: {
				// Ȱ��ȭ�� ��ǻ���� ���
				if (find(_enableComputers.begin(), _enableComputers.end(), objectID) != _enableComputers.end()) {
					pGameObject = make_shared<Computer>();
					// Basic Shader�� �׸���.

					activeComputer = true;
					AddInteractObject(objectID, pGameObject, GetIndex(position));
					pInteractObjTable[objectID] = dynamic_pointer_cast<InteractObject>(pGameObject);
					pScene->AddComputer(dynamic_pointer_cast<Computer>(pGameObject));
				}
				else {
					continue;
					pGameObject = make_shared<GameObject>();

				}
				break;
			}
			default: {
				pGameObject = make_shared<GameObject>();
			}
			}
			pGameObject->Create(objName, _pDevice, _pCommandList);
			pGameObject->SetLocalPosition(position);
			pGameObject->SetLocalScale(scale);
			pGameObject->SetLocalRotation(rotation);
			pGameObject->UpdateObject();
			pGameObject->SetID(objectID);

			// ���Ϳ� ������Ʈ�� �߰��Ѵ�. (�浹üũ, �������� �ø���)
			AddObject(objLayer, objectID, pGameObject, GetIndex(position));

			if (activeComputer) {
				pGameObject->GetObj()->SetShaderType(ShaderType::basic);
				gameFramework.GetShader("BasicShader")->AddObject(pGameObject->GetObj());
			}

			if (pGameObject->GetObj()->GetShaderType() == ShaderType::instancing) {
				// �ν��Ͻ��� ����ϴ� �������� ������Ʈ�� �׸��� ���̴�
				// ���̴������� �д� ������ �޶����Ƿ� ��ġ��ķ� �ٲپ��ش�.
				world = pGameObject->GetWorldTransform();
				XMStoreFloat4x4(&temp, XMMatrixTranspose(XMLoadFloat4x4(&world)));
				instanceDatas[objName].push_back(temp);
			}
			break;
		}
		case SectorLayer::sprite: {
			pGameObject = make_shared<GameObject>();
			pGameObject->Create(objName, _pDevice, _pCommandList);
			pGameObject->SetLocalPosition(position);
			pGameObject->SetLocalScale(scale);
			pGameObject->SetLocalRotation(rotation);
			pGameObject->UpdateObject();
			pGameObject->SetID(objectID);
			if (pGameObject->GetObj()->GetShaderType() == ShaderType::instancing) {
				// �ν��Ͻ��� ����ϴ� �������� ������Ʈ�� �׸��� ���̴�
				// ���̴������� �д� ������ �޶����Ƿ� ��ġ��ķ� �ٲپ��ش�.
				world = pGameObject->GetWorldTransform();
				XMStoreFloat4x4(&temp, XMMatrixTranspose(XMLoadFloat4x4(&world)));
				instanceDatas[objName].push_back(temp);
			}
			break;
		}
		}
	}
#ifdef USING_INSTANCING
	gameFramework.GetGameObjectManager().InitInstanceResource(_pDevice, _pCommandList, instanceDatas);
#endif

}

vector<shared_ptr<GameObject>> Zone::CheckCollisionRotate(shared_ptr<GameObject> _pFloor) {
	// �÷��̾ ���Ե� ���� �� ������ ���͸� �����´�.
	vector<shared_ptr<GameObject>> result;
	vector<Sector*> checkSector = GetAroundSectors(pindex);

	// ���� �����ӿ� ����� �÷��̾��� ȸ����ŭ �ٿ���ڽ��� ȸ�����Ѻ���.
	BoundingOrientedBox boundingBox = pPlayer->GetBoundingBox();
	boundingBox.Orientation = Vector4::QuaternionMultiply(boundingBox.Orientation, pPlayer->GetRotation());
	vector<shared_ptr<GameObject>> objs;
	for (auto& sector : checkSector) {
		objs = sector->CheckCollisionRotate(boundingBox, _pFloor);
		for (auto& obj : objs) {
			result.push_back(obj);
		}
	}
	return result;
}

shared_ptr<GameObject> Zone::CheckCollisionHorizontal(shared_ptr<GameObject> _pFloor) {
	// �÷��̾ ���Ե� ���� �� ������ ���͸� �����´�.
	vector<Sector*> checkSector = GetAroundSectors(pindex);

	BoundingOrientedBox boundingBox = pPlayer->GetBoundingBox();
	XMFLOAT3 velocity = pPlayer->GetVelocity();
	// ���� �����ӿ� ����� �÷��̾��� �̵����͸�ŭ �ٿ���ڽ��� �̵����Ѻ���.
	XMFLOAT3 moveVector = Vector3::ScalarProduct(Vector3::Normalize(pPlayer->GetWorldLookVector()), velocity.z);
	boundingBox.Center = Vector3::Add(boundingBox.Center, moveVector);

	shared_ptr<GameObject> obj;
	for (auto& sector : checkSector) {
		obj = sector->CheckCollisionHorizontal(boundingBox, pPlayer, _pFloor);
		if (obj) {
			return obj;
		}
	}
	return nullptr;
}

shared_ptr<GameObject> Zone::CheckCollisionVertical(float _timeElapsed) {
	// �÷��̾ ���Ե� ���� �� ������ ���͸� �����´�.
	vector<Sector*> checkSector = GetAroundSectors(pindex);

	BoundingOrientedBox boundingBox = pPlayer->GetBoundingBox();
	XMFLOAT3 velocity = pPlayer->GetVelocity();
	boundingBox.Center.y += velocity.y * _timeElapsed;

	shared_ptr<GameObject> obj;
	for (auto& sector : checkSector) {
		obj = sector->CheckCollisionVertical(boundingBox, pPlayer);
		if (obj) {
			return obj;
		}
	}
	return nullptr;
}

void Zone::CheckCollisionWithAttack() {
	vector<Sector*> checkSector = GetAroundSectors(pindex);
	auto pStudent = dynamic_pointer_cast<Student>(pPlayer);
	if (pStudent) {
		for (auto& sector : checkSector) {
			sector->CheckCollisionWithAttack(pStudent);
		}
	}

}

void Zone::CheckCollisionProjectileWithObstacle() {

	BoundingOrientedBox boundingBox;
	for (auto& [objectID, pAttack] : pAttackObjTable) {
		if (pAttack->GetAttackType() == AttackType::throwAttack) {	// ����ü�� ���
			boundingBox = pAttack->GetBoundingBox();
			// �ֺ������� ��ֹ���� �浹üũ�� �Ѵ�.
			vector<Sector*> pSectors = GetAroundSectors(GetIndex(pAttack->GetWorldPosition()));
				auto pThrowAttack = dynamic_pointer_cast<ThrowAttack>(pAttack);

				for (const auto& pSector : pSectors) {
					if (!pThrowAttack->GetIsStuck() && pSector->CheckCollisionProjectileWithObstacle(boundingBox)) {
						dynamic_pointer_cast<ThrowAttack>(pAttack)->SetIsStuck(true);
					}
				}
		}
	}
}

bool Zone::CheckObstacleBetweenPlayerAndCamera(shared_ptr<Camera> _pCamera) {
	XMFLOAT3 cameraWorldPosition = _pCamera->GetWorldPosition();
	XMFLOAT3 cameraWorldLookVector = _pCamera->GetWorldLookVector();
	XMVECTOR xmCameraWorldPosition = XMLoadFloat3(&cameraWorldPosition);
	XMVECTOR xmCameraWorldLookVector = XMLoadFloat3(&cameraWorldLookVector);

	vector<Sector*> sectors = GetAroundSectors(GetIndex(_pCamera->GetWorldPosition()));
	for (auto& sector : sectors) {
		if (sector->CheckObstacleBetweenPlayerAndCamera(xmCameraWorldPosition, xmCameraWorldLookVector, _pCamera->GetCurrentDistance())) {
			return true;
		}
	}
	return false;
}

void Zone::UpdatePlayerSector() {
	XMINT3 prevIndex = pindex;
	pindex = GetIndex(pPlayer->GetWorldPosition());
}

void Zone::AnimateObjects(float _timeElapsed) {
	XMINT3 prevIndex, nextIndex;
	for (auto [objectID, pGameObject] : pInteractObjTable) {	// ��ȣ�ۿ� ������Ʈ
		pGameObject->Animate(_timeElapsed);
	}

	for (auto& [objectID, pAttack] : pAttackObjTable) {	// ���� ������Ʈ
		prevIndex = GetIndex(pAttack->GetWorldPosition());
		// ��������� ������ ����, �Ҽ� ���� ������Ʈ
		if (pAttack->GetIsRemove()) {
			GetSector(pAttack->GetWorldPosition())->RemoveObject(SectorLayer::attack, objectID, pAttack);
			pAttackObjTable.erase(objectID);
			continue;
		}

		pAttack->Animate(_timeElapsed);
		nextIndex = GetIndex(pAttack->GetWorldPosition());

		if (prevIndex.x != nextIndex.x || prevIndex.y != nextIndex.y || prevIndex.z != nextIndex.z) {
			HandOffObject(SectorLayer::attack, pAttack->GetID(), pAttack, prevIndex, nextIndex);

		}
	}
}

shared_ptr<InteractObject> Zone::UpdateInteractableObject(bool _isPlayerProfessor) {
	shared_ptr<InteractObject> nearestObject = nullptr;
	float minDist = FLT_MAX;

	vector<Sector*> checkSector = GetAroundSectors(pindex);
	// ���͸��� �÷��̾ �ٶ󺸰� �����鼭 ���� ����� ������Ʈ�� ��ȯ
	for (auto& sector : checkSector) {

		auto [dist, pGameObject] = sector->GetNearestInteractObject(pPlayer, _isPlayerProfessor);
		if (pGameObject) {
			if (minDist > dist) {
				minDist = dist;
				nearestObject = pGameObject;
			}
		}
	}
	// �ٶ󺸴� ������ ���� ����� ������Ʈ�� ��ȯ�Ѵ�.
	return nearestObject;
}

shared_ptr<Attack> Zone::GetAttack(UINT _objectID) {
	if (pAttackObjTable[_objectID]) return pAttackObjTable[_objectID];
	return nullptr;
}

void Zone::AddAttack(AttackType _attackType, UINT _objectID, shared_ptr<GameObject> _pPlayerObject, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	shared_ptr<Attack> pAttack;
	XMFLOAT3 offset = XMFLOAT3(0.f, 0.8f, 0.f);	// ������ ���� ��ġ ������
	// �ش� �÷��̾��� ��ġ, ȸ������ �����ϰ� ��ġ�� �´� ���Ϳ� �߰��Ѵ�.
	if (_attackType == AttackType::swingAttack) {
		pAttack = make_shared<SwingAttack>(_pPlayerObject->GetID());
		pAttack->Create("SwingAttack", _pDevice, _pCommandList);
		pAttack->SetLocalPosition(Vector3::Add(_pPlayerObject->GetWorldPosition(), offset));
		pAttack->SetLocalRotation(_pPlayerObject->GetLocalRotate());
		pAttack->UpdateObject();
		AddObject(SectorLayer::attack, _objectID, pAttack, pAttack->GetWorldPosition());

		pAttack->SetID(_objectID);
	}
	else if (_attackType == AttackType::throwAttack) {

		pAttack = make_shared<ThrowAttack>(_pPlayerObject->GetID(), _pPlayerObject->GetWorldLookVector());
		pAttack->Create("ThrowAttack", _pDevice, _pCommandList);
		pAttack->SetLocalPosition(Vector3::Add(_pPlayerObject->GetWorldPosition(), offset));
		pAttack->SetLocalRotation(_pPlayerObject->GetLocalRotate());
		pAttack->UpdateObject();
		AddObject(SectorLayer::attack, _objectID, pAttack, pAttack->GetWorldPosition());

		pAttack->SetID(_objectID);
	}
	pAttackObjTable[_objectID] = pAttack;
}

void Zone::RemoveAttack(UINT _objectID) {
	pAttackObjTable[_objectID]->Remove();
}

void Zone::Interact(UINT _objectID) {
	shared_ptr<InteractObject> pGameObject = pInteractObjTable[_objectID];
	if (pGameObject)
		pGameObject->Interact();
	else
		cout << "InteractObject Null Error. objID : " << _objectID << "\n";
}

