#include <iostream>
#include <fstream>
#include <random>
#include <ranges>
#include <format>
#include <memory.h>
#include <memory>

#include <vector>
#include <chrono>

using namespace std;

class Scene {

public:
	Scene() { cout << "Scene ����\n"; };
	~Scene() { cout << "Scene �Ҹ�\n"; };
	//virtual ~Scene() { cout << "Scene �Ҹ�\n"; };

	virtual void Animate() { cout << "Scene Animate\n"; };
	void Animate2() { cout << "Scene Animate2\n"; };
};

class ShopScene : public Scene {

public:
	ShopScene() { cout << "ShopScene ����\n"; };
	~ShopScene() { cout << "ShopScene �Ҹ�\n"; };

	void Animate() final { cout << "ShopScene Animate\n"; };
	void Animate2() { cout << "ShopScene Animate2\n"; };
};

int main() 
{
	shared_ptr<Scene> test;
}