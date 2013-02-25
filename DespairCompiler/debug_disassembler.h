/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include "declarations.h"

#ifdef _DEBUG_MODE
#ifndef _DEBUG_DISASSEMBLER_H
#define _DEBUG_DISASSEMBLER_H

#include <sstream>
#include <list>
#include <string>
#include "declarations.h"
#include "debug_opcodeToStrTable.h"
#include "intermediateRepresentation.h"

class Debug_Disassembler {
private:
	Debug_OpcodeToStr opToStr;
public:
	std::string disassemble(const std::list<IntermediateRepresentation> *irs);
};

#endif
#endif