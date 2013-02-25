/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "declarations.h"
#include "identTable.h"
#include "functionTable.h"
#include "groupTable.h"
#include "tokenModule.h"
#include "identifierDataType.h"
#include "errorStrings.h"
#include "constCharTable.h"

namespace SymbolTable {
	void createSymbolTable(TokenModule *tokenModule, const GroupTable *groupTable, FunctionTable *functionTable, ConstCharTable *constCharTable, int globalIndexOffset);
}

#endif