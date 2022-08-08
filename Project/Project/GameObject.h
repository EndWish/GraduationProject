#pragma once
class GameObject : public enable_shared_from_this<GameObject> {
protected:
	string name;
	
	// ������ǥ�� ���� : eachTransform �� �ٲ�� �׻� ����ȭ ���ش�.
	XMFLOAT4X4 worldTransform;

	// �θ���ǥ�� ����
	XMFLOAT4X4 eachTransform;

	BoundingOrientedBox boundingBox;
	// true�ϰ�� ���� ������Ʈ���� ��� �����ϴ� �ٿ���ڽ� ��ü��
	bool isOOBBBCover;

	weak_ptr<GameObject> pParent;
	vector<shared_ptr<GameObject>> pChildren;

public:
	GameObject();
	virtual ~GameObject();

// get set �Լ�
	
	// ������ ������ �������͸� ��´�.
	XMFLOAT3 GetRightUnitVector() const;
	// ���� ������ �������͸� ��´�.
	XMFLOAT3 GetUpUnitVector() const;
	// �ٶ󺸰� �ִ� ������ �������͸� ��´�.
	XMFLOAT3 GetLookUnitVector() const;
	// �θ���ǥ�� ���� �ڽ��� ��ġ�� �����Ѵ�.
	XMFLOAT3 GetEachPosition() const;
	// �ڽ��� �ٿ�� �ڽ��� ���۷����� �����Ѵ�.
	const BoundingOrientedBox& GetBoundingBox() const;

	// �ڽ��� �߰��Ѵ�.
	void SetChild(const shared_ptr<GameObject> pChild);
	// eachTransform�� ��ġ���� �ٲ۴�.
	void SetEachPosition(const XMFLOAT3& _position);

	// eachTransform�� ������ worldTransform�� ������Ʈ �Ѵ�.
	void UpdateWorldTransform();	
	// ������ �̵� (_distance�� ������ ��� �ڷ� �̵�)
	void MoveFront(float _distance);
	// ���������� �̵�(_distance�� ������ ��� �������� �̵�)
	void MoveRight(float _distance);
	// Y���� �������� ȸ��
	
	virtual void Animate(double _timeElapsed);

};

