#pragma once
#include "Mesh.h"

class GameObject : public enable_shared_from_this<GameObject> {
protected:
	string name;
	
	// 월드좌표계 기준 : eachTransform 이 바뀌면 항상 동기화 해준다.
	XMFLOAT4X4 worldTransform;

	// 부모좌표계 기준
	XMFLOAT4X4 eachTransform;

	BoundingOrientedBox boundingBox;
	// true일경우 하위 오브젝트들을 모두 포함하는 바운딩박스 객체임
	bool isOOBBBCover;

	weak_ptr<GameObject> pParent;
	vector<shared_ptr<GameObject>> pChildren;

	weak_ptr<Mesh> pMesh;

public:
	GameObject();
	virtual ~GameObject();

// get set 함수
	
	// 오른쪽 방향의 단위벡터를 얻는다.
	XMFLOAT3 GetEachRightVector() const;
	// 위쪽 방향의 단위벡터를 얻는다.
	XMFLOAT3 GetEachUpVector() const;
	// 바라보고 있는 방향의 단위벡터를 얻는다.
	XMFLOAT3 GetEachLookVector() const;
	// 부모좌표계 기준 자신의 위치를 리턴한다.
	XMFLOAT3 GetEachPosition() const;
	// 월드좌표계 기준 자신의 위치를 리턴한다.
	XMFLOAT3 GetWorldPosition() const;
	XMFLOAT3 GetWorldRightVector() const;
	XMFLOAT3 GetWorldUpVector() const;
	XMFLOAT3 GetWorldLookVector() const;

	// 앞으로 이동하는 행령을 얻는다.
	XMFLOAT4X4 GetFrontMoveMatrix(float _distance) const;
	// 옆으로 이동하는 행령을 얻는다.
	XMFLOAT4X4 GetRightMoveMatrix(float _distance) const;
	// Z축을 기준으로 회전하는 행렬을 얻는다.

	// 자신의 바운딩 박스의 래퍼런스를 리턴한다.
	const BoundingOrientedBox& GetBoundingBox() const;

	// 위치를 강제로 이동시킨다.
	void SetEachPosition(XMFLOAT3& _position);
	// 자식을 추가한다.
	void SetChild(const shared_ptr<GameObject> _pChild);

	// eachTransform를 가지고 worldTransform를 업데이트 한다.
	virtual void UpdateWorldTransform();
	// 변환행렬을 적용하고 worldTransform을 업데이트 한다.
	void ApplyTransform(XMFLOAT4X4& _transform);

	// 애니메이션
	virtual void Animate(double _timeElapsed);
	// 렌더
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

};

