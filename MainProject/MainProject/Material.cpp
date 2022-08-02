#include "stdafx.h"
#include "Material.h"

// 정적 함수▼
shared_ptr<Material> Material::LoadFromFile(const string& fileName)
{
	ifstream in("Material/" + fileName, ios::binary);
	if (in) {
		
		// 어떤 클래스의 재질인지 확인하고 그에 맞는 클래스를 생성후 정보를 가져온다.

	}

#ifdef DEBUG
	else {
		cout << "Material::LoadFromFile() : 파일을 찾지 못했습니다.\n";
	}
#endif // DEBUG

}

/// 멤버 변수▼
// 생성자 및 소멸자
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
