/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef _MAIN_FUNCTION_H
#define _MAIN_FUNCTION_H

#include <string>
#include "declarations.h"
#include "functionTable.h"

namespace MainFunction {
	uint32 findMainFunction(const FunctionTable *functionTable);
	Function *getMainFunction(FunctionTable *functionTable, uint32 mainFunctionID);
	void fixMainFunction(FunctionTable *functionTable, uint32 mainFuncID);
}

#endif