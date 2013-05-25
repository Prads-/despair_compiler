#include "constantParser.h"
#include "tokenTypes.h"
#include "lockableException.h"
#include "errorStrings.h"

int ConstantParser::constantToInteger(const TokenLine *constantTokens) {
	int index = 1, retVal;
	retVal = atoi(constantTokens->tokens.at(0).value.c_str());

	while (true) {
		//Operator
		switch (constantTokens->tokens.at(index++).operation) {
			case OPERATOR_ADD:
				retVal += atoi(constantTokens->tokens.at(index++).value.c_str());
				break;
			case OPERATOR_SUBSTRACT:
				retVal -= atoi(constantTokens->tokens.at(index++).value.c_str());
				break;
			case OPERATOR_MULTIPLY:
				retVal *= atoi(constantTokens->tokens.at(index++).value.c_str());
				break;
			case OPERATOR_DIVIDE:
				retVal /= atoi(constantTokens->tokens.at(index++).value.c_str());
				break;
			case OPERATOR_AND:
				retVal &= atoi(constantTokens->tokens.at(index++).value.c_str());
				break;
			case OPERATOR_OR:
				retVal |= atoi(constantTokens->tokens.at(index++).value.c_str());
				break;
			case OPERATOR_XOR:
				retVal ^= atoi(constantTokens->tokens.at(index++).value.c_str());
				break;
			case OPERATOR_MOD:
				retVal %= atoi(constantTokens->tokens.at(index++).value.c_str());
				break;
			case OPERATOR_SEMI_COLON:
				return retVal;
			default:
				throw LockableException(ERR_STR_INVALID_CONSTANT + constantTokens->toString());
		}
	}
}