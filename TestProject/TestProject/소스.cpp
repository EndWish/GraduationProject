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

int main() 
{

	shared_ptr<int> sp(new int(11));
	int& a = *sp;
	sp.reset();
	cout << sp.use_count() << endl;
	cout << a << endl;

}