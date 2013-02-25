/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <list>
#include <string>
#include "functionTable.h"
#include "codeContainer.h"
#include "headerGenerator.h"
#include "constCharTable.h"

namespace CodeGenerator {
	void generateCode(FunctionTable *functionTable, uint32 mainFunctionID, CodeContainer *codeOut, HeaderGenerator::DPVMHeader *header, const ConstCharTable *constCharTable, char *font, int fontSize);
}

#endif