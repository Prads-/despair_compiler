/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include "parseHelper.h"
#include "functionTable.h"
#include "errorStrings.h"
#include "lockableException.h"
#include "instructionsSet.h"
using namespace std;
using namespace IREmitter;

//From parseModule.cpp
extern const GroupTable *groupTable;
extern Function *function;
extern Function *globalFunction;
extern FunctionTable *functionTable;
extern int parseExpression(const TokenLine *expression, IdentifierDataType dataType, Token terminator, int paramMemOffset, int intRegOffset, int floatRegOffset, bool isCompare, list<IntermediateRepresentation> *irsOut);

//Gets the function call statement
void ParseHelper::getFunctionCall(const TokenLine *tokenLine, uint32 &offset, TokenLine *funcCallOut) {
	int bracketLevel = 0;

	while(true) {
		funcCallOut->tokens.push_back(tokenLine->tokens.at(offset));
		if (tokenLine->tokens.at(offset).operation == OPERATOR_OPEN_ROUND_BRACKET) {
			++bracketLevel;
		} else if (tokenLine->tokens.at(offset).operation == OPERATOR_CLOSE_ROUND_BRACKET) {
			--bracketLevel;
			if (bracketLevel == 0) {
				break;
			}
		}
		++offset;
	}
}

//This function is called by getExpressionDataType only
//Checks whether or not the data type can be changed from current type to proposed type, and changes the data type
void changeDataType(IdentifierDataType &currentDataType, IdentifierDataType dataType, string *optCurrentDataTypeStr, const string *optDataTypeStr) {
	if (currentDataType > DATA_TYPE_STRING || dataType == DATA_TYPE_VOID) {
		goto throwException;
	} else {
		if (dataType > DATA_TYPE_STRING) {
			if (currentDataType == DATA_TYPE_VOID) {
				currentDataType = dataType;
				if (optDataTypeStr && optCurrentDataTypeStr) *optCurrentDataTypeStr = *optDataTypeStr;
			} else {
				goto throwException;
			}
		} else {
			if (currentDataType < dataType) {
				currentDataType = dataType;
				if (optDataTypeStr && optCurrentDataTypeStr) *optCurrentDataTypeStr = *optDataTypeStr;
			}
		}
	}
	return;

throwException:
	string errMsg = ERR_STR_TYPE_MISMATCH;
	throw LockableException(errMsg);
}

//This function is called by getExpressionDataType only
//Checks whether casting is possible or not and cast from one data type to another
IdentifierDataType getCastExpDataType(const TokenLine *castExpression, Keyword kw) {
	//Get DataType of cast expression
	bool isCompare;
	IdentifierDataType dataType = ParseHelper::getExpressionDataType(castExpression, 0, isCompare);
	if (isCompare || dataType > DATA_TYPE_FLOAT) {
		string errMsg = ERR_STR_TYPE_CASTING_INVALID;
		throw LockableException(errMsg);
	}

	switch (kw) {
		case KW_BYTE:
			return DATA_TYPE_BYTE;
			break;
		case KW_INT:
			return DATA_TYPE_INT;
			break;
		case KW_FLOAT:
			return DATA_TYPE_FLOAT;
			break;
		case KW_STRING:
			return DATA_TYPE_STRING;
		default:
			{
				string errStr = ERR_STR_SYNTAX_ERROR;
				throw LockableException(errStr.c_str());
			}
	}
}

