/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef SWITCH_H
#define SWITCH_H

#include "declarations.h"
#include <vector>
#include "tokenLiner.h"
#include <list>
#include "intermediateRepresentation.h"

namespace SwitchParser {
	void parseSwitch(const std::vector<TokenLine> *tokenLines, uint32 &index, int &label, int lblContinue, std::list<IntermediateRepresentation> *irsOut);
}

#endif