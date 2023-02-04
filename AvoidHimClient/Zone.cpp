#include "stdafx.h"
#include "Zone.h"
#include "GameFramework.h"
#include "GameObject.h"


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
void Sector::RemoveObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pGameObject) {
	if (_pGameObject == NULL) {
		cout << format("버그 : 오브젝트 포인터가 NULL입니다.\n");
		return;
	}

	Layer& pGameObjectLayer = pGameObjectLayers[(int)_sectorLayer];
	auto it = pGameObjectLayer.find(_objectID);

	if (it != pGameObjectLayer.end()) {		// 객체가 존재할 경우
		pGameObjectLayer.erase(it);
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

vector<shared_ptr<GameObject>>  Sector::CheckCollisionRotate(BoundingOrientedBox& _boundingBox, shared_ptr<GameObject> _pFloor) {
	
	for (auto [gid, pGameObject] : pGameObjectLayers[(UINT)SectorLayer::attack]) {
		if (pGameObject->GetBoundingBox().Intersects(_boundingBox)) {
			// 충돌한 pGameObject에 대한 처리 및 플레이어의 체력을 깎는다.
		}
	}
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
	
	// 점프없이 올라갈 수 있는 최대 높이값
	float bias = 0.4f;

	for (auto [gid, pGameObject] : pGameObjectLayers[(UINT)SectorLayer::obstacle]) {
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
	for (auto [gid, pGameObject] : pGameObjectLayers[(UINT)SectorLayer::obstacle]) {
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


///////////////////////////////////////////////////////////////////////////////
/// Zone

// 생성자, 소멸자
Zone::Zone() {
	startPoint = XMFLOAT3(0, 0, 0);
	size = XMFLOAT3(0, 0, 0);
	div = XMINT3(0, 0, 0);
	sectorSize = XMFLOAT3(0, 0, 0);
}
Zone::Zone(const XMFLOAT3& _size, const XMINT3& _div, shared_ptr<PlayScene> _pScene) : size(_size), div(_div), pScene(_pScene) {
	sectors.assign(div.x, vector<vector<Sector>>(div.y, vector<Sector>(div.z, Sector())));
	sectorSize = Vector3::Division(size, div);
	startPoint = XMFLOAT3(-50, -50, -50);
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
// 오브젝트 제거
void Zone::RemoveObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMFLOAT3& _pos) {
	Sector* sector = GetSector(_pos);
	sector->RemoveObject(_sectorLayer, _objectID, _pObject);
}

void Zone::RemoveObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMINT3& _index) {
	Sector* sector = GetSector(_index);
	sector->RemoveObject(_sectorLayer, _objectID, _pObject);
}

// 오브젝트를 다른 섹터로 이동
void Zone::HandOffObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMFLOAT3& _prePos, const XMFLOAT3& _nextPos) {
	AddObject(_sectorLayer, _objectID, _pObject, _nextPos);		// 새로추가하고
	RemoveObject(_sectorLayer, _objectID, _pObject, _prePos);	// 이전위치에서는 제거
}
void Zone::HandOffObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMINT3& _preIndex, const XMINT3& _nextIndex) {
	AddObject(_sectorLayer, _objectID, _pObject, _nextIndex);		// 새로추가하고
	RemoveObject(_sectorLayer, _objectID, _pObject, _preIndex);	// 이전위치에서는 제거
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

	GameObject::RenderInstanceObjects(_pCommandList);

	gameFramework.GetShader("BasicShader")->PrepareRender(_pCommandList);
	//gameFramework.GetShader("SkinnedShader")->PrepareRender(_pCommandList);
	pPlayer->Render(_pCommandList);
#else
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

#endif
#endif


}



void Zone::LoadZoneFromFile(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
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
	SectorLayer objType;

	XMFLOAT4X4 world, temp;

	XMFLOAT3 position, scale;
	XMFLOAT4 rotation;

	// nInstance (UINT)
	file.read((char*)&nInstance, sizeof(UINT));

	for (UINT objectID = 1; objectID <= nInstance; ++objectID) {
		// nameSize(UINT) / fileName (string)
		ReadStringBinary(objName, file);

		// objectType(char)
		file.read((char*)&objType, sizeof(SectorLayer));

		// position(float * 3) / scale(float * 3) / rotation(float * 3)
		file.read((char*)&position, sizeof(XMFLOAT3));
		file.read((char*)&scale, sizeof(XMFLOAT3));
		file.read((char*)&rotation, sizeof(XMFLOAT4));

		switch (objType) {
		case SectorLayer::player: {
			pPlayer = make_shared<Player>();

			pPlayer->Create(objName, _pDevice, _pCommandList);
			pPlayer->SetLocalPosition(position);
			pPlayer->SetLocalScale(scale);
			pPlayer->SetLocalRotation(rotation);
			pPlayer->UpdateObject();
			pScene->SetPlayer(pPlayer);
                                                                                                                                                                                                                                                                                                                                                   
			pindex = GetIndex(position);
			pid = objectID;
			cout << "pid = " << pid << "\n";
			AddObject(objType, pid, pPlayer, pindex);
			break;
		}
		case SectorLayer::obstacle: {
			shared_ptr<GameObject> pGameObject = make_shared<GameObject>();
			pGameObject->Create(objName, _pDevice, _pCommandList);
			pGameObject->SetLocalPosition(position);
			pGameObject->SetLocalScale(scale);
			pGameObject->SetLocalRotation(rotation);
			pGameObject->UpdateObject();

			AddObject(objType, objectID, pGameObject, GetIndex(position));
			// 쉐이더에서는 읽는 기준이 달라지므로 전치행렬로 바꾸어준다. 
			world = pGameObject->GetWorldTransform();
			XMStoreFloat4x4(&temp, XMMatrixTranspose(XMLoadFloat4x4(&world)));
			instanceDatas[objName].push_back(temp);
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
		obj = sector->CheckCollisionVertical(boundingBox, pPlayer);
		if (obj) {
			return obj;
		}
	}
	return nullptr;
}

void Zone::UpdatePlayerSector() {
	XMINT3 prevIndex = pindex;
	pindex = GetIndex(pPlayer->GetWorldPosition());

	// 이전과 현재 플레이어 섹터 인덱스가 다를경우
	if (!(prevIndex.x == pindex.x && prevIndex.y == pindex.y && prevIndex.z == pindex.z)) {
		cout << "발동!\n";
		HandOffObject(SectorLayer::player, pid, pPlayer, prevIndex, pindex);
	}
}


