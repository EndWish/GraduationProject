#pragma once

class Material {
protected:	// ��� ������
	XMFLOAT4 m_ambient;
	XMFLOAT4 m_diffuse;
	XMFLOAT4 m_specular; //(r,g,b,a=power)
	XMFLOAT4 m_emissive;

public:		// �������� ��� �Լ���
	// ������ �� �Ҹ���
	Material();
	virtual ~Material();

public:		// ��� �Լ���

};