//Gets the datatype of an expression
IdentifierDataType ParseHelper::getExpressionDataType(const TokenLine *expression, string *optDataTypeStrOut, bool &isCompare) {
	IdentifierDataType retDataType = DATA_TYPE_VOID;
	isCompare = false;

	for (uint32 i = 0; i < expression->tokens.size(); ++i) {
		switch (expression->tokens.at(i).type) {
			case TOKEN_KEYWORD:	//Data casting
				{
					Keyword keyword = expression->tokens.at(i++).keyword;

					//Get the cast expression
					TokenLine castExpression;
					int bracketLevel = 0;
					while (true) {
						if (expression->tokens.at(i).operation == OPERATOR_SEMI_COLON)  {
							break;
						}
						castExpression.tokens.push_back(expression->tokens.at(i++));
					}
					castExpression.tokens.push_back(Token(TOKEN_OPERATOR, KW_NONE, OPERATOR_SEMI_COLON, ";"));

					IdentifierDataType dataType = getCastExpDataType(&castExpression, keyword);
					changeDataType(retDataType, dataType, 0, 0);
				}
				break;
			case TOKEN_IDENTIFIER:
				switch (expression->tokens.at(i + 1).operation) {
					case OPERATOR_OPEN_ROUND_BRACKET:	//Function call
						{
							//Get the function call
							TokenLine funcCallLine;
							getFunctionCall(expression, i, &funcCallLine);

							//Get the function that is being called
							const Function *function = matchFunctionCall(&funcCallLine, 0);

							//Check if the function returns group and that group member is being accessed
							if (function->returnType == DATA_TYPE_GROUP && expression->tokens.at(i + 1).operation == OPERATOR_PERIOD) {
								const Ident *Ident = getGroupMember(expression, ++i, function->returnTypeStr, 0, 0, 0, 0, 0);
								changeDataType(retDataType, Ident->dataType, optDataTypeStrOut, &Ident->dataTypeStr);
							} else {
								changeDataType(retDataType, function->returnType, optDataTypeStrOut, (function->returnType == DATA_TYPE_GROUP) ?  &function->returnTypeStr : 0);
							}
						}
						break;
					case OPERATOR_PERIOD:	//Group
						{
							const Ident *ident = getIdent(expression, i, 0, 0, 0, 0, 0, 0);
							changeDataType(retDataType, ident->dataType, optDataTypeStrOut, &ident->dataTypeStr);
						}
						break;
					default:	//Variable
						const Ident *ident = getIdent(expression, i, 0, 0, 0, 0, 0, 0);
						changeDataType(retDataType, ident->dataType, optDataTypeStrOut, &ident->dataTypeStr);
				}
				break;
			case TOKEN_FLOAT_NUMBER:
				changeDataType(retDataType, DATA_TYPE_FLOAT, 0, 0);
				break;
			case TOKEN_NUMBER:
				changeDataType(retDataType, DATA_TYPE_INT, 0, 0);
				break;
			case TOKEN_CONST_CHAR:
				changeDataType(retDataType, DATA_TYPE_STRING, 0, 0);
				break;
			case TOKEN_OPERATOR:	//Check if there are any compare operators
				switch (expression->tokens.at(i).operation) {
					case OPERATOR_EQUAL:
					case OPERATOR_GREATER:
					case OPERATOR_GREATER_EQUAL:
					case OPERATOR_LESS:
					case OPERATOR_LESS_EQUAL:
						isCompare = true;
				}
		}
	}

	return retDataType;
}

