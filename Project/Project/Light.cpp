#include "stdafx.h"
#include "Light.h"
#include "GameObject.h"


Light::Light(const shared_ptr<GameObject>& _object) {
	// �� �׵�. �ӽ÷� �����Ѱ� �̹Ƿ� ���� ���� Light ����� set�ϴ� �Լ��� ������
	ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	position = XMFLOAT3(0.5f, 0.5f, -10.0f);
	range = 0;

	offset = XMFLOAT3(0.0f, 0.0f, 0.0f);	
	theta = 0;
	phi = 0;
	attenuation = XMFLOAT3(0.0f, 0.0f, 0.0f);	
	direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	padding = 0;

	object = _object;

	// 1 = ��, 2 = ����Ʈ, 3 = ����
	lightType = 3;

	// �� ���� �����ִ� �������� Ȯ��
	enable = true;
}

Light::~Light() {

}


