/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include "declarations.h"
#include "tokenTypes.h"

struct Token {
	TokenType type;
	Keyword keyword;
	Operator operation;
	std::string value;

	Token() {}

	Token(TokenType type, Keyword kw, Operator op, std::string val) {
		this->type = type;
		keyword = kw;
		operation = op;
		value = val;
	}

	bool operator != (const Token &token) const {
		if (this->keyword == token.keyword && this->operation == token.operation && this->type == token.type)
			return false;
		return true;
	}

	bool operator == (const Token &token) const {
		if (*this != token)
			return false;
		return true;
	}
};

#endif
