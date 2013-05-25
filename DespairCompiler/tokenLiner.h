/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef TOKEN_LINER_H
#define TOKEN_LINER_H

#include <vector>
#include <stdexcept>
#include "token.h"
#include "declarations.h"

enum TokenLineType {
	TOKEN_LINE_ASSIGNMENT, TOKEN_LINE_COMPARE, TOKEN_LINE_LOOP, TOKEN_LINE_CLOSE_SCOPE, TOKEN_LINE_GROUP, TOKEN_LINE_VAR_DECLARE,
	TOKEN_LINE_FUNCTION_DECLARE, TOKEN_LINE_FUNCTION_CALL, TOKEN_LINE_RETURN, TOKEN_LINE_POINTER_ASSIGNMENT,
	TOKEN_LINE_DEALLOCATION, TOKEN_LINE_THREAD_START, TOKEN_LINE_CONST_DECLARE, TOKEN_LINE_CASE, TOKEN_LINE_DEFAULT, TOKEN_LINE_BREAK,
	TOKEN_LINE_SWITCH, TOKEN_LINE_FCASE, TOKEN_LINE_FDEFAULT, TOKEN_LINE_CONTINUE
};

struct TokenLine {
	std::vector<Token> tokens;
	TokenLineType type;

	//Used only if line is in global sub module
	uint32 moduleID;

	std::string toString() const {
		std::string retStr = "";
		for (uint32 i = 0; i < tokens.size(); ++i) {
			retStr += tokens[i].value + " ";
		}
		return retStr;
	}
};

namespace TokenLiner {
	void getTokenLines(const std::vector<Token> *tokens, std::vector<TokenLine> *tLineOut);
}

#endif
