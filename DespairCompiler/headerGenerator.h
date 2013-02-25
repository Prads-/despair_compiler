/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef HEADER_GENERATOR_H
#define HEADER_GENERATOR_H

#include "codeContainer.h"
#include "constCharTable.h"

namespace HeaderGenerator {
	#pragma pack(push)
	#pragma pack(1)
	struct HeaderPart1 {
		uint32 magicNumber, version, headerSize, codeSize, codeOffset, dataSize, stackSize, globalDataSize, frameBufferWidth, frameBufferHeight;
		uint32 signature[8];
	};

	struct HeaderPart2 {
		uint8 *globalPreData;
		uint32 globalPreDataSize;

		HeaderPart2() {
			globalPreData = 0;
		}

		~HeaderPart2() {
			delete [] globalPreData;
			globalPreData = 0;
		}
	};

	struct DPVMHeader {
		HeaderPart1 part1;
		HeaderPart2 part2;
	};
	#pragma pack(pop)
	
	void generateHeader(const CodeContainer *code, uint32 globalDataSize, uint32 codeOffset, const ConstCharTable *constCharTable, char *font, int fontSize, DPVMHeader *headerOut);
}

#endif