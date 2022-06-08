#include <iostream>
#include <fstream>
#include <random>
#include <ranges>
#include <format>

#include <vector>
#include <chrono>

using namespace std;

random_device rd;
default_random_engine dre{ rd() };
uniform_int_distribution<int> uid;

int main() 
{
	//저장하기
	//ofstream out("LoadData", ios::binary);

	//for (int i = 0; i < 10'000'000; ++i) {	//천만개
	//	int temp = uid(dre);
	//	out.write((const char*)&temp, sizeof(temp));
	//}

	vector<int> dumy(10'000'000);
	for (int i = 0; i < 10'000'000; ++i) {	//천만개
		dumy[i] = uid(dre);
	}

	//읽어오기
	auto start1 = chrono::steady_clock::now();

	ifstream in("LoadData", ios::binary);
	vector<int> v(10'000'000);
	in.read((char*)v.data(), sizeof(int) * 10'000'000);
	//cout << *v.rbegin() << endl;

	auto stop1 = chrono::steady_clock::now();
	auto et1 = chrono::duration_cast<chrono::microseconds>(stop1 - start1).count();
	cout << format("binary file 에서 vector로 읽어오기 - {0:8} us", et1) << endl;

	//복사하기
	auto start2 = chrono::steady_clock::now();

	vector<int> v2(10'000'000);
	ranges::copy(dumy, v2.begin());
	//cout << *v2.rbegin() << endl;

	auto stop2 = chrono::steady_clock::now();
	auto et2 = chrono::duration_cast<chrono::microseconds>(stop2 - start2).count();
	cout << format("vector에서 vector로 copy하기 	   - {0:8} us", et2) << endl;

	//cout << "1";
}