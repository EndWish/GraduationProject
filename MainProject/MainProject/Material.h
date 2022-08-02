#pragma once

class Material {
public:		// 정적 함수▼
	static shared_ptr<Material> LoadFromFile(const string& fileName);

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

class MaterialStore {
private:
	unordered_map<string, pair<int, shared_ptr<Material>> > store;

public:


};