#include "stdafx.h"
#include "Zone.h"
#include "GameObject.h"


///////////////////////////////////////////////////////////////////////////////
/// Sector
Sector::Sector() {
	pGameObjectLayers.assign((UINT)SectorLayer::num, {});
}
Sector::~Sector() {

}

void Sector::AddObject(SectorLayer _sectorLayer, UINT _objectID, GameObject* _pGameObject) {
	Layer pGameObjectLayer = pGameObjectLayers[(int)_sectorLayer];
	auto it = pGameObjectLayer.find(_objectID);

	if (it == pGameObjectLayer.end()) {		// ��ü�� �������� ���� ���
		pGameObjectLayer[_objectID] = _pGameObject;	// �߰��Ѵ�.
	}
	else {
		cout << format("���� : �ߺ��Ǵ� id���� �����մϴ� : {} vs {} \n", pGameObjectLayer[_objectID]->GetName(), _pGameObject->GetName());
	}
}
void Sector::RemoveObject(SectorLayer _sectorLayer, UINT _objectID, GameObject* _pGameObject) {
	if (_pGameObject == NULL) {
		cout << format("���� : ������Ʈ �����Ͱ� NULL�Դϴ�.\n");
		return;
	}

	Layer pGameObjectLayer = pGameObjectLayers[(int)_sectorLayer];
	auto it = pGameObjectLayer.find(_objectID);

	if (it != pGameObjectLayer.end()) {		// ��ü�� ������ ���
		pGameObjectLayer.erase(it);
	}
	else {
		cout << format("���� : �ش� ID�� ������ ������Ʈ�� �����ϴ�.\n");
	}
}

GameObject* Sector::FindObject(SectorLayer _sectorLayer, UINT _objectID) {
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


///////////////////////////////////////////////////////////////////////////////
/// Zone

// ������, �Ҹ���
Zone::Zone() {

}
Zone::Zone(const XMFLOAT3& _size, const XMINT3& _div) : size(_size), div(_div) {
	sectors.assign(div.x, vector<vector<Sector>>(div.y, vector<Sector>(div.z, Sector())));
	sectorSize = Vector3::Division(size, div);
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
void Zone::AddObject(SectorLayer _sectorLayer, UINT _objectID, GameObject* _pObject, const XMFLOAT3& _pos) {
	Sector* sector = GetSector(_pos);
	sector->AddObject(_sectorLayer, _objectID, _pObject);
}
void Zone::AddObject(SectorLayer _sectorLayer, UINT _objectID, GameObject* _pObject, const XMINT3& _index) {
	Sector* sector = GetSector(_index);
	sector->AddObject(_sectorLayer, _objectID, _pObject);
}
// ������Ʈ ����
void Zone::RemoveObject(SectorLayer _sectorLayer, UINT _objectID, GameObject* _pObject, const XMFLOAT3& _pos) {
	Sector* sector = GetSector(_pos);
	sector->AddObject(_sectorLayer, _objectID, _pObject);
}
void Zone::RemoveObject(SectorLayer _sectorLayer, UINT _objectID, GameObject* _pObject, const XMINT3& _index) {
	Sector* sector = GetSector(_index);
	sector->AddObject(_sectorLayer, _objectID, _pObject);
}
// ������Ʈ�� �ٸ� ���ͷ� �̵�
void Zone::HandOffObject(SectorLayer _sectorLayer, UINT _objectID, GameObject* _pObject, const XMFLOAT3& _prePos, const XMFLOAT3& _nextPos) {
	AddObject(_sectorLayer, _objectID, _pObject, _nextPos);		// �����߰��ϰ�
	RemoveObject(_sectorLayer, _objectID, _pObject, _prePos);	// ������ġ������ ����
}
void Zone::HandOffObject(SectorLayer _sectorLayer, UINT _objectID, GameObject* _pObject, const XMINT3& _preIndex, const XMINT3& _nextIndex) {
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
	vector<Sector*> sectors;
	for (int x = 0; x < div.x; ++x) {
		for (int y = 0; y < div.y; ++y) {
			for (int z = 0; z < div.z; ++z) {
				XMINT3 index = XMINT3(x, y, z);
				XMFLOAT3 extents = Vector3::ScalarProduct(sectorSize, 0.5f);
				//center = startPoint + index * sectorSize - extend;
				XMFLOAT3 center = Vector3::Subtract(Vector3::Add(startPoint, Vector3::Multiple(sectorSize, index)), extents);

				BoundingBox boundingBox(center, extents);
				if (_frustum.Intersects(boundingBox)) {
					sectors.push_back(GetSector(index));
				}
			}
		}
	}
	return sectors;
}

