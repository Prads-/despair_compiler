/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include "stringExtension.h"
using namespace std;

void StringExtension::strAppendInteger(int integer, string *strOut) {
	string appendStr = intToStr(integer);
	strOut->append(appendStr);
}

std::string StringExtension::intToStr(int integer) {
	string retStr = "";
	int digit;
	bool neg = false;
	
	if (integer < 0) {
		neg = true;
		integer = ~integer + 1;
	}
	while (integer != 0) {
		digit = integer % 10;
		retStr += (char)(digit + 48);
		integer /= 10;
	}
	for (int start = 0, end = retStr.size() - 1; start < retStr.size() / 2; ++start, --end) {
		retStr[start] ^= retStr[end];
		retStr[end] ^= retStr[start];
		retStr[start] ^= retStr[end];
	}
	if (neg) retStr.insert(retStr.begin(), '-');

	return retStr;
}

bool StringExtension::getLineFromFile(ifstream *file, string *lineOut) {
	bool retVal = getline(*file, *lineOut);
	if (retVal) {
		if (lineOut->at(lineOut->size() - 1) == '\r') {
			lineOut->resize(lineOut->size() - 1);
		}
	}
	return retVal;
}