const Ident *ParseHelper::getIdent(const TokenLine *tokenLine, uint32 &offset, int identReg, list<IntermediateRepresentation> *optIRSout, uint32 *optIndexOut, bool *optIsIndexNotValid, bool *optIsGlobal, int intRegOffset) {
	bool isGlobal = (function->name == "") ? true : false;
	if (optIsGlobal) *optIsGlobal = isGlobal;

	if (optIsIndexNotValid) *optIsIndexNotValid = false;

	//Get the ident
	IdentTable::const_iterator identIt = function->identTable.find(tokenLine->tokens.at(offset).value);
	if (identIt == function->identTable.end()) {
		//If ident is not in the local ident table, find it in global ident table
		pair<IdentTable::const_iterator, IdentTable::const_iterator> rangeIt;
		rangeIt = globalFunction->identTable.equal_range(tokenLine->tokens.at(offset).value);
		for (identIt = rangeIt.first; identIt != rangeIt.second; ++identIt) {
			if (identIt->second.isPublic || identIt->second.moduleID == function->moduleID) {
				break;
			}
		}
		if (identIt == rangeIt.second) {
			throw LockableException(ERR_STR_UNDECLARED_IDENT + tokenLine->tokens.at(offset).value);
		}
		isGlobal = true;
		if (optIsGlobal) *optIsGlobal = true;
	}
	const Ident *ident = &identIt->second;
	if (optIndexOut) *optIndexOut = ident->index;

	//If ident is a pointer and is accessing its element via index
	int arrayIndexReg;
	bool isIndexed = false;
	if ((ident->isPointer || ident->isArray) && tokenLine->tokens.at(offset + 1).operation == OPERATOR_OPEN_SQUARE_BRACKET) {
		isIndexed = true;
		offset += 2;

		//Get the expression inside the square brackets
		TokenLine indexExp;
		int bracketLevel = 0;
		while (true) {
			if (tokenLine->tokens.at(offset).operation == OPERATOR_OPEN_SQUARE_BRACKET) {
				++bracketLevel;
			} else if (tokenLine->tokens.at(offset).operation == OPERATOR_CLOSE_SQUARE_BRACKET) {
				--bracketLevel;
				if (bracketLevel < 0) break;
			}
			indexExp.tokens.push_back(tokenLine->tokens.at(offset++));
		}
		indexExp.tokens.push_back(Token(TOKEN_OPERATOR, KW_NONE, OPERATOR_SEMI_COLON, ";"));	//Terminator

		if (optIRSout) {
			//Get the data type of the expression
			bool isCompare;
			IdentifierDataType indexExpDataType = getExpressionDataType(&indexExp, 0, isCompare);
			if (indexExpDataType != DATA_TYPE_INT || isCompare) {
				throw LockableException(ERR_STR_TYPE_MISMATCH);
			}

			//Parse the expresssion
			arrayIndexReg = parseExpression(&indexExp, indexExpDataType, Token(TOKEN_OPERATOR, KW_NONE,
				OPERATOR_SEMI_COLON, ";"), function->identsSize, intRegOffset, 0, false, optIRSout);

			opcode_r_immi(optIRSout, _MUL_R_IMMI, arrayIndexReg, getDataTypeSize(ident->dataType, ident->dataTypeStr));
			if (optIsIndexNotValid) *optIsIndexNotValid = true;
			if (optIndexOut) *optIndexOut = 0;
		}
	}

	//Get the ident address IRS
	if (optIRSout) {
		if (ident->isPointer) {
			if (isGlobal) {
				opcode_r_m(optIRSout, _MOVP_R_M, identReg, ident->index);
			} else {
				opcode_r_r(optIRSout, _MOV_R_R, identReg, REG_BASE_POINTER);
				opcode_r_immi(optIRSout, _ADD_R_IMMI, identReg, ident->index);
				opcode_r_r(optIRSout, _MOVP_R_MR, identReg, identReg);
			}

			//Add the array index if it exists
			if (isIndexed) {
				opcode_r_r(optIRSout, _ADD_R_R, identReg, arrayIndexReg);
			}
		} else {
			opcode_r_r(optIRSout, _MOV_R_R, identReg, (isGlobal) ? REG_GLOBAL_BASE_POINTER : REG_BASE_POINTER);

			//Add the array index if it exists
			if (isIndexed) {
				opcode_r_immi(optIRSout, _ADD_R_IMMI, arrayIndexReg, ident->index);
				opcode_r_r(optIRSout, _ADD_R_R, identReg, arrayIndexReg);
			}
		}
	}

	//If ident is a group and is trying to access it's group member
	if (ident->dataType == DATA_TYPE_GROUP && tokenLine->tokens.at(offset + 1).operation == OPERATOR_PERIOD) {
		++offset;
		if (ident->isPointer) {
			if (optIsIndexNotValid) *optIsIndexNotValid = true;
			ident = getGroupMember(tokenLine, offset, ident->dataTypeStr, identReg, intRegOffset, optIRSout, 0, optIsIndexNotValid);
		} else {
			ident = getGroupMember(tokenLine, offset, ident->dataTypeStr, identReg, intRegOffset, optIRSout, optIndexOut, optIsIndexNotValid);
		}
	}
	if (optIsIndexNotValid && optIsGlobal) {
		if (*optIsIndexNotValid) *optIsGlobal = false;
	}
	return ident;
}

const Group *ParseHelper::getGroup(string groupName) {
	GroupTable::const_iterator groupIt = groupTable->find(groupName);
	if (groupIt == groupTable->end()) {
		string errMsg = ERR_STR_UNDECLARED_GROUP + groupName;
		throw LockableException(errMsg);
	}
	return &groupIt->second;
}

