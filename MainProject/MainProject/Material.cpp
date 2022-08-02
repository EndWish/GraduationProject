#include "stdafx.h"
#include "Material.h"

// ���� �Լ���
shared_ptr<Material> Material::LoadFromFile(const string& fileName)
{
	ifstream in("Material/" + fileName, ios::binary);
	if (in) {
		
		// � Ŭ������ �������� Ȯ���ϰ� �׿� �´� Ŭ������ ������ ������ �����´�.

	}

#ifdef DEBUG
	else {
		cout << "Material::LoadFromFile() : ������ ã�� ���߽��ϴ�.\n";
	}
#endif // DEBUG

}

/// ��� ������
// ������ �� �Ҹ���
Material::Material() 
{
	m_ambient = XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f);
	m_diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f); //(r,g,b,a=power)
	m_emissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
}
Material::~Material() 
{

}
