/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef CONST_TABLE_H
#define CONST_TABLE_H

#include <string>
#include <map>
#include <vector>
#include "tokenLiner.h"
#include"declarations.h"

typedef std::map<std::string, Token> ConstantTable;

namespace ConstTable {
	void createConstantTable(ConstantTable *tblOut, std::vector<std::vector<TokenLine> > *tokenLines);
}

#endif
