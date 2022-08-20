#include "stdafx.h"
#include "Light.h"
#include "GameObject.h"


Light::Light(const shared_ptr<GameObject>& _object) {
	ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// object�� ���� �� position�� ����. object�� ��ġ+offset���� position���� �ѱ�
	position = XMFLOAT3(0.5f, 0.5f, -2.0f);
	range = 15;

	offset = XMFLOAT3(0.0f, 0.0f, 0.0f);	
	theta = 0;
	phi = 0;
	attenuation = XMFLOAT3(1.0f, 0.05f, 0.001f);	
	direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	falloff = 0;

	object = _object;

	// 1 = ��, 2 = ����Ʈ, 3 = ����
	lightType = 1;

	// �� ���� �����ִ� �������� Ȯ��
	enable = true;
}

Light::~Light() {

}

