#pragma once

class Material {
public:		// ���� �Լ���
	static shared_ptr<Material> LoadFromFile(const string& fileName);

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

class MaterialStore {
private:
	unordered_map<string, pair<int, shared_ptr<Material>> > store;

public:


};