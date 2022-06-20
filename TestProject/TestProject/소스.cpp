#include <iostream>
#include <fstream>
#include <random>
#include <ranges>
#include <format>

#include <vector>
#include <chrono>

using namespace std;

class Animal {
public:
	virtual void Print() {
		cout << "나는 동물이야" << endl;
	}
};

class Dog : public Animal {
public:
	virtual void Print() {
		Animal::Print();
		cout << "나는 개야" << endl;
	}
};

class Retriever : public Dog {
public:
	virtual void Print() {
		Dog::Print();
		cout << "나는 리트리버야" << endl;
	}
};

int main() 
{
	Animal* retriever = new Retriever();
	retriever->Print();
	delete retriever;
}