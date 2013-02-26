/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef STRING_PARSER_H
#define STRING_PARSER_H

#include <list>
#include "declarations.h"
#include "tokenLiner.h"
#include "intermediateRepresentation.h"

namespace StringParser {
	void parseString(const TokenLine *tokenLine, uint32 offset, int intRegOffset, int floatRegOffset, int paramMemOffset, Token terminator, std::list<IntermediateRepresentation> *irsOut);
}

#endif