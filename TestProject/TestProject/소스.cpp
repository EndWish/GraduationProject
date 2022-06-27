#include <iostream>
#include <fstream>
#include <random>
#include <ranges>
#include <format>
#include <memory.h>

#include <vector>
#include <chrono>

using namespace std;

class Dog {
public:
	int* pNum = nullptr;

	Dog() {
		pNum = new int(0);
	}
	Dog(int num) {
		pNum = new int(num);
	}

	Dog& operator=(const Dog& other) {	// ���� �Ҵ� ������
		if (this == &other)
			return *this;

		if (pNum)
			delete pNum;

		pNum = new int;
		memcpy(pNum, other.pNum, sizeof(int));

		cout << "�����Ҵ�\n";
		return *this;
	}

	//Dog& operator=(Dog&& other) noexcept {	// �̵� �Ҵ� ������
	//	if (this == &other)
	//		return *this;

	//	if (pNum)
	//		delete pNum;

	//	pNum = other.pNum;
	//	other.pNum = nullptr;

	//	cout << "�̵��Ҵ�\n";
	//	return *this;
	//}

};

int main() 
{
	Dog a(10);
	Dog b;
	b = move(a);
	*b.pNum = 5;

	cout << "a�� num" << *a.pNum << "\n";
	cout << "b�� num" << *b.pNum << "\n";
}