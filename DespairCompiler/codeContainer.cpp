/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include "codeContainer.h"
#include <cstring>

template void CodeContainer::writeBack(uint8 val);
template void CodeContainer::writeBack(uint16 val);
template void CodeContainer::writeBack(uint32 val);
template void CodeContainer::writeBack(uint64 val);
template void CodeContainer::writeBack(float32 val);

CodeContainer::CodeContainer() {
	counter = 0;
	size = 1024 * 1024;	//1MB at start
	code = (uint8*)malloc(size);
}

CodeContainer::~CodeContainer() {
	free(code);
}

void CodeContainer::checkBoundary(int cSize) {
	if (size - counter < cSize) {
		size += cSize + 1024;	//1KB addition
		code = (uint8*)realloc(code, size);
	}
}

template <typename Type>
void CodeContainer::writeBack(Type val) {
	checkBoundary(sizeof(Type));
	*(Type*)&code[counter] = val;
	counter += sizeof(Type);
}

void CodeContainer::writeBig(uint8 *srcCode, int cSize) {
	checkBoundary(cSize);
	memcpy(&code[counter], srcCode, cSize);
	counter += cSize;
}

const uint8 *CodeContainer::getCode() const {
	return code;
}

uint32 CodeContainer::getSize() const {
	return counter;
}