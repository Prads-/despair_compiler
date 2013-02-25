/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef EXPRESSION_PARSER_H
#define EXPRESSION_PARSER_H

#include <list>
#include <vector>
#include "declarations.h"
#include "intermediateRepresentation.h"
#include "token.h"
#include "identifierDataType.h"
#include "ExpressionToken.h"

namespace ExpressionParser {
	void parse(std::list<IntermediateRepresentation> *irOut, std::vector<ExpressionToken> *tks, ExpressionToken parseEnd, int intRegOffset,
		int floatRegOffset, bool floatExpression);
}

#endif