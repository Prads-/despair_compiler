/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include "headerGenerator.h"
#include "sha256.h"
using namespace HeaderGenerator;
using namespace SHA256;

uint32 getGlobalPreDataSize(const ConstCharTable *constCharTable);
void getGlobalPreData(const ConstCharTable *constCharTable, uint8 *dataOut);

void HeaderGenerator::generateHeader(const CodeContainer *code, uint32 globalDataSize, uint32 codeOffset, const ConstCharTable *constCharTable, char *font, int fontSize, DPVMHeader *headerOut) {
	headerOut->part2.globalPreDataSize = getGlobalPreDataSize(constCharTable);
	if (headerOut->part2.globalPreDataSize == 0) {
		headerOut->part2.globalPreDataSize = fontSize;
	}

	headerOut->part1.magicNumber = 0x4450564D;
	headerOut->part1.version = 1;
	headerOut->part1.codeSize = code->getSize();
	headerOut->part1.headerSize = sizeof(HeaderPart1) + headerOut->part2.globalPreDataSize;
	headerOut->part1.codeOffset = codeOffset;
	headerOut->part1.globalDataSize = globalDataSize + headerOut->part2.globalPreDataSize;
	
	SHA_256_MessageDigest hash = sha256(code->getCode(), code->getSize());
	memcpy(headerOut->part1.signature, hash.h, 32);

	headerOut->part2.globalPreData = new uint8[fontSize + headerOut->part2.globalPreDataSize];
	memcpy(headerOut->part2.globalPreData, font, fontSize);		//Copy Font
	getGlobalPreData(constCharTable, headerOut->part2.globalPreData);	//Copy const strings
}

uint32 getGlobalPreDataSize(const ConstCharTable *constCharTable) {
	if (constCharTable->size() == 0) return 0;
	ConstCharTable::const_iterator it = constCharTable->end();
	--it;
	return it->second.index + it->second.val.size() + 1;
}

void getGlobalPreData(const ConstCharTable *constCharTable, uint8 *dataOut) {
	for (ConstCharTable::const_iterator it = constCharTable->begin(); it != constCharTable->end(); ++it) {
		memcpy(&dataOut[it->second.index], it->second.val.c_str(), it->second.val.size() + 1);
	}
}