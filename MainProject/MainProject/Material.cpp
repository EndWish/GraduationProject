#include "stdafx.h"
#include "Material.h"

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
