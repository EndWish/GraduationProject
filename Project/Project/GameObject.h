#pragma once
class GameObject {
	string name;
	
	// ������ǥ�� ����
	XMFLOAT4X4 worldTransform;

	// �θ���ǥ�� ����
	XMFLOAT4X4 eachTransform;

	BoundingOrientedBox boundingBox;
	// true�ϰ�� ���� ������Ʈ���� ��� �����ϴ� �ٿ���ڽ� ��ü��
	bool isOOBBBCover;


	weak_ptr<GameObject> pParent;
	vector<shared_ptr<GameObject>> pChildren;
};

