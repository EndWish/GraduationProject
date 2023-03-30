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

	if (it == pGameObjectLayer.end()) {		// 객체가 존재하지 않을 경우
		pGameObjectLayer[_objectID] = _pGameObject;	// 추가한다.
	}
	else {
		cout << _objectID << "오류! ";
		cout << format("버그 : 중복되는 id값이 존재합니다 : {} vs {} \n", (*it).second->GetName(), _pGameObject->GetName());
	}
}
void Sector::RemoveObject(SectorLayer _sectorLayer, UINT _objectID) {

	Layer& pGameObjectLayer = pGameObjectLayers[(int)_sectorLayer];
	auto it = pGameObjectLayer.find(_objectID);

	if (it != pGameObjectLayer.end()) {		// 객체가 존재할 경우
		pGameObjectLayer.erase(it);
	}
	else {
		cout << format("버그 : 해당 ID를 가지는 오브젝트가 없습니다.\n");
	}
}

void Sector::AddInteractObject(UINT _objectID, shared_ptr<GameObject> _pGameObject) {
	auto it = pInteractionObjects.find(_objectID);

	if (it == pInteractionObjects.end()) {		// 객체가 존재하지 않을 경우
		pInteractionObjects[_objectID] = dynamic_pointer_cast<InteractObject>(_pGameObject);	// 추가한다.
	}
	else {
		cout << _objectID << "오류! ";
		cout << format("버그 : 중복되는 id값이 존재합니다 : {} vs {} \n", (*it).second->GetName(), _pGameObject->GetName());
	}
}

void Sector::RemoveInteractObject(UINT _objectID, shared_ptr<GameObject> _pGameObject) {
	if (_pGameObject == nullptr) {
		cout << format("버그 : 오브젝트 포인터가 NULL입니다.\n");
		return;
	}

	auto it = pInteractionObjects.find(_objectID);

	if (it != pInteractionObjects.end()) {		// 객체가 존재할 경우
		pInteractionObjects.erase(it);
	}
	else {
		cout << format("버그 : 해당 ID를 가지는 오브젝트가 없습니다.\n");
	}
}

shared_ptr<GameObject> Sector::FindObject(SectorLayer _sectorLayer, UINT _objectID) {
	Layer pGameObjectLayer = pGameObjectLayers[(int)_sectorLayer];
	auto it = pGameObjectLayer.find(_objectID);

	if (it != pGameObjectLayer.end()) {		// 객체가 존재할 경우
		return it->second;
	}
	else {		// 존재하지 않을 경우
		cout << "찾으려는 오브젝트가 없습니다.\n";
		return NULL;
	}
}

vector<shared_ptr<GameObject>> Sector::GetObjectsByLayer(SectorLayer _sectorLayer) {
	vector<shared_ptr<GameObject>> result;
	for (auto [objectID, pGameObject] : pGameObjectLayers[(int)_sectorLayer]) {
		result.push_back(pGameObject);
	}
	return result;
}

void Sector::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	for (auto pGameObjectLayer : pGameObjectLayers) {
		for (auto [objectID, pGameObject] : pGameObjectLayer) {
			pGameObject->Render(_pCommandList);
		}
	}

}

void Sector::RenderHitBox(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, HitBoxMesh& _mesh) {

	for (auto pGameObjectLayer : pGameObjectLayers) {
		for (auto [objectID, pGameObject] : pGameObjectLayer) {
			pGameObject->RenderHitBox(_pCommandList, _mesh);
		}
	}
}

bool Sector::SetVisiblePlayer(shared_ptr<Camera> _pCamera, const XMFLOAT3& _playerCenter) {
	XMFLOAT3 cameraWorldPosition = _pCamera->GetWorldPosition();
	XMFLOAT3 rayWorldDirection = Vector3::Normalize(Vector3::Subtract(_playerCenter, cameraWorldPosition));

	float playerDistance = Vector3::Length(_playerCenter, cameraWorldPosition);
	XMVECTOR xmCameraWorldPosition = XMLoadFloat3(&cameraWorldPosition);
	XMVECTOR xmRayWorldDirection = XMLoadFloat3(&rayWorldDirection);
	float distance;
	// 카메라에서 다른플레이어로의 광선이 다른 물체와 부딪혀 보이지 않는경우를 판단

	for (auto [objectID, pObject] : pGameObjectLayers[(int)SectorLayer::obstacle]) {
		if (pObject->GetBoundingBox().Intersects(xmCameraWorldPosition, xmRayWorldDirection, distance)) {
			// 플레이어와의 거리보다 먼 오브젝트는 무시한다.
			if (distance > playerDistance) continue;

			return false;
		}
	}
	return true;
}

void Sector::SetBoundingBox(const BoundingBox& _boundingBox) {
	boundingBox = _boundingBox;
}

const BoundingBox& Sector::GetBoundingBox() const {
	return boundingBox;
}

