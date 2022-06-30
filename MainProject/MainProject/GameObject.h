#pragma once
class GameObject : public enable_shared_from_this<GameObject> {
protected:	// ��� ������
	string m_name;	// ��ü�� �̸�

	XMFLOAT4X4 m_worldTransform;	// ��->���� ��ǥ�� �ٲٱ� ���� ���
	XMFLOAT4X4 m_eachTransform;		// �θ� ��ǥ �������� �ڽ��� ��ȯ�ϴ� ���

	BoundingOrientedBox	m_OOBB;	// Object-Oriented Bounding Box
	bool m_isOOBBCover;	// true�� ��� m_OOBB�� �ڽĵ��� OOBB���� ��ǥ�� �浹�ϴ��� üũ�Ѵ�.(== �浹���� �ʴ´ٸ� ���� ��ü�� OOBB�鵵 ���� �浹x�� �����Ѵ�.)

	// �Ž� ������
	// ���׸��� �����͵� �ʱ�ȭ

	// https://modoocode.com/252 : ����Ʈ �����͸� �Ϻ��� �����ϰ� ���� �ʴٸ� �ݵ�� ����.
	weak_ptr<GameObject> m_pParent;	// �θ� ����Ű�� ������
	vector<shared_ptr<GameObject>> m_pChildren;	// �ڽĵ��� ����Ű�� �����͵� (�ڽ��� root�� ��� Scene���� �����ͷ� ������ ������Ų��.)

public:		// �������� ��� �Լ���
	// ������, �Ҹ���, ���������, �̵�������, �����Ҵ�, �̵��Ҵ�
	GameObject();
	virtual ~GameObject();
	GameObject(const GameObject& other);	//��������� (*�ڽ� ������Ʈ�� �Բ� ���������Ѵ�. *�θ� ������ Scene�� ���� ��������� �Ѵ�.)
	GameObject(GameObject&& other) noexcept;	//�̵������� (�� �Լ��� ȣ���� �� other�� ��� �ϸ� �ȵȴ�. other�� �Ҹ�Ǳ� ������, ���� other���� ����Ǿ� �ִ� �����͵� ��� ����� ������ ���� ������ ���ǰ� �ʿ��ϴ�.)
	virtual GameObject& operator=(const GameObject& other);	//�����Ҵ� (*�ڽ� ������Ʈ�� �Բ� ���������Ѵ�. *�θ� ������ Scene�� ���� ��������� �Ѵ�.)
	virtual GameObject& operator=(GameObject&& other) noexcept;	//�̵��Ҵ�

public:		// ��� �Լ���
	void SetChild(const shared_ptr<GameObject>& pChild);

};

