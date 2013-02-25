/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef TOKEN_MODULE_H
#define TOKEN_MODULE_H

#include <vector>
#include <string>
#include <map>
#include "declarations.h"
#include "tokenLiner.h"
#include "groupTable.h"

struct TokenSubModule {
	uint32 moduleID, subModID;
	std::string name;
	std::vector<TokenLine> tokenLines;
};

typedef std::multimap<std::string, TokenSubModule> TokenModule;

namespace TokenMod {
	void createTokenModule(std::vector<std::vector<TokenLine> > *tokenLines, TokenModule *tokenModuleOut, GroupTable *groupTableOut);
}

#endif
