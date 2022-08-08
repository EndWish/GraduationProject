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
	Scene() { cout << "Scene 积己\n"; };
	~Scene() { cout << "Scene 家戈\n"; };
	//virtual ~Scene() { cout << "Scene 家戈\n"; };

	virtual void Animate() { cout << "Scene Animate\n"; };
	void Animate2() { cout << "Scene Animate2\n"; };
};

class ShopScene : public Scene {

public:
	ShopScene() { cout << "ShopScene 积己\n"; };
	~ShopScene() { cout << "ShopScene 家戈\n"; };

	void Animate() final { cout << "ShopScene Animate\n"; };
	void Animate2() { cout << "ShopScene Animate2\n"; };
};

int main() 
{
	shared_ptr<Scene> test;
}