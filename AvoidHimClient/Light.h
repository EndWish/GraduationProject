#pragma once

class GameObject;
class Texture;
class Light
{
public:
	// �� ��ġ�� ���⿡���� ��ȯ ��� (��ġ ����� �Ǿ� ����!!)
	XMFLOAT4X4 viewTransform;
	XMFLOAT4X4 projectionTransform;
	// �� �׵�
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;

	XMFLOAT3 position;
	float range;

	XMFLOAT3 offset;	// ���� ���� ��ü�� �߽����κ��� ������ ��
	float theta; // �ܺ� ���� �׸��� ��, ����Ʈ����Ʈ���� ���
	XMFLOAT3 attenuation;
	float phi; // ���� ���� �׸��� ��, ����Ʈ����Ʈ���� ���
	XMFLOAT3 direction;
	float falloff;	// phi�� theta�� ���� ���� ����
private:
	weak_ptr<GameObject> object;
public:
	// 1 = ��, 2 = ����Ʈ, 3 = ����
	int lightType;

	// �� ���� �����ִ� �������� Ȯ��
	bool enable;

	D3D12_CPU_DESCRIPTOR_HANDLE bakedShadowMapCPUDescriptorHandle;
	shared_ptr<Texture> pBakedShadowMap;
	
public:
	Light(const shared_ptr<GameObject>& _object = nullptr);
	~Light();
	void UpdateLight();

	void SetBakedShadowMap(shared_ptr<Texture> _pTexture, D3D12_CPU_DESCRIPTOR_HANDLE _handle);

	void UpdateComputeShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	void UpdateViewTransform();
	void UpdateProjectionTransform(float _nearDistance, float _farDistance, float _aspectRatio, float _fovAngle);
};


struct LightsMappedFormat {
	array<Light, MAX_LIGHTS> lights;
	XMFLOAT4 globalAmbient;
	int nLight;
};