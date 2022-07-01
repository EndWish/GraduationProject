#pragma once

class Material {
protected:	// 멤버 변수▼
	XMFLOAT4 m_ambient;
	XMFLOAT4 m_diffuse;
	XMFLOAT4 m_specular; //(r,g,b,a=power)
	XMFLOAT4 m_emissive;

public:		// 생성관련 멤버 함수▼
	// 생성자 및 소멸자
	Material();
	virtual ~Material();

public:		// 멤버 함수▼

};

