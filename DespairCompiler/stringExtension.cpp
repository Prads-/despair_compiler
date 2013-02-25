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
	string appendStr = "";
	int digit;
	bool neg = false;
	
	if (integer < 0) {
		neg = true;
		integer = ~integer + 1;
	}
	while (integer != 0) {
		digit = integer % 10;
		appendStr += (char)(digit + 48);
		integer /= 10;
	}
	for (int start = 0, end = appendStr.size() - 1; start < appendStr.size() / 2; ++start, --end) {
		appendStr[start] ^= appendStr[end];
		appendStr[end] ^= appendStr[start];
		appendStr[start] ^= appendStr[end];
	}
	if (neg) appendStr.insert(appendStr.begin(), '-');

	strOut->append(appendStr);
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