vector<shared_ptr<GameObject>>  Sector::CheckCollisionRotate(BoundingOrientedBox& _boundingBox, shared_ptr<GameObject> _pFloor) {

	vector<shared_ptr<GameObject>> result;
	for (auto [objectID, pGameObject] : pGameObjectLayers[(UINT)SectorLayer::obstacle]) {
		if (_pFloor && _pFloor == pGameObject) continue;
		if (pGameObject->GetBoundingBox().Intersects(_boundingBox)) {
			result.push_back(pGameObject);
		}
	}
	for (auto [objectID, pGameObject] : pGameObjectLayers[(UINT)SectorLayer::otherPlayer]) {
		if (_pFloor && _pFloor == pGameObject) continue;
		if (pGameObject->GetBoundingBox().Intersects(_boundingBox)) {
			result.push_back(pGameObject);
		}
	}
	return result;
}

shared_ptr<GameObject> Sector::CheckCollisionHorizontal(BoundingOrientedBox& _boundingBox, shared_ptr<Player> _pPlayer, shared_ptr<GameObject> _pFloor) {

	// 점프없이 올라갈 수 있는 최대 높이값
	float bias = 0.2f;

	for (auto [objectID, pGameObject] : pGameObjectLayers[(UINT)SectorLayer::obstacle]) {
		if (_pFloor && _pFloor == pGameObject) continue;
		BoundingOrientedBox boundingBox = pGameObject->GetBoundingBox();

		if (boundingBox.Intersects(_boundingBox)) {
			// 부딪혔지만 충분히 올라갈만한 높이일 경우
			float heightGap = boundingBox.Extents.y + boundingBox.Center.y + _boundingBox.Extents.y - _boundingBox.Center.y;
			if (heightGap < bias) {
				// 플레이어를 그 높이만큼 이동
				_pPlayer->MoveUp(bias);
			}
			else return pGameObject;
		}
	}
	for (auto [objectID, pGameObject] : pGameObjectLayers[(UINT)SectorLayer::otherPlayer]) {
		if (_pFloor && _pFloor == pGameObject) continue;
		BoundingOrientedBox boundingBox = pGameObject->GetBoundingBox();

		if (boundingBox.Intersects(_boundingBox)) {
			// 부딪혔지만 충분히 올라갈만한 높이일 경우
			float heightGap = boundingBox.Extents.y + boundingBox.Center.y + _boundingBox.Extents.y - _boundingBox.Center.y;
			if (heightGap < bias) {
				// 플레이어를 그 높이만큼 이동
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
	for (auto [objectID, pGameObject] : pGameObjectLayers[(UINT)SectorLayer::obstacle]) {
		BoundingOrientedBox boundingBox = pGameObject->GetBoundingBox();
		if (boundingBox.Intersects(_boundingBox)) {
			// 이동 하기전 바운딩박스가 물체의 위쪽에 있던것인지 확인
			if (_boundingBox.Center.y - _boundingBox.Extents.y - displacement >= boundingBox.Center.y + boundingBox.Extents.y) {

				return pGameObject;
			}
			// 천장에서 부딪힌 경우
			if (boundingBox.Center.y - boundingBox.Extents.y > _boundingBox.Center.y + _boundingBox.Extents.y - displacement) {
				_pPlayer->SetVelocity(XMFLOAT3(vel.x, 0.f, vel.z));
			}
		}
	}
	for (auto [objectID, pGameObject] : pGameObjectLayers[(UINT)SectorLayer::otherPlayer]) {
		BoundingOrientedBox boundingBox = pGameObject->GetBoundingBox();
		if (boundingBox.Intersects(_boundingBox)) {
			// 이동 하기전 바운딩박스가 물체의 위쪽에 있던것인지 확인
			if (_boundingBox.Center.y - _boundingBox.Extents.y - displacement >= boundingBox.Center.y + boundingBox.Extents.y) {
				return pGameObject;
			}
			// 천장에서 부딪힌 경우
			if (boundingBox.Center.y - boundingBox.Extents.y > _boundingBox.Center.y + _boundingBox.Extents.y - displacement) {
				_pPlayer->SetVelocity(XMFLOAT3(vel.x, 0.f, vel.z));
			}
		}
	}
	return nullptr;
}

void Sector::CheckCollisionWithAttack(shared_ptr<Student> _pPlayer) {
	BoundingOrientedBox playerOOBB = _pPlayer->GetBoundingBox();
	for (auto [objectID, pGameObject] : pGameObjectLayers[(UINT)SectorLayer::attack]) {

		shared_ptr<Attack> pAttack = dynamic_pointer_cast<Attack>(pGameObject);

		
		BoundingOrientedBox boundingBox = pGameObject->GetBoundingBox();

		if (boundingBox.Intersects(playerOOBB)) {
			// 박혀있는 투사체일경우 건너뛴다.
			auto pThrowAttack = dynamic_pointer_cast<ThrowAttack>(pAttack);
			if (pThrowAttack && pThrowAttack->GetIsStuck()) {
				continue;
			}
			_pPlayer->AddHP(-pAttack->GetDamage());

			// 플레이어의 체력이 0이 되었을 경우 자신을 감옥으로 이동시키고 패킷을 보낸다.
			auto pStudent = dynamic_pointer_cast<Student>(_pPlayer);
			if (pStudent->GetHP() <= 0) {
				// 수감중으로 바꾼다.
				pStudent->SetImprisoned(true);
				// 체력을 반피로 바꾼다.
				pStudent->SetHP(50.f);
				// 순간이동 시킨다.
				pStudent->SetLocalPosition(prisonPosition);
				pStudent->UpdateObject();

				// 패킷을 보낸다.
				CS_GO_PRISON sendPacket;
				sendPacket.cid = cid;
				sendPacket.playerObjectID = myObjectID;
				SendFixedPacket(sendPacket);
			}
			// 맞고 생존한 경우
			CS_ATTACK_HIT sendAttackPacket;
			sendAttackPacket.attackObjectID = objectID;
			sendAttackPacket.attackType = pAttack->GetAttackType();
			sendAttackPacket.hitPlayerObjectID = myObjectID;
			sendAttackPacket.cid = cid;
			SendFixedPacket(sendAttackPacket);
			// 플레이어에게 무적시간을 잠시 적용
			
			pAttack->Remove();
		}
	}
}

void Sector::CheckCollisionWithItem(shared_ptr<Student> _pPlayer) {
	BoundingOrientedBox playerOOBB = _pPlayer->GetBoundingBox();
	for (auto [objectID, pGameObject] : pGameObjectLayers[(UINT)SectorLayer::item]) {

		shared_ptr<Item> pItem = static_pointer_cast<Item>(pGameObject);

		BoundingOrientedBox boundingBox = pGameObject->GetBoundingBox();

		// 플레이어가 그 아이템과 충돌시 아이템을 삭제하고 플레이어에게 아이템을 준다.
		// 해당사실을 서버에 알린다.
		if (boundingBox.Intersects(playerOOBB)) {
			_pPlayer->SetItem(pItem->GetType());

			// UI 갱신
			shared_ptr<Image2D> pFrameUI = Scene::GetUI("2DUI_leftSkill");
			shared_ptr<Image2D> pUI;
			pFrameUI->SetEnable(false);
			switch (pItem->GetType()) {
			case ObjectType::prisonKeyItem:
				pUI = Scene::GetUI("2DUI_prisonKey");
				break;
			case ObjectType::trapItem:
				pUI = Scene::GetUI("2DUI_trap");
				break;
			case ObjectType::medicalKitItem:
				pUI = Scene::GetUI("2DUI_medicalKit");
				break;
			case ObjectType::energyDrinkItem:
				pUI = Scene::GetUI("2DUI_energyDrink");
				break;
			}
			pUI->SetEnable(true);


			CS_REMOVE_ITEM sendPacket;
			sendPacket.cid = cid;
			sendPacket.itemLocationIndex = pItem->GetIndex();
			
			sendPacket.itemObjectID = objectID;
			sendPacket.itemType = pItem->GetType();
			sendPacket.playerObjectID = myObjectID;

			pItem->Remove();
			SendFixedPacket(sendPacket);
			return;
		}
	}
}

void Sector::CheckCollisionWithTrap(shared_ptr<Professor> _pPlayer) {
	BoundingOrientedBox playerOOBB = _pPlayer->GetBoundingBox();
	for (auto [objectID, pGameObject] : pGameObjectLayers[(UINT)SectorLayer::trap]) {

		shared_ptr<Trap> pTrap = dynamic_pointer_cast<Trap>(pGameObject);
		BoundingOrientedBox boundingBox = pGameObject->GetBoundingBox();
		if (boundingBox.Intersects(playerOOBB)) {
			pTrap->Remove();
			// 둔화를 적용하고 해당 트랩이 사라졌다는 패킷을 보낸다.
			CS_REMOVE_TRAP packet;
			packet.cid = cid;
			packet.trapObjectID = pTrap->GetID();
			cout << packet.trapObjectID << " 를 밟음\n";
			_pPlayer->SetSlowRate(pTrap->GetSlowRate());
			_pPlayer->SetSlowTime(pTrap->GetSlowTime());
			SendFixedPacket(packet);
		}
	}
}

bool Sector::CheckCollisionProjectileWithObstacle(const BoundingOrientedBox& _boundingBox) {
	for (auto [objectID, pObstacle] : pGameObjectLayers[(UINT)SectorLayer::obstacle]) {
		if (pObstacle->GetBoundingBox().Intersects(_boundingBox)) {
			return true;
		}
	}
	return false;
}


bool Sector::CheckObstacleBetweenPlayerAndCamera(const XMVECTOR& _origin, const XMVECTOR& _direction, float _curDistance) {
	for (auto [objectID, pGameObject] : pGameObjectLayers[(UINT)SectorLayer::obstacle]) {
		BoundingOrientedBox boundingBox = pGameObject->GetBoundingBox();
		float dist;
		if (boundingBox.Intersects(_origin, _direction, dist) && dist <= _curDistance) {
			return true;
		}
	}
	return false;
}

pair<float, shared_ptr<InteractObject>> Sector::GetNearestInteractObject(const shared_ptr<Player>& _pPlayer) {
	float minDist = 1.0f;
	float dist = 0.f;
	shared_ptr<InteractObject> pNearestObject;
	for (auto [objectID, pGameObject] : pInteractionObjects) {
		// 그 플레이어가 사용할 수 없는 오브젝트일 경우 건너뛴다.
		if(!pGameObject->IsInteractable()) continue;

		BoundingOrientedBox boundingBox = pGameObject->GetBoundingBox();
		BoundingOrientedBox playerBoundingBox = _pPlayer->GetBoundingBox();
		// 거리값을 근사. 거리가 되지 않을경우 충돌체크를 하지 않는다.
		if (Vector3::Length(boundingBox.Center, playerBoundingBox.Center) - boundingBox.Extents.x - boundingBox.Extents.z > minDist) {
			continue;
		}
		// 플레이어가 바라보는 방향으로 바운딩 박스를 이동시켜본다.
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

// 생성자, 소멸자
Zone::Zone() {
	startPoint = XMFLOAT3(0, 0, 0);
	size = XMFLOAT3(0, 0, 0);
	div = XMINT3(0, 0, 0);
	sectorSize = XMFLOAT3(0, 0, 0);
	pindex = XMINT3(0, 0, 0);
	pid = 0;
}
Zone::Zone(const XMFLOAT3& _size, const XMINT3& _div, shared_ptr<PlayScene> _pScene) : size(_size), div(_div), wpScene(_pScene) {
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
				// startPoint + (x,y,z = 인덱스) * (sectorSize) + Extents;
				boundingBox.Center = Vector3::Add(Vector3::Add(startPoint, Vector3::Multiple(XMFLOAT3((float)x, (float)y, (float)z), sectorSize)), boundingBox.Extents);
				sectors[x][y][z].SetBoundingBox(boundingBox);
			}
		}
	}
}
Zone::~Zone() {
}


// 위치로 부터 인덱스 얻기
XMINT3 Zone::GetIndex(const XMFLOAT3& _pos) {
	XMFLOAT3 indexFloat = Vector3::Division(Vector3::Subtract(_pos, startPoint), sectorSize);
	XMINT3 index((int)indexFloat.x, (int)indexFloat.y, (int)indexFloat.z);
	index = Vector3::Clamp(index, XMINT3(0, 0, 0), XMINT3(div.x - 1, div.y - 1, div.z - 1));
	return index;
}

// 섹터 얻기
Sector* Zone::GetSector(const XMFLOAT3& _pos) {
	XMINT3 index = GetIndex(_pos);
	return &sectors[index.x][index.y][index.z];
}
Sector* Zone::GetSector(const XMINT3& _index) {
	return &sectors[_index.x][_index.y][_index.z];
}

// 오브젝트 추가
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

// 오브젝트 제거
void Zone::RemoveObject(SectorLayer _sectorLayer, UINT _objectID, const XMFLOAT3& _pos) {
	Sector* sector = GetSector(_pos);
	sector->RemoveObject(_sectorLayer, _objectID);
}
void Zone::RemoveObject(SectorLayer _sectorLayer, UINT _objectID, const XMINT3& _index) {
	Sector* sector = GetSector(_index);
	sector->RemoveObject(_sectorLayer, _objectID);
}

void Zone::RemoveInteractObject(UINT _objectID, shared_ptr<GameObject> _pObject, const XMFLOAT3& _pos) {
	Sector* sector = GetSector(_pos);
	sector->RemoveInteractObject(_objectID, _pObject);
}

void Zone::RemoveInteractObject(UINT _objectID, shared_ptr<GameObject> _pObject, const XMINT3& _index) {
	Sector* sector = GetSector(_index);
	sector->RemoveInteractObject(_objectID, _pObject);
}

// 오브젝트를 다른 섹터로 이동
void Zone::HandOffObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMFLOAT3& _prePos, const XMFLOAT3& _nextPos) {
	AddObject(_sectorLayer, _objectID, _pObject, _nextPos);		// 새로추가하고
	RemoveObject(_sectorLayer, _objectID, _prePos);	// 이전위치에서는 제거
}
void Zone::HandOffObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMINT3& _preIndex, const XMINT3& _nextIndex) {
	AddObject(_sectorLayer, _objectID, _pObject, _nextIndex);		// 새로추가하고
	RemoveObject(_sectorLayer, _objectID, _preIndex);	// 이전위치에서는 제거
}
// 같은 섹터인지를 리턴하는 함수
bool Zone::IsSameSector(const XMFLOAT3& _pos1, const XMFLOAT3& _pos2) {
	XMINT3 index1 = GetIndex(_pos1);
	XMINT3 index2 = GetIndex(_pos2);
	return index1.x == index2.x && index1.y == index2.y && index1.z == index2.z;
}

