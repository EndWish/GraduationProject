#include "stdafx.h"
#include "Light.h"
#include "GameObject.h"


Light::Light(const shared_ptr<GameObject>& _object) {
	// 각 항들. 임시로 설정한것 이므로 따로 내일 Light 멤버를 set하는 함수를 만들어라
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

	// 1 = 점, 2 = 스포트, 3 = 직접
	lightType = 3;

	// 이 빛이 켜져있는 상태인지 확인
	enable = true;
}

Light::~Light() {

}


