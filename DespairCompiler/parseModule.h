/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef PARSE_MODULE_H
#define PARSE_MODULE_H

#include <list>
#include <map>
#include <vector>
#include "declarations.h"
#include "intermediateRepresentation.h"
#include "tokenModule.h"
#include "functionTable.h"
#include "groupTable.h"
#include "constCharTable.h"

namespace ParseModule {
	uint32 parseModule(const TokenModule *tokenModule, const GroupTable *grpTable, FunctionTable *funcTable, ConstCharTable *cCTable);
}

#endif