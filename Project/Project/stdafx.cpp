#include "stdafx.h"

void ReadStringBinary(string& _dest, ifstream& _file){
	int len;
	_file.read((char*)&len, sizeof(len));
	_dest.assign(len, ' ');
	_file.read((char*)_dest.data(), sizeof(char) * len);
}