//Matches functions definition
Function *ParseHelper::matchFunctionCall(const TokenLine *functionCall, vector<TokenLine> *optParamsOut) {
	Function dummyFunction;
	dummyFunction.name = functionCall->tokens.at(0).value;

	//Get function call parameters
	int bracketLevel = 1;
	TokenLine paramTokens;
	bool ptrIsWeak = false;
	for (uint32 counter = 2; counter < functionCall->tokens.size(); ++counter) {
		switch (functionCall->tokens.at(counter).operation) {
			case OPERATOR_OPEN_ROUND_BRACKET:
				++bracketLevel;
				break;
			case OPERATOR_CLOSE_ROUND_BRACKET:
				--bracketLevel;
				if (bracketLevel != 0) break;
			case OPERATOR_COMMA:
				{
					if (bracketLevel > 1) break;
					Ident param;
					bool isCompare;
					paramTokens.tokens.push_back(Token(TOKEN_OPERATOR, KW_NONE, OPERATOR_SEMI_COLON, ";"));
					param.dataType = getExpressionDataType(&paramTokens, &param.dataTypeStr, isCompare);
					if (param.dataType != DATA_TYPE_VOID) {
						if (isCompare) {
							param.dataType = DATA_TYPE_INT;
						}
						param.ptrIsWeak = ptrIsWeak;
						dummyFunction.parameters.push_back(param);
						
						ptrIsWeak = false;
						if (optParamsOut) optParamsOut->push_back(paramTokens);
						paramTokens.tokens.clear();
					}
					continue;
				}
			default:
				if (functionCall->tokens.at(counter).keyword == KW_WEAK_PTR) {
					ptrIsWeak = true;
					continue;
				}
		}
		paramTokens.tokens.push_back(functionCall->tokens.at(counter));
	}

	//Check the function table to see if there is a function with same definition as dummy function
	pair<FunctionTable::iterator, FunctionTable::iterator> rangeIt;
	rangeIt = functionTable->equal_range(dummyFunction.name);
	for (FunctionTable::iterator it = rangeIt.first; it != rangeIt.second; ++it) {
		if (dummyFunction == it->second) {
			if (it->second.isPublic || it->second.moduleID == function->moduleID) {
				return &it->second;
			}
		}
	}

	//If the function was not found
	throw LockableException(ERR_STR_UNDECLARED_FUNCTION + dummyFunction.name);
}

