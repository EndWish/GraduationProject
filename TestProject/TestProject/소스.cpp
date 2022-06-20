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
		cout << "���� �����̾�" << endl;
	}
};

class Dog : public Animal {
public:
	virtual void Print() {
		Animal::Print();
		cout << "���� ����" << endl;
	}
};

class Retriever : public Dog {
public:
	virtual void Print() {
		Dog::Print();
		cout << "���� ��Ʈ������" << endl;
	}
};

int main() 
{
	Animal* retriever = new Retriever();
	retriever->Print();
	delete retriever;
}