// 주변 섹터 얻기
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


// 뷰프러스텀과 충돌하는 섹터 얻기
vector<Sector*> Zone::GetFrustumSectors(const BoundingFrustum& _frustum) {
	// 분할정복을 이용하면 개선가능
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
	gameFramework.GetShader("BasicShader")->Render(_pCommandList);
	gameFramework.GetShader("InstancingShader")->Render(_pCommandList);
	gameFramework.GetShader("SkinnedShader")->Render(_pCommandList);
	gameFramework.GetShader("SkinnedTransparentShader")->Render(_pCommandList);

#else
	gameFramework.GetShader("BasicShader")->PrepareRender(_pCommandList);
	gameFramework.GetShader("SkinnedShader")->Render(_pCommandList);
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
	HitBoxMesh& hitBoxMesh = gameFramework.GetHitBoxMesh();
	gameFramework.GetShader("BoundingMeshShader")->PrepareRender(_pCommandList);


	for (auto& divx : sectors) {
		for (auto& divy : divx) {
			for (auto& sector : divy) {
				sector.RenderHitBox(_pCommandList, hitBoxMesh);
			}
		}
	}
	pPlayer->RenderHitBox(_pCommandList, hitBoxMesh);

#endif

}



void Zone::LoadZoneFromFile(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const array<UINT, MAX_PARTICIPANT>& _enableComputers) {
	GameFramework& gameFramework = GameFramework::Instance();

	ifstream file("Map", ios::binary);

	if (!file) {
		cout << "Zone File Load Failed!\n";
		return;
	}

	// 리소스를 만들기 위해 임시로 정적 인스턴스의 월드 행렬 정보를 저장해둔다.
	unordered_map<string, vector<XMFLOAT4X4>> instanceDatas;

	// 맵내 전체 인스턴스의 개수
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
			case ObjectType::prisonDoor:
			case ObjectType::exitRDoor:
			case ObjectType::exitLDoor: {
				pGameObject = make_shared<Door>(objType);
				AddInteractObject(objectID, pGameObject, GetIndex(position));
				pInteractObjTable[objectID] = static_pointer_cast<InteractObject>(pGameObject);
				break;
			}
			case ObjectType::lever: {
				pGameObject = make_shared<Lever>();
				AddInteractObject(objectID, pGameObject, GetIndex(position));
				pInteractObjTable[objectID] = static_pointer_cast<InteractObject>(pGameObject);
				break;
			}
			case ObjectType::waterDispenser: {
				pGameObject = make_shared<WaterDispenser>();
				AddInteractObject(objectID, pGameObject, GetIndex(position));
				pInteractObjTable[objectID] = static_pointer_cast<InteractObject>(pGameObject);
				break;
			}
			case ObjectType::computer: {
				// 활성화된 컴퓨터일 경우
				if (find(_enableComputers.begin(), _enableComputers.end(), objectID) != _enableComputers.end()) {
					pGameObject = make_shared<Computer>();
					// Basic Shader로 그린다.

					activeComputer = true;
					AddInteractObject(objectID, pGameObject, GetIndex(position));
					pInteractObjTable[objectID] = static_pointer_cast<InteractObject>(pGameObject);
					wpScene.lock()->AddComputer(static_pointer_cast<Computer>(pGameObject));
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

			// 섹터에 오브젝트를 추가한다. (충돌체크, 프러스텀 컬링용)
			AddObject(objLayer, objectID, pGameObject, GetIndex(position));

			if (activeComputer) {
				pGameObject->GetObj()->SetShaderType(ShaderType::basic);
				gameFramework.GetShader("BasicShader")->AddObject(pGameObject->GetObj());
			}

			if (pGameObject->GetObj()->GetShaderType() == ShaderType::instancing) {
				// 인스턴싱을 사용하는 불투명한 오브젝트를 그리는 쉐이더
				// 쉐이더에서는 읽는 기준이 달라지므로 전치행렬로 바꾸어준다.
				world = pGameObject->GetWorldTransform();
				XMStoreFloat4x4(&temp, XMMatrixTranspose(XMLoadFloat4x4(&world)));
				instanceDatas[objName].push_back(temp);
			}
			break;
		}
		case SectorLayer::light: {
			pGameObject = make_shared<GameObject>();
			pGameObject->Create(objName, _pDevice, _pCommandList);
			pGameObject->SetLocalPosition(position);
			pGameObject->SetLocalScale(scale);
			pGameObject->SetLocalRotation(rotation);
			pGameObject->UpdateObject();
			pGameObject->SetID(objectID);

			// 섹터에 오브젝트를 추가한다. (충돌체크, 프러스텀 컬링용)
			AddObject(SectorLayer::obstacle, objectID, pGameObject, GetIndex(position));
			gameFramework.GetShader("BasicShader")->AddObject(pGameObject->GetObj());

			// 빛을 추가한다.
			shared_ptr<Light> pNewLight = make_shared<Light>();

			pNewLight->lightType = 2;
			pNewLight->position = Vector3::Subtract(position, XMFLOAT3(0, 0.1f, 0));
			pNewLight->direction = XMFLOAT3(0.f, -1.f, 0.f);
			pNewLight->diffuse = XMFLOAT4(2.5f, 3.5f, 2.5f, 1.f);
			pNewLight->ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.f);
			pNewLight->specular = XMFLOAT4(1.0f, 0.1f, 0.1f, 1.0f);
			pNewLight->falloff = 0.7f;
			pNewLight->attenuation = XMFLOAT3(1.0f, 0.5f, 0.25f);
			pNewLight->phi = (float)numbers::pi * 0.5f;
			pNewLight->theta = (float)numbers::pi * 0.7f;
			pNewLight->range = 6.f;

			if (auto pScene = wpScene.lock()) {
				pScene->AddLight(pNewLight);
				pNewLight->UpdateLight();
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
				// 인스턴싱을 사용하는 불투명한 오브젝트를 그리는 쉐이더
				// 쉐이더에서는 읽는 기준이 달라지므로 전치행렬로 바꾸어준다.
				world = pGameObject->GetWorldTransform();
				XMStoreFloat4x4(&temp, XMMatrixTranspose(XMLoadFloat4x4(&world)));
				instanceDatas[objName].push_back(temp);
			}
			break;
		}
		case SectorLayer::etc: {
			switch (objType) {
			case ObjectType::prisonPosition:
				prisonPosition = position;
				cout << "감옥위치 : " << position << "\n";
				break;
			case ObjectType::prisonExitPosition:
				prisonExitPosition = position;
				break;
			case ObjectType::itemSpawnLocation:
				wpScene.lock()->AddItemSpawnLocation(position);
				break;
			case ObjectType::exitBox: {
				BoundingBox exitBox;
				file.read((char*)&exitBox.Center, sizeof(XMFLOAT3));
				file.read((char*)&exitBox.Extents, sizeof(XMFLOAT3));
				exitBox.Center = Vector3::Add(exitBox.Center, position);
				wpScene.lock()->SetExitBox(exitBox);
				break;
			}
			default:
				break;
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
	// 플레이어가 포함된 섹터 및 인접한 섹터를 가져온다.
	vector<shared_ptr<GameObject>> result;
	vector<Sector*> checkSector = GetAroundSectors(pindex);

	// 현재 프레임에 저장된 플레이어의 회전만큼 바운딩박스를 회전시켜본다.
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
	// 플레이어가 포함된 섹터 및 인접한 섹터를 가져온다.
	vector<Sector*> checkSector = GetAroundSectors(pindex);

	BoundingOrientedBox boundingBox = pPlayer->GetBoundingBox();
	XMFLOAT3 velocity = pPlayer->GetVelocity();
	// 현재 프레임에 저장된 플레이어의 이동벡터만큼 바운딩박스를 이동시켜본다.
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
	// 플레이어가 포함된 섹터 및 인접한 섹터를 가져온다.
	vector<Sector*> checkSector = GetAroundSectors(pindex);

	BoundingOrientedBox boundingBox = pPlayer->GetBoundingBox();
	XMFLOAT3 velocity = pPlayer->GetVelocity();
	boundingBox.Center.y += velocity.y * _timeElapsed;

	shared_ptr<GameObject> obj;
	for (auto& sector : checkSector) {
		obj = sector->CheckCollisionVertical(boundingBox, pPlayer, _timeElapsed);
		if (obj) {
			return obj;
		}
	}
	return nullptr;
}

void Zone::CheckCollisionWithAttack() {
	// 이 함수가 호출되는 곳에서 Student 체크를 함.
	auto pStudent = static_pointer_cast<Student>(pPlayer);
	vector<Sector*> checkSector = GetAroundSectors(pindex);
	if (pStudent) {
		for (auto& sector : checkSector) {
			sector->CheckCollisionWithAttack(pStudent);
		}
	}

}

void Zone::CheckCollisionWithItem() {
	// 이 함수가 호출되는 곳에서 Student 체크를 함.
	auto pStudent = static_pointer_cast<Student>(pPlayer);
	// 이미 플레이어가 아이템을 보유중이라면 스킵한다.
	if (pStudent->GetItem() != ObjectType::none) return;

	vector<Sector*> checkSector = GetAroundSectors(pindex);
	if (pStudent) {
		for (auto& sector : checkSector) {
			sector->CheckCollisionWithItem(pStudent);
		}
	}
}

void Zone::CheckCollisionWithTrap() { 
	// 이 함수가 호출되는 곳에서 Professor 체크를 함.
	auto pProfessor = static_pointer_cast<Professor>(pPlayer);

	vector<Sector*> checkSector = GetAroundSectors(pindex);
	if (pProfessor) {
		for (auto& sector : checkSector) {
			sector->CheckCollisionWithTrap(pProfessor);
		}
	}
}

void Zone::CheckCollisionProjectileWithObstacle() {

	BoundingOrientedBox boundingBox;
	for (auto& [objectID, pAttack] : pAttackObjTable) {
		if (pAttack->GetAttackType() == AttackType::throwAttack) {	// 투사체일 경우
			boundingBox = pAttack->GetBoundingBox();
			// 주변섹터의 장애물들과 충돌체크를 한다.
			vector<Sector*> pSectors = GetAroundSectors(GetIndex(pAttack->GetWorldPosition()));
				auto pThrowAttack = dynamic_pointer_cast<ThrowAttack>(pAttack);

				for (const auto& pSector : pSectors) {
					if (!pThrowAttack->GetIsStuck() && pSector->CheckCollisionProjectileWithObstacle(boundingBox)) {
						pThrowAttack->SetIsStuck(true);
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

void Zone::SetVisiblePlayer(shared_ptr<Camera> _pCamera) {
	// 
	vector<shared_ptr<InterpolateMoveGameObject>> pOtherPlayers;
	vector<Sector*> sectors = GetAroundSectors(GetIndex(_pCamera->GetWorldPosition()));

	bool isVisible;

	for (auto& sector : sectors) {
		auto pTempPlayers = sector->GetObjectsByLayer(SectorLayer::otherPlayer);
		for (auto pTempPlayer : pTempPlayers) {
			pOtherPlayers.push_back(static_pointer_cast<InterpolateMoveGameObject>(pTempPlayer));
		}
	}

	for (auto pOtherPlayer : pOtherPlayers) {

		XMFLOAT3 rayWorldDirection = Vector3::Normalize(Vector3::Subtract(pOtherPlayer->GetWorldPosition(), _pCamera->GetWorldPosition()));

		if (Vector3::Angle(_pCamera->GetWorldLookVector(), rayWorldDirection, false) > 90.0f) {
			pOtherPlayer->SetVisible(false);
			break;
		}

		XMFLOAT3 center = pOtherPlayer->GetBoundingBox().Center;
		isVisible = true;
		for (auto& sector : sectors) {
			// 안보일 경우
			if (!sector->SetVisiblePlayer(_pCamera, center)) {
				isVisible = false;
				break;
			}
		}
		pOtherPlayer->SetVisible(isVisible);

	}
}

void Zone::UpdatePlayerSector() {
	XMINT3 prevIndex = pindex;
	pindex = GetIndex(pPlayer->GetWorldPosition());
}

void Zone::AnimateObjects(float _timeElapsed) {
	XMINT3 prevIndex, nextIndex;
	for (auto [objectID, pGameObject] : pInteractObjTable) {	// 상호작용 오브젝트
		pGameObject->Animate(_timeElapsed);
	}

	for (auto [objectID, pItem] : pItemObjTable) {	// 상호작용 오브젝트
		pItem->Animate(_timeElapsed);
	}

	auto pAttackTableIter = pAttackObjTable.begin();
	while (pAttackTableIter != pAttackObjTable.end()) {
		auto& [objectID, pAttack] = *pAttackTableIter;

		prevIndex = GetIndex(pAttack->GetWorldPosition());
		// 사라져야할 공격은 삭제, 소속 섹터 업데이트
		if (pAttack->GetIsRemove()) {
			GetSector(pAttack->GetWorldPosition())->RemoveObject(SectorLayer::attack, objectID);
			pAttackTableIter = pAttackObjTable.erase(pAttackTableIter);
		}
		else {
			pAttack->Animate(_timeElapsed);
			nextIndex = GetIndex(pAttack->GetWorldPosition());

			if (prevIndex.x != nextIndex.x || prevIndex.y != nextIndex.y || prevIndex.z != nextIndex.z) {
				HandOffObject(SectorLayer::attack, pAttack->GetID(), pAttack, prevIndex, nextIndex);
			}
			pAttackTableIter++;
		}
		
	}

	auto pItemTableIter = pItemObjTable.begin();
	while (pItemTableIter != pItemObjTable.end()) {
		auto& [objectID, pItem] = *pItemTableIter;
		// 사라져야할 아이템은 삭제
		if (pItem->GetIsRemove()) {
			GetSector(pItem->GetWorldPosition())->RemoveObject(SectorLayer::item, objectID);
			pItemTableIter = pItemObjTable.erase(pItemTableIter);
		}
		else pItemTableIter++;
	}


	auto pTrapTableIter = pTrapObjTable.begin();
	while (pTrapTableIter != pTrapObjTable.end()) {
		auto& [objectID, pTrap] = *pTrapTableIter;
		// 사라져야할 아이템은 삭제
		if (pTrap->GetIsRemove()) {
			GetSector(pTrap->GetWorldPosition())->RemoveObject(SectorLayer::trap, objectID);
			pTrapTableIter = pTrapObjTable.erase(pTrapTableIter);
		}
		else pTrapTableIter++;
	}
}

void Zone::SetAllComputerPower(bool _power) {
	for (auto [objectID, pGameObject] : pInteractObjTable) {
		if (pGameObject->GetObjectType() == ObjectType::computer) {
			static_pointer_cast<Computer>(pGameObject)->SetPower(_power);
		}
	}
}


shared_ptr<InteractObject> Zone::UpdateInteractableObject() {
	shared_ptr<InteractObject> nearestObject = nullptr;
	float minDist = FLT_MAX;

	vector<Sector*> checkSector = GetAroundSectors(pindex);
	// 섹터마다 플레이어가 바라보고 있으면서 가장 가까운 오브젝트를 반환
	for (auto& sector : checkSector) {

		auto [dist, pGameObject] = sector->GetNearestInteractObject(pPlayer);
		if (pGameObject) {
			if (minDist > dist) {
				minDist = dist;
				nearestObject = pGameObject;
			}
		}
	}
	// 바라보는 방향의 가장 가까운 오브젝트를 반환한다.
	return nearestObject;
}

shared_ptr<Attack> Zone::GetAttack(UINT _objectID) {
	if (pAttackObjTable[_objectID]) return pAttackObjTable[_objectID];
	return nullptr;
}

shared_ptr<Trap> Zone::GetTrap(UINT _objectID) {
	if (pTrapObjTable[_objectID]) return pTrapObjTable[_objectID];
	return nullptr;
}

void Zone::AddAttack(AttackType _attackType, UINT _objectID, shared_ptr<GameObject> _pPlayerObject, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	auto pProfessor = dynamic_pointer_cast<Professor>(_pPlayerObject);

	shared_ptr<Attack> pAttack;
	if (_attackType == AttackType::swingAttack) {
		pAttack = make_shared<SwingAttack>(_pPlayerObject->GetID());
		pAttack->Create("SwingAttack", _pDevice, _pCommandList);

		// 캐릭터의 중심위치로 이동시킨다.
		if (pProfessor)
			pAttack->SetLocalPosition(pProfessor->GetWorldPosition());
		else
			pAttack->SetLocalPosition(static_pointer_cast<InterpolateMoveGameObject>(_pPlayerObject)->GetWorldPosition());
		pAttack->SetLocalRotation(_pPlayerObject->GetLocalRotate());
		pAttack->MoveRight(-0.10f);
		pAttack->MoveFront(1.2f);
		pAttack->MoveUp(0.30f);
		
		pAttack->Rotate(pAttack->GetLocalLookVector(), -18.f);
		pAttack->Rotate(pAttack->GetLocalRightVector(), -35.f);
	}
	else if (_attackType == AttackType::throwAttack) {

		pAttack = make_shared<ThrowAttack>(_pPlayerObject->GetID(), _pPlayerObject->GetWorldLookVector());
		pAttack->Create("BookAttack", _pDevice, _pCommandList);

		// 손 오브젝트를 얻는다.
		shared_ptr<GameObject> pHandObject;
		if (pProfessor)
			pHandObject = pProfessor->GetHandObject();
		else
			pHandObject = static_pointer_cast<InterpolateMoveGameObject>(_pPlayerObject)->GetHandObject(); 

		pAttack->SetLocalPosition(pHandObject->GetWorldPosition());
		// 캐릭터의 손 위치로 이동
		if (pProfessor)
			pAttack->SetLocalPosition(pProfessor->GetHandObject()->GetWorldPosition());
		else
			pAttack->SetLocalPosition(static_pointer_cast<InterpolateMoveGameObject>(_pPlayerObject)->GetHandObject()->GetWorldPosition());
		pAttack->SetLocalRotation(_pPlayerObject->GetLocalRotate());
	}
	
	pAttack->UpdateObject();
	AddObject(SectorLayer::attack, _objectID, pAttack, pAttack->GetWorldPosition());

	pAttack->SetID(_objectID);

	pAttackObjTable[_objectID] = pAttack;
}

void Zone::AddItem(ObjectType _objectType, UINT _itemSpawnIndex, UINT _objectID, const XMFLOAT3& _position,  const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	shared_ptr<Item> pItem = make_shared<Item>(_objectType);
	if (_objectType == ObjectType::prisonKeyItem) {
		pItem->Create("PrisonKey", _pDevice, _pCommandList);
	}
	if (_objectType == ObjectType::medicalKitItem) {
		pItem->Create("MedicalKit", _pDevice, _pCommandList);
	}
	if (_objectType == ObjectType::energyDrinkItem) {
		pItem->Create("EnergyDrink", _pDevice, _pCommandList);
	}
	if (_objectType == ObjectType::trapItem) {
		pItem->Create("Trap", _pDevice, _pCommandList);
	}
	// 오브젝트의 초기 위치를 설정해준다.
	pItem->SetLocalPosition(_position);
	pItem->UpdateObject();
	pItem->SetID(_objectID);
	pItem->SetIndex(_itemSpawnIndex);
	pItem->SetBasePosition(_position);
	// 해당 섹터에 아이템을 추가하고, 테이블에도 추가한다.
	Sector* pSector = GetSector(GetIndex(_position));
	pSector->AddObject(SectorLayer::item, _objectID, pItem);
	pItemObjTable[_objectID] = pItem;
}

void Zone::AddTrap(UINT _objectID, shared_ptr<Trap> _pTrap) {
	AddObject(SectorLayer::trap, _objectID, _pTrap, _pTrap->GetWorldPosition());
	pTrapObjTable[_objectID] = _pTrap;
}

void Zone::RemoveAttack(UINT _objectID) {
	pAttackObjTable[_objectID]->Remove();
}

void Zone::RemoveItem(UINT _objectID) {
	pItemObjTable[_objectID]->Remove();
}

void Zone::RemoveTrap(UINT _objectID) {
	pTrapObjTable[_objectID]->Remove();
}

void Zone::Interact(UINT _objectID) {
	shared_ptr<InteractObject> pGameObject = pInteractObjTable[_objectID];
	if (pGameObject)
		pGameObject->Interact();
	else
		cout << "InteractObject Null Error. objID : " << _objectID << "\n";
}