const Ident *ParseHelper::getGroupMember(const TokenLine *tokenLine, uint32 &offset, string groupName, int grpMemAddrReg, int intRegOffset, list<IntermediateRepresentation> *optIRSout, uint32 *optIndexOut, bool *optIsIndexNotValid) {
	const Group *group = getGroup(groupName);

	uint32 grpMemIndex = 0;
	if (optIndexOut) grpMemIndex = *optIndexOut;
	IdentTable::const_iterator ident;
	while (true) {
		//Get the group member
		++offset;
		ident = group->members.find(tokenLine->tokens.at(offset).value);
		if (ident == group->members.end()) {
			string errMsg = ERR_STR_UNDECLARED_GRP_MEMBER + tokenLine->tokens.at(offset).value;
			throw LockableException(errMsg);
		}
		grpMemIndex += ident->second.index;

		//Check if the ident is accessing its element via index
		bool isIndexed = false;
		if ((ident->second.isPointer || ident->second.isArray) && tokenLine->tokens.at(offset + 1).operation == OPERATOR_OPEN_SQUARE_BRACKET) {
			isIndexed = true;
			offset += 2;

			//Get the expression inside the square brackets
			TokenLine indexExp;
			int bracketLevel = 0;
			while (true) {
				if (tokenLine->tokens.at(offset).operation == OPERATOR_OPEN_SQUARE_BRACKET) {
					++bracketLevel;
				} else if (tokenLine->tokens.at(offset).operation == OPERATOR_CLOSE_SQUARE_BRACKET) {
					--bracketLevel;
					if (bracketLevel < 0) break;
				}
				indexExp.tokens.push_back(tokenLine->tokens.at(offset++));
			}
			indexExp.tokens.push_back(Token(TOKEN_OPERATOR, KW_NONE, OPERATOR_SEMI_COLON, ";"));	//Terminator

			//Get the data type of the expression
			bool isCompare;
			IdentifierDataType indexExpDataType = getExpressionDataType(&indexExp, 0, isCompare);
			if (indexExpDataType != DATA_TYPE_INT || isCompare) {
				throw LockableException(ERR_STR_TYPE_MISMATCH);
			}

			if (optIRSout) {
				opcode_r_immi(optIRSout, _ADD_R_IMMI, grpMemAddrReg, grpMemIndex);
				if (ident->second.isPointer) opcode_r_mr(optIRSout, _MOVP_R_MR, grpMemAddrReg, grpMemAddrReg);

				//Parse the expresssion
				int arrayIndexReg = parseExpression(&indexExp, indexExpDataType, Token(TOKEN_OPERATOR, KW_NONE,
					OPERATOR_SEMI_COLON, ";"), function->identsSize, intRegOffset,  0, false, optIRSout);

				opcode_r_immi(optIRSout, _MUL_R_IMMI, arrayIndexReg, ident->second.size);
				opcode_r_r(optIRSout, _ADD_R_R, grpMemAddrReg, arrayIndexReg);
				if (optIsIndexNotValid) *optIsIndexNotValid = true;
				grpMemIndex = 0;
			}
		}

		//If the member is of type group and it's member is being accessed
		if (ident->second.dataType == DATA_TYPE_GROUP && tokenLine->tokens.at(offset + 1).operation == OPERATOR_PERIOD) {
			++offset;
			group = getGroup(ident->second.dataTypeStr);
			if (optIRSout && !isIndexed && ident->second.isPointer) {
				opcode_r_immi(optIRSout, _ADD_R_IMMI, grpMemAddrReg, grpMemIndex);
				opcode_r_mr(optIRSout, _MOVP_R_MR, grpMemAddrReg, grpMemAddrReg);
				if (optIsIndexNotValid) *optIsIndexNotValid = true;
				grpMemIndex = 0;
			}
		} else {
			break;
		}
	}

	if (optIRSout) {
		if (grpMemIndex > 0) opcode_r_immi(optIRSout, _ADD_R_IMMI, grpMemAddrReg, grpMemIndex);
		if (ident->second.isPointer) opcode_r_mr(optIRSout, _MOVP_R_MR, grpMemAddrReg, grpMemAddrReg);
	}
	if (optIndexOut) *optIndexOut = grpMemIndex;

	return &ident->second;
}

void ParseHelper::dmaTransfer(int regSrc, int regDest, uint32 size, list<IntermediateRepresentation> *irsOut) {
	opcode_immi_r(irsOut, _OUT_IMMI_R64, 66, regSrc);		//Source
	opcode_immi_r(irsOut, _OUT_IMMI_R64, 74, regDest);		//Destination
	opcode_immi_immi8(irsOut, _OUT_IMMI_IMMI8, 82, size);	//Size, initiate DMA transfer
}

int ParseHelper::getDataTypeSize(IdentifierDataType dataType, string dataTypeStr) {
	switch (dataType) {
		case DATA_TYPE_BYTE:
			return 1;
		case DATA_TYPE_FILE:
		case DATA_TYPE_IMAGE:
		case DATA_TYPE_STRING:
			return 8;
		case DATA_TYPE_FLOAT:
		case DATA_TYPE_INT:
			return 4;
		case DATA_TYPE_GROUP:
			{
				const Group *group = getGroup(dataTypeStr);
				return group->size;
			}
		default:
			return 0;
	}
}

//Offset should point just after the open bracket
void ParseHelper::getExpressionInsideBracket(const TokenLine *tokenLine, uint32 &offset, Operator openBracket, Operator closeBracket, TokenLine *tlOut) {
	int bracketLevel = 0;

	while (true) {
		if (tokenLine->tokens.at(offset).operation == openBracket) {
			++bracketLevel;
		} else if (tokenLine->tokens.at(offset).operation == closeBracket) {
			--bracketLevel;
			if (bracketLevel < 0) return;
		}
		tlOut->tokens.push_back(tokenLine->tokens.at(offset++));
	}
}