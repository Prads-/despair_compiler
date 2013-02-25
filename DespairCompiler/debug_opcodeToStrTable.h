/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include "declarations.h"

#ifdef _DEBUG_MODE
#ifndef _DEBUG_OPCODE_TO_STR_TABLE_H
#define _DEBUG_OPCODE_TO_STR_TABLE_H

#include <string>

class Debug_OpcodeToStr {
private:
	std::string opcode[0x177];
public:
	Debug_OpcodeToStr();
	std::string debug_opcodeToStr(uint16 op);
};

#endif
#endif