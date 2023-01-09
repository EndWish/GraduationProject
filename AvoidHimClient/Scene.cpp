#include "stdafx.h"
#include "Scene.h"
#include "Timer.h"
#include "GameFramework.h"

Scene::Scene() {
	globalAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
}

Scene::~Scene() {
	pLightsBuffer->Unmap(0, NULL);
}

void Scene::CheckCollision() {

}


void Scene::Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();
	// �������� ����
	LoadStage(_pDevice, _pCommandList);
	camera = make_shared<Camera>();
	camera->Create(_pDevice, _pCommandList);

	//camera->SetLocalPosition(XMFLOAT3(0.0, 0.0, 0.0));
	camera->SetLocalPosition(XMFLOAT3(0.0, 10.0, -20));

	camera->SetLocalRotation(Vector4::QuaternionRotation(XMFLOAT3(0, 1, 0), 0.0f));

	camera->SetPlayerPos(pPlayer->GetWorldPosition());

	camera->UpdateLocalTransform();
	camera->UpdateWorldTransform();

	pPlayer->SetChild(camera);
	pPlayer->UpdateObject();

	ComPtr<ID3D12Resource> temp;
	UINT ncbElementBytes = ((sizeof(LightsMappedFormat) + 255) & ~255); //256�� ���
	pLightsBuffer = ::CreateBufferResource(_pDevice, _pCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, temp);
	
	pLightsBuffer->Map(0, NULL, (void**)&pMappedLights);

}

void Scene::ReleaseUploadBuffers() {
	GameFramework& gameFramework = GameFramework::Instance();
	gameFramework.GetMeshManager().ReleaseUploadBuffers();
	gameFramework.GetTextureManager().ReleaseUploadBuffers();
}


void Scene::ProcessKeyboardInput(const array<UCHAR, 256>& _keysBuffers, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {


	GameFramework& gameFramework = GameFramework::Instance();

	// ȸ���� �����ϸ��� ���ʿ� move�� ���ʿ� ���Ѵ�.
	TEST_PACKET t;
	t.cid = 0;
	if (_keysBuffers['A'] & 0xF0) {
		t.letter = 'a';
		send(server_sock, (char*)&t, sizeof(t), 0);
		cout << "���´�\n";
	}

}

void Scene::AnimateObjects(double _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	pPlayer->Animate(_timeElapsed);
	camera->SetPlayerPos(pPlayer->GetWorldPosition());

	for (auto& pLight : pLights) {
		if (pLight) {
			pLight->UpdateLight();
		}
	}
}

void Scene::ProcessSocketMessage()
{
	char* buf = new char[256];
	recv(server_sock, buf, 1, 0);
	if (buf[0] == 0) {
		recv(server_sock, buf + 1, sizeof(TEST_PACKET) - 1, 0);
		TEST_PACKET* t = reinterpret_cast<TEST_PACKET*>(buf);
		cout << t->cid << " : ���� - " << t->letter << "\n";
	}

	else cout << "������ ��Ŷ\n";
}

void Scene::UpdateLightShaderVariables(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	int nLight = (UINT)pLights.size();
	for (int i = 0; i < nLight; ++i) {

		memcpy(&pMappedLights->lights[i], pLights[i].get(), sizeof(Light));
	}

	memcpy(&pMappedLights->globalAmbient, &globalAmbient, sizeof(XMFLOAT4));
	memcpy(&pMappedLights->nLight, &nLight, sizeof(int));

	D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = pLightsBuffer->GetGPUVirtualAddress();
	_pCommandList->SetGraphicsRootConstantBufferView(2, gpuVirtualAddress);

}


void Scene::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed) {

	GameFramework& gameFramework = GameFramework::Instance();

	float timeElapsed = _timeElapsed;

	// �����ӿ�ũ���� ������ ���� ��Ʈ�ñ״�ó�� set
	camera->SetViewPortAndScissorRect(_pCommandList);
	camera->UpdateShaderVariable(_pCommandList);

	UpdateLightShaderVariables(_pCommandList);

	gameFramework.GetShader("BasicShader")->PrepareRender(_pCommandList);
	pPlayer->Render(_pCommandList);
}

void Scene::AddLight(const shared_ptr<Light>& _pLight) {
	pLights.push_back(_pLight);
}

void Scene::LoadStage(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	// ���� �׷��� ������Ʈ���� ���� ����.

	GameFramework& gameFramework = GameFramework::Instance();
	gameFramework.GetGameObjectManager().GetGameObject("Gunship", _pDevice, _pCommandList);

	pPlayer = make_shared<Player>();
	pPlayer->Create("Gunship", _pDevice, _pCommandList);
	pPlayer->SetLocalScale(XMFLOAT3(2.0f, 2.0f, 2.0f));
	//pPlayer->SetLocalScale(XMFLOAT3(20.0f, 20.0f, 20.0f));
	pPlayer->UpdateObject();

	shared_ptr<Light> baseLight = make_shared<Light>();
	baseLight->lightType = 3;
	baseLight->position = XMFLOAT3(0, 500, 0);
	baseLight->direction = XMFLOAT3(0, -1, 0);
	baseLight->diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.f);
	AddLight(baseLight);

}