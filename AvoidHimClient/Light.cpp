#include "stdafx.h"
#include "Light.h"
#include "GameObject.h"


Light::Light(const shared_ptr<GameObject>& _object) {
	ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// object�� ���� �� position�� ����. object�� ��ġ+offset���� position���� �ѱ�
	position = XMFLOAT3(0.0f, 0.5f, -0.0f);
	range = 15;

	offset = XMFLOAT3(0.0f, 0.0f, 0.0f);
	theta = 0;
	phi = 0;
	attenuation = XMFLOAT3(2.0f, 0.05f, 0.01f);
	direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	falloff = 0;

	if (_object) object = _object;

	// 1 = ��, 2 = ����Ʈ, 3 = ����
	lightType = 2;

	// �� ���� �����ִ� �������� Ȯ��
	enable = true;


	//UpdateProjectionTransform(0.1f, 10000.0f, aspectRatio, 65.0f);
	
}

Light::~Light() {

}

void Light::UpdateLight() {
	if (object.lock())
	{
		position = Vector3::Add(object.lock()->GetWorldPosition(), offset);
		direction = object.lock()->GetWorldLookVector();
	}
	if (lightType == 2) {
		float aspectRatio = float(C_WIDTH) / C_HEIGHT;
		float degree = XMConvertToDegrees(theta);
		UpdateViewTransform();
		UpdateProjectionTransform(0.01f, 10.0f, aspectRatio, degree);
	}
}


void Light::UpdateViewTransform() {
	// ������ ��ġ�� ���⿡ ���� ����
	XMFLOAT3 worldPosition = position;

	XMFLOAT3 lookAtWorld = Vector3::Add(worldPosition, Vector3::Normalize(direction));

	// look�� +z, up�� +y������ �������� look�� -y�� �ɰ�� up�� +z�� �ȴ�.
	XMFLOAT4X4 view = Matrix4x4::LookAtLH(worldPosition, lookAtWorld, XMFLOAT3(0,0,1));
	XMStoreFloat4x4(&viewTransform, XMMatrixTranspose(XMLoadFloat4x4(&view)));	
}

void Light::UpdateProjectionTransform(float _nearDistance, float _farDistance, float _aspectRatio, float _fovAngle) {
	XMFLOAT4X4 projection = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(_fovAngle), _aspectRatio, _nearDistance, _farDistance);
	XMStoreFloat4x4(&projectionTransform, XMMatrixTranspose(XMLoadFloat4x4(&projection)));	// ���̴��� ��?�켱 ����̱� ������ ��ġ��ķ� �ٲپ �����ش�.
}
