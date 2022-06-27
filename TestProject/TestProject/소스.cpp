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

	Dog& operator=(const Dog& other) {	// 복사 할당 연산자
		if (this == &other)
			return *this;

		if (pNum)
			delete pNum;

		pNum = new int;
		memcpy(pNum, other.pNum, sizeof(int));

		cout << "복사할당\n";
		return *this;
	}

	//Dog& operator=(Dog&& other) noexcept {	// 이동 할당 연산자
	//	if (this == &other)
	//		return *this;

	//	if (pNum)
	//		delete pNum;

	//	pNum = other.pNum;
	//	other.pNum = nullptr;

	//	cout << "이동할당\n";
	//	return *this;
	//}

};

int main() 
{
	Dog a(10);
	Dog b;
	b = move(a);
	*b.pNum = 5;

	cout << "a의 num" << *a.pNum << "\n";
	cout << "b의 num" << *b.pNum << "\n";
}