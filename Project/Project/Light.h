#pragma once

class GameObject;

class Light
{
private:
	// �� �׵�
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;

	XMFLOAT3 position;
	float range;

	XMFLOAT3 offset;	// ���� ���� ��ü�� �߽����κ��� ������ ��
	float theta; //cos(m_fTheta), ����Ʈ����Ʈ���� ���
	XMFLOAT3 attenuation;
	float phi; //cos(m_fPhi), ����Ʈ����Ʈ���� ���
	XMFLOAT3 direction;
	float padding;

	// �� ���� ���� �ִ� ������Ʈ�� ������
	weak_ptr<GameObject> object;

	// 1 = ��, 2 = ����Ʈ, 3 = ����
	int lightType;

	// �� ���� �����ִ� �������� Ȯ��
	bool enable;
	XMFLOAT2 padding2;
	// �е� ��Ģ �ؼ�
public:
	Light(const shared_ptr<GameObject>& _object);
	~Light();


};

struct LightsMappedFormat {
	array<Light, MAX_LIGHTS> lights;
	XMFLOAT4 globalAmbient;
	int nLight;
};