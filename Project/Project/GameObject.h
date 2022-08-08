#pragma once
class GameObject {
	string name;
	
	// 월드좌표계 기준
	XMFLOAT4X4 worldTransform;

	// 부모좌표계 기준
	XMFLOAT4X4 eachTransform;

	BoundingOrientedBox boundingBox;
	// true일경우 하위 오브젝트들을 모두 포함하는 바운딩박스 객체임
	bool isOOBBBCover;


	weak_ptr<GameObject> pParent;
	vector<shared_ptr<GameObject>> pChildren;
};

