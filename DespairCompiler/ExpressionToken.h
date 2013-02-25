/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef EXPRESSION_TOKEN_H
#define EXPRESSION_TOKEN_H

#include <string>
#include "tokenTypes.h"
#include "identifierDataType.h"

struct ExpressionToken {
	TokenType type;
	IdentifierDataType	dataType;
	Operator operation;
	std::string strValue;
	uint32 value;
	bool isGlobalIdent, isPointer;

	ExpressionToken() { }

	ExpressionToken(TokenType type, IdentifierDataType dataType, Operator operation, std::string strValue, uint32 value, bool isGlobalIdent, bool isPointer) {
		this->type = type;
		this->dataType = dataType;
		this->operation = operation;
		this->strValue = strValue;
		this->value = value;
		this->isGlobalIdent = isGlobalIdent;
		this->isPointer = isPointer;
	}

	bool operator != (const ExpressionToken &expToken) {
		if (this->type == expToken.type && this->dataType == expToken.dataType && this->operation == expToken.operation && 
			this->strValue == expToken.strValue && this->value == expToken.value) {
				return false;
		}
		return true;
	}
};

#endif