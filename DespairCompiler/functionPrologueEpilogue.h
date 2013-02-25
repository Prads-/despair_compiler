/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef _FUNCTION_PROLOGUE_EPILOGUE_H
#define _FUNCTION_PROLOGUE_EPILOGUE_H

#include <list>
#include "functionTable.h"
#include "identTable.h"
#include "groupTable.h"

namespace FunctionPrologueEpilogue {
	void addPrologueEpilogue(FunctionTable *funcTable, const GroupTable *grpTable);
}

#endif