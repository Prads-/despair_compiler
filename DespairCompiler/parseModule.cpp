/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include <cstdlib>
#include "parseModule.h"
#include "errorStrings.h"
#include "expressionParser.h"
#include "intermediateRepresentation.h"
#include "instructionsSet.h"
#include "functionPrologueEpilogue.h"
#include "mainFunction.h"
#include "lockableException.h"
#include "portAddress.h"
#include "parseHelper.h"
#include "stringParser.h"
#include "switch.h"
using namespace std;
using namespace IREmitter;
using namespace ExpressionParser;
using namespace FunctionPrologueEpilogue;
using namespace MainFunction;
using namespace ParseHelper;
using namespace StringParser;
using namespace SwitchParser;

void parseLines(const vector<TokenLine> *tokenLines, uint32 &offset, list<IntermediateRepresentation> *irsOut, int lblBreak = 0, int lblContinue = 0);
int parseExpression(const TokenLine *expression, IdentifierDataType dataType, Token terminator, int paramMemOffset, int intRegOffset, int floatRegOffset, bool isCompare, list<IntermediateRepresentation> *irsOut);
void assignment(const TokenLine *tokenLine, list<IntermediateRepresentation> *irsOut);
void pointerAssignment(const TokenLine *tokenLine, list<IntermediateRepresentation> *irsOut);
IdentifierDataType memoryAllocation(const TokenLine *allocationLine, string *dataTypeStrOut, list<IntermediateRepresentation> *irsOut);
void memoryDeallocation(const TokenLine *deallocationLine, list<IntermediateRepresentation> *irsOut);
void compare(const vector<TokenLine> *tokenLines, uint32 tokenOffset, uint32 &lineOffset, int lblEnd, list<IntermediateRepresentation> *irsOut, int lblBreak = 0, int lblContinue = 0);
void lineReturn(const TokenLine *tokenLine, list<IntermediateRepresentation> *irsOut);
void ifElseStatement(const vector<TokenLine> *tokenLines,uint32 &lineOffset, list<IntermediateRepresentation> *irsOut, int lblBreak = 0, int lblContinue = 0);
void whileLoop(const vector<TokenLine> *tokenLines,uint32 &lineOffset, list<IntermediateRepresentation> *irsOut);
void startThread(const TokenLine *tokenLine, list<IntermediateRepresentation> *irsOut);
const Function *functionCall(const TokenLine *functionCallTokens, int paramMemOffset, int intRegOffset, int floatRegOffset, list<IntermediateRepresentation> *irsOut);

const GroupTable *groupTable;
const ConstCharTable *constCharTable;
FunctionTable *functionTable;
Function *function;
Function *globalFunction;
int label = 1, functionEndLabel;	//functionEndLabel marks the end of the function body, just before epilogue

uint32 ParseModule::parseModule(const TokenModule *tokenModule, const GroupTable *grpTable, FunctionTable *funcTable, ConstCharTable *cCTable) {
	//Get main function ID
	uint32 mainFuncID = findMainFunction(funcTable);
	getMainFunction(funcTable, mainFuncID)->isCalled = true;	//Main function is always called at the beginning of the program

	groupTable = grpTable;
	constCharTable = cCTable;
	functionTable = funcTable;
	globalFunction = &funcTable->find("")->second;

	//Parse the main function first, so find the token module of main function
	TokenModule::const_iterator tMainModule;
	pair<TokenModule::const_iterator, TokenModule::const_iterator> rangeIt = tokenModule->equal_range("main");
	for (tMainModule = rangeIt.first; tMainModule != rangeIt.second; ++tMainModule) {
		if (tMainModule->second.subModID == mainFuncID) break;
	}

	//Parse functions (sub-modules)
	TokenModule::const_iterator tModIt = tMainModule;
	while (tModIt != tokenModule->end()) {
		pair<FunctionTable::iterator, FunctionTable::iterator> rangeIt;
		rangeIt = funcTable->equal_range(tModIt->second.name);
		for (FunctionTable::iterator it = rangeIt.first; it != rangeIt.second; ++it) {
			if (it->second.functionID == tModIt->second.subModID) {
				function = &it->second;
				break;
			}
		}

		if (function->isCalled && !function->isParsed) {
			uint32 offset = (function->name == "") ? 0 : 1;	//If global module then start with first line else skip the first function declaration line
			functionEndLabel = label++;
			parseLines(&tModIt->second.tokenLines, offset, &function->irs);
			if (function->name != "") opcode(&function->irs, _NOP, functionEndLabel);	//Put the function end label at the end of the function
			function->isParsed = true;

			//Start searching from beginning of the token module again
			tModIt = tokenModule->begin();
		} else {
			++tModIt;
		}
	}

	//Add function prologue and epilogue
	addPrologueEpilogue(funcTable, grpTable);

	fixMainFunction(funcTable, mainFuncID);

	return mainFuncID;
}

//Goes through a function line by line and parses them using appropriate function
void parseLines(const vector<TokenLine> *tokenLines, uint32 &offset, list<IntermediateRepresentation> *irsOut, int lblBreak, int lblContinue) {
	while (offset < tokenLines->size()) {
		try {
			switch (tokenLines->at(offset).type) {
				case TOKEN_LINE_ASSIGNMENT:
					assignment(&tokenLines->at(offset), irsOut);
					break;
				case TOKEN_LINE_POINTER_ASSIGNMENT:
					pointerAssignment(&tokenLines->at(offset), irsOut);
					break;
				case TOKEN_LINE_CLOSE_SCOPE:
					++offset;
					return;
				case TOKEN_LINE_COMPARE:
					if (tokenLines->at(offset).tokens.at(0).keyword == KW_IF) {
						ifElseStatement(tokenLines, offset, irsOut, lblBreak, lblContinue);
					} else {
						string errMsg = ERR_STR_INVALID_CMP_STATEMENT;
						throw LockableException(errMsg);
					}
					continue;
				case TOKEN_LINE_FUNCTION_CALL:
					functionCall(&tokenLines->at(offset), function->identsSize, 1, 1, irsOut);
					break;
				case TOKEN_LINE_LOOP:
					if (tokenLines->at(offset).tokens.at(0).keyword == KW_WHILE) {
						whileLoop(tokenLines, offset, irsOut);
					} else {
						throw LockableException(ERR_STR_INVALID_LOOP_STATEMENT);
					}
					continue;
				case TOKEN_LINE_RETURN:
					lineReturn(&tokenLines->at(offset), irsOut);
					break;
				case TOKEN_LINE_DEALLOCATION:
					memoryDeallocation(&tokenLines->at(offset), irsOut);
					break;
				case TOKEN_LINE_THREAD_START:
					startThread(&tokenLines->at(offset), irsOut);
					break;
				case TOKEN_LINE_BREAK:
					if (lblBreak == 0) {
						LockableException e = LockableException(ERR_STR_INVALID_BREAK + tokenLines->at(offset).toString());
						e.lock();
						throw e;
					}
					opcode_label(irsOut, _JMP_IMMI, lblBreak);
					break;
				case TOKEN_LINE_CONTINUE:
					if (lblContinue == 0) {
						LockableException e = LockableException(ERR_STR_INVALID_CONTINUE + tokenLines->at(offset).toString());
						e.lock();
						throw e;
					}
					opcode_label(irsOut, _JMP_IMMI, lblContinue);
					break;
				case TOKEN_LINE_SWITCH:
					parseSwitch(tokenLines, offset, label, lblContinue, irsOut);
					continue;
			}
			++offset;
		} catch (LockableException &e) {
			string errStr = " in line '" + tokenLines->at(offset).toString() + "'";
			e.appendMessageAndLock(errStr);
			throw e;
		}
	}
}

int parseExpression(const TokenLine *expression, IdentifierDataType dataType, Token terminator, int paramMemOffset, int intRegOffset, int floatRegOffset, bool isCompare,
					 list<IntermediateRepresentation> *irsOut) {
	vector<ExpressionToken> expTokens;

	for (uint32 i = 0; i < expression->tokens.size(); ++i) {
		switch (expression->tokens.at(i).type) {
			case TOKEN_IDENTIFIER:
				switch (expression->tokens.at(i + 1).operation) {
					case OPERATOR_OPEN_ROUND_BRACKET:	//Function Call
						{
							//Get function call tokens
							TokenLine functionCallTokens;
							getFunctionCall(expression, i, &functionCallTokens);

							//Get Function call IRs
							list<IntermediateRepresentation> funcCallIRs;
							const Function *calledFunction = functionCall(&functionCallTokens, paramMemOffset, intRegOffset, floatRegOffset, &funcCallIRs);

							//If there are any register separated, push those registers in stack, put the funcCallIRs in function body, and pop the registers
							list<IntermediateRepresentation> regPush;
							list<IntermediateRepresentation> regPop;
							if (intRegOffset == 2) {
								opcode_r(&regPush, _PUSH_R, 1);
								opcode_r(&regPop, _POP_R, 1);
							} else if (intRegOffset > 2) {
								opcode_r_r(&regPush, _PUSHES_R_R, 1, intRegOffset - 1);
								opcode_r_r(&regPop, _POPS_R_R, 1, intRegOffset - 1);
							}
							if (floatRegOffset == 2) {
								opcode_r(&regPush, _FPUSH_FR, 1);
								opcode_r(&regPop, _FPOP_FR, 1);
							} else if (floatRegOffset > 2) {
								opcode_fr_fr(&regPush, _FPUSHES_FR_FR, 1, floatRegOffset - 1);
								opcode_fr_fr(&regPop, _FPOPS_FR_FR, 1, floatRegOffset - 1);
							}
							regPop.reverse();
							irsOut->splice(irsOut->end(), regPush);
							irsOut->splice(irsOut->end(), funcCallIRs);
							irsOut->splice(irsOut->end(), regPop);

							IdentifierDataType retDataType = calledFunction->returnType;

							//Check if the function return type is group and it is accessing group member
							if (retDataType == DATA_TYPE_GROUP && expression->tokens.at(i + 1).operation == OPERATOR_PERIOD) {
								//Get the group member
								list<IntermediateRepresentation> groupMemberAddress;
								bool isIndexNotValid;
								uint32 index = 0;
								const Ident *ident = getGroupMember(expression, ++i, calledFunction->returnTypeStr, 0, intRegOffset, &groupMemberAddress, &index, &isIndexNotValid);
								retDataType = ident->dataType;

								//Put the value in group member to register 0
								if (isIndexNotValid) {
									irsOut->splice(irsOut->end(), groupMemberAddress);
								} else {
									opcode_r_immi(irsOut, _ADD_R_IMMI, 0, index);
									if (ident->isPointer) {
										opcode_r_mfr(irsOut, _MOVP_R_MR, 0, 0);
									}
								}
								(retDataType == DATA_TYPE_FLOAT) ? opcode_fr_mfr(irsOut, _FMOV_FR_MFR, 0, 0) : opcode_r_mr(irsOut, _MOV_R_MR, 0, 0);
							}

							uint32 reg;
							if (dataType == DATA_TYPE_FLOAT) {
								//Seperate a register for this function
								reg = floatRegOffset++;

								//Put the return value in the separated register
								(retDataType == DATA_TYPE_FLOAT) ?
									opcode_fr_fr(irsOut, _FMOV_FR_FR, reg, 0) :
									opcode_fr_r(irsOut, _FCON_FR_R, reg, 0);
							} else {
								//Seperate a register for this function
								reg = intRegOffset++;

								//Put the return value in the separated register
								(retDataType == DATA_TYPE_FLOAT) ?
									opcode_r_fr(irsOut, _FCON_R_FR, reg, 0) :
									opcode_r_r(irsOut, _MOV_R_R, reg, 0);
							}

							//Add the expression token
							ExpressionToken expToken(TOKEN_REGISTER, dataType, OPERATOR_NONE, "", reg, false, false);
							expTokens.push_back(expToken);
						}
						break;
					default:	//Group and Variable
						{
							//Get the ident
							list<IntermediateRepresentation> identAddrIRs; //Will be used only if index is not valid
							uint32 identIndex;
							bool isIdentIndexNotValid, isIdentGlobal;
							const Ident *ident = getIdent(expression, i, intRegOffset, &identAddrIRs, &identIndex, &isIdentIndexNotValid, &isIdentGlobal, intRegOffset + 1);

							//Create Expression Token
							if (isIdentIndexNotValid) {
								irsOut->splice(irsOut->end(), identAddrIRs);

								ExpressionToken expToken(TOKEN_REGISTER, ident->dataType, OPERATOR_NONE, "", intRegOffset++, false, true);
								expTokens.push_back(expToken);
							} else {
								ExpressionToken expToken(TOKEN_IDENTIFIER, ident->dataType, OPERATOR_NONE, "", identIndex, isIdentGlobal, ident->isPointer);
								expTokens.push_back(expToken);
							}
						}
				}
				break;
			case TOKEN_FLOAT_NUMBER:
				{
					ExpressionToken expToken(TOKEN_FLOAT_NUMBER, DATA_TYPE_FLOAT, OPERATOR_NONE, expression->tokens.at(i).value, 0, false, false);
					expTokens.push_back(expToken);
				}
				break;
			case TOKEN_NUMBER:
				{
					ExpressionToken expToken(TOKEN_NUMBER, DATA_TYPE_FLOAT, OPERATOR_NONE, expression->tokens.at(i).value, 0, false, false);
					expTokens.push_back(expToken);
				}
				break;
			case TOKEN_OPERATOR:
				{
					ExpressionToken expToken(TOKEN_OPERATOR, DATA_TYPE_VOID, expression->tokens.at(i).operation, expression->tokens.at(i).value, 0, false, false);
					expTokens.push_back(expToken);
				}
		}
	}

	//Parse the expresion tokens
	parse(irsOut, &expTokens, ExpressionToken(TOKEN_OPERATOR, DATA_TYPE_VOID, OPERATOR_SEMI_COLON, ";", 0, false, false), intRegOffset, floatRegOffset,
		(dataType == DATA_TYPE_FLOAT) ? true : false);

	if (isCompare) {
		return intRegOffset;
	} else {
		return (dataType == DATA_TYPE_FLOAT) ? floatRegOffset : intRegOffset;
	}
}

void assignment(const TokenLine *tokenLine, list<IntermediateRepresentation> *irsOut) {
	uint32 tokenCounter = 0;

	//Get the destination address
	bool isDestIdentGlobal, isDestIdentIndexNotValid;
	list<IntermediateRepresentation> destAddress;	//destAddress will be used if the index is not valid
	uint32 destIdentIndex;
	const int REG_DEST_POINTER = 1;
	const Ident *destIdent = getIdent(tokenLine, tokenCounter, REG_DEST_POINTER, &destAddress, &destIdentIndex, &isDestIdentIndexNotValid, &isDestIdentGlobal, REG_DEST_POINTER + 1);

	//Check if the ident is an pointer or array which was indexed
	bool wasIndexed = false;
	if (tokenLine->tokens.at(tokenCounter).operation == OPERATOR_CLOSE_SQUARE_BRACKET) {
		wasIndexed = true;
	}

	++tokenCounter;
	if (tokenLine->tokens.at(tokenCounter++).operation != OPERATOR_ASSIGNMENT) {
		string errMsg = ERR_STR_SYNTAX_ERROR;
		throw LockableException(errMsg.c_str());
	}

	//Get the source expression
	TokenLine srcExpression;
	do {
		srcExpression.tokens.push_back(tokenLine->tokens.at(tokenCounter));
	} while(tokenLine->tokens.at(tokenCounter++).operation != OPERATOR_SEMI_COLON);

	if (srcExpression.tokens.at(0).keyword == KW_NEW) { //If memory allocation
		if (!destIdent->isPointer) {
			throw LockableException(ERR_STR_ALLOC_IS_NOT_A_POINTER);
		}
		//If the ident was indexed, we can't assign a pointer to it
		if (wasIndexed) {
			throw LockableException(ERR_STR_DEST_NOT_POINTER);
		}
		
		if (isDestIdentIndexNotValid) {
			//We don't need the value inside the ident, but the address of the ident. So, if the last opcode was MOVP_R_MR then delete it
			list<IntermediateRepresentation>::iterator it = destAddress.end();
			--it;
			if (it->opcode == _MOVP_R_MR) {
				destAddress.erase(it);
			}

			irsOut->splice(irsOut->end(), destAddress);
		}

		string srcDataTypeStr;
		IdentifierDataType srcDataType = memoryAllocation(&srcExpression, &srcDataTypeStr, irsOut);

		if (srcDataType == DATA_TYPE_GROUP) {
			if (srcDataTypeStr != destIdent->dataTypeStr) {
				throw LockableException(ERR_STR_ALLOC_TYPE);
			}
		} else {
			if (srcDataType != destIdent->dataType) {
				throw LockableException(ERR_STR_ALLOC_TYPE);
			}
		}

		//Move the newly created heap memory to the destination
		if (isDestIdentIndexNotValid) {
			opcode_mr_r(irsOut, _MOVP_MR_R, REG_DEST_POINTER, 0);
		} else {
			if (isDestIdentGlobal) {
				opcode_m_r(irsOut, _MOVP_M_R, destIdentIndex, 0);
			} else {
				opcode_r_r(irsOut, _MOV_R_R, REG_DEST_POINTER, REG_BASE_POINTER);
				opcode_r_immi(irsOut, _ADD_R_IMMI, REG_DEST_POINTER, destIdentIndex);
				opcode_mr_r(irsOut, _MOVP_MR_R, REG_DEST_POINTER, 0);
			}
		}
	} else {
		if (isDestIdentIndexNotValid) {
			irsOut->splice(irsOut->end(), destAddress);
		}

		string srcDataTypeStr;
		bool srcIsCompare;
		IdentifierDataType srcDataType = getExpressionDataType(&srcExpression, &srcDataTypeStr, srcIsCompare);

		//Get the IRs
		int srcReg;
		if (srcDataType <= DATA_TYPE_FLOAT && destIdent->dataType <= DATA_TYPE_FLOAT) {
			srcReg = parseExpression(&srcExpression, srcDataType, Token(TOKEN_OPERATOR, KW_NONE, OPERATOR_SEMI_COLON, ";"),
				function->identsSize, 2, 1, srcIsCompare, irsOut);
			if (srcIsCompare) srcDataType = DATA_TYPE_INT;
		}

		switch (destIdent->dataType) {
			case DATA_TYPE_BYTE:
				switch (srcDataType) {
					case DATA_TYPE_BYTE:
					case DATA_TYPE_INT:
						if (isDestIdentIndexNotValid) {
							opcode_mr_r(irsOut, _BMOV_BR_R, REG_DEST_POINTER, srcReg);
						} else {
							if (isDestIdentGlobal) {
								if (destIdent->isPointer) {
									opcode_r_m(irsOut, _MOVP_R_M, REG_DEST_POINTER, destIdentIndex);
									opcode_mr_r(irsOut, _BMOV_BR_R, REG_DEST_POINTER, srcReg);
								} else {
									opcode_m_r(irsOut, _BMOV_BM_R, destIdentIndex, srcReg);
								}
							} else {
								opcode_r_r(irsOut, _MOV_R_R, REG_DEST_POINTER, REG_BASE_POINTER);
								opcode_r_immi(irsOut, _ADD_R_IMMI, REG_DEST_POINTER, destIdentIndex);
								if (destIdent->isPointer) {
									opcode_r_mr(irsOut, _MOVP_R_MR, REG_DEST_POINTER, REG_DEST_POINTER);
								}
								opcode_mr_r(irsOut, _BMOV_BR_R, REG_DEST_POINTER, srcReg);
							}
						}
						break;
					case DATA_TYPE_FLOAT:
						opcode_r_fr(irsOut, _FCON_R_FR, 0, srcReg);
						if (isDestIdentIndexNotValid) {
							opcode_mr_r(irsOut, _BMOV_BR_R, REG_DEST_POINTER, 0);
						} else {
							if (isDestIdentGlobal) {
								if (destIdent->isPointer) {
									opcode_r_m(irsOut, _MOVP_R_M, REG_DEST_POINTER, destIdentIndex);
									opcode_mr_r(irsOut, _BMOV_BR_R, REG_DEST_POINTER, 0);
								} else {
									opcode_m_r(irsOut, _BMOV_BM_R, destIdentIndex, 0);
								}
							} else {
								opcode_r_r(irsOut, _MOV_R_R, REG_DEST_POINTER, REG_BASE_POINTER);
								opcode_r_immi(irsOut, _ADD_R_IMMI, REG_DEST_POINTER, destIdentIndex);
								if (destIdent->isPointer) {
									opcode_r_mr(irsOut, _MOVP_R_MR, REG_DEST_POINTER, REG_DEST_POINTER);
								}
								opcode_mr_r(irsOut, _BMOV_BR_R, REG_DEST_POINTER, 0);
							}
						}
						break;
					default:
						{
							string errMsg = ERR_STR_TYPE_MISMATCH;
							throw LockableException(errMsg);
						}
				}
				break;
			case DATA_TYPE_INT:
				switch (srcDataType) {
					case DATA_TYPE_BYTE:
					case DATA_TYPE_INT:
						if (isDestIdentIndexNotValid) {
							opcode_mr_r(irsOut, _MOV_MR_R, REG_DEST_POINTER, srcReg);
						} else {
							if (isDestIdentGlobal) {
								if (destIdent->isPointer) {
									opcode_r_m(irsOut, _MOVP_R_M, REG_DEST_POINTER, destIdentIndex);
									opcode_mr_r(irsOut, _MOV_MR_R, REG_DEST_POINTER, srcReg);
								} else {
									opcode_m_r(irsOut, _MOV_M_R, destIdentIndex, srcReg);
								}
							} else {
								opcode_r_r(irsOut, _MOV_R_R, REG_DEST_POINTER, REG_BASE_POINTER);
								opcode_r_immi(irsOut, _ADD_R_IMMI, REG_DEST_POINTER, destIdentIndex);
								if (destIdent->isPointer) {
									opcode_r_mr(irsOut, _MOVP_R_MR, REG_DEST_POINTER, REG_DEST_POINTER);
								}
								opcode_mr_r(irsOut, _MOV_MR_R, REG_DEST_POINTER, srcReg);
							}
						}
						break;
					case DATA_TYPE_FLOAT:
						if (isDestIdentIndexNotValid) {
							opcode_r_fr(irsOut, _FCON_R_FR, 0, srcReg);
							opcode_mr_r(irsOut, _MOV_MR_R, REG_DEST_POINTER, 0);
						} else {
							if (isDestIdentGlobal) {
								if (destIdent->isPointer) {
									opcode_r_m(irsOut, _MOVP_R_M, REG_DEST_POINTER, destIdentIndex);
									opcode_r_fr(irsOut, _FCON_R_FR, 0, srcReg);
									opcode_mr_r(irsOut, _MOV_MR_R, REG_DEST_POINTER, 0);
								} else {
									opcode_r_fr(irsOut, _FCON_R_FR, 0, srcReg);
									opcode_m_r(irsOut, _MOV_M_R, destIdentIndex, 0);
								}
							} else {
								opcode_r_r(irsOut, _MOV_R_R, REG_DEST_POINTER, REG_BASE_POINTER);
								opcode_r_immi(irsOut, _ADD_R_IMMI, REG_DEST_POINTER, destIdentIndex);
								if (destIdent->isPointer) {
									opcode_r_mr(irsOut, _MOVP_R_MR, REG_DEST_POINTER, REG_DEST_POINTER);
								}
								opcode_r_fr(irsOut, _FCON_R_FR, 0, srcReg);
								opcode_mr_r(irsOut, _MOV_MR_R, REG_DEST_POINTER, 0);
							}
						}
						break;
					default:
						{
							string errMsg = ERR_STR_TYPE_MISMATCH;
							throw LockableException(errMsg);
						}
				}
				break;
			case DATA_TYPE_FLOAT:
				if (srcIsCompare) {
					string errMsg = ERR_STR_TYPE_MISMATCH;
					throw LockableException(errMsg.c_str());
				}
				switch (srcDataType) {
					case DATA_TYPE_BYTE:
					case DATA_TYPE_INT:
						if (isDestIdentIndexNotValid) {
							opcode_fr_r(irsOut, _FCON_FR_R, 0, srcReg);
							opcode_mfr_fr(irsOut, _FMOV_MFR_FR, REG_DEST_POINTER, 0);
						} else {
							if (isDestIdentGlobal) {
								if (destIdent->isPointer) {
									opcode_r_m(irsOut, _MOVP_R_M, REG_DEST_POINTER, destIdentIndex);
									opcode_fr_r(irsOut, _FCON_FR_R, 0, srcReg);
									opcode_mfr_fr(irsOut, _FMOV_MFR_FR, REG_DEST_POINTER, 0);
								} else {
									opcode_fr_r(irsOut, _FCON_FR_R, 0, srcReg);
									opcode_mfr_fr(irsOut, _FMOV_FM_FR, destIdentIndex, 0);
								}
							} else {
								opcode_r_r(irsOut, _MOV_R_R, REG_DEST_POINTER, REG_BASE_POINTER);
								opcode_r_immi(irsOut, _ADD_R_IMMI, REG_DEST_POINTER, destIdentIndex);
								if (destIdent->isPointer) {
									opcode_r_mr(irsOut, _MOVP_R_MR, REG_DEST_POINTER, REG_DEST_POINTER);
								}
								opcode_fr_r(irsOut, _FCON_FR_R, 0, srcReg);
								opcode_mfr_fr(irsOut, _FMOV_MFR_FR, REG_DEST_POINTER, 0);
							}
						}
						break;
					case DATA_TYPE_FLOAT:
						if (isDestIdentIndexNotValid) {
							opcode_mfr_fr(irsOut, _FMOV_MFR_FR, REG_DEST_POINTER, srcReg);
						} else {
							if (isDestIdentGlobal) {
								if (destIdent->isPointer) {
									opcode_r_m(irsOut, _MOVP_R_M, REG_DEST_POINTER, destIdentIndex);
									opcode_mfr_fr(irsOut, _FMOV_MFR_FR, REG_DEST_POINTER, srcReg);
								} else {
									opcode_fm_fr(irsOut, _FMOV_FM_FR, destIdentIndex, srcReg);
								}
							} else {
								opcode_r_r(irsOut, _MOV_R_R, REG_DEST_POINTER, REG_BASE_POINTER);
								opcode_r_immi(irsOut, _ADD_R_IMMI, REG_DEST_POINTER, destIdentIndex);
								if (destIdent->isPointer) {
									opcode_r_mr(irsOut, _MOVP_R_MR, REG_DEST_POINTER, REG_DEST_POINTER);
								}
								opcode_mfr_fr(irsOut, _FMOV_MFR_FR, REG_DEST_POINTER, srcReg);
							}
						}
						break;
					default:
						{
							string errMsg = ERR_STR_TYPE_MISMATCH;
							throw LockableException(errMsg);
						}
				}
				break;
			case DATA_TYPE_STRING:
				if (srcDataType <= DATA_TYPE_STRING) {
					if (isDestIdentIndexNotValid) {
						opcode_r_r(irsOut, _MOV_R_R, REG_STRING_POINTER, REG_DEST_POINTER);
					} else {
						opcode_r_r(irsOut, _MOV_R_R, REG_STRING_POINTER, (isDestIdentGlobal) ? REG_GLOBAL_BASE_POINTER : REG_BASE_POINTER);
						opcode_r_immi(irsOut, _ADD_R_IMMI, REG_STRING_POINTER, destIdentIndex);
						if (destIdent->isPointer) {
							opcode_r_mr(irsOut, _MOVP_R_MR, REG_STRING_POINTER, REG_STRING_POINTER);
						}
					}
					opcode_r_mr(irsOut, _MOVP_R_MR, REG_STRING_POINTER, REG_STRING_POINTER);
					parseString(&srcExpression, 0, 2, 1, function->identsSize, Token(TOKEN_OPERATOR, KW_NONE, OPERATOR_SEMI_COLON, ";"),
						irsOut);
				} else {
					string errMsg = ERR_STR_TYPE_MISMATCH;
					throw LockableException(errMsg);
				}
				break;
			default:
				if (srcDataType == destIdent->dataType && srcDataTypeStr == destIdent->dataTypeStr) {
					//Get destination address
					if (!isDestIdentIndexNotValid) {
						opcode_r_r(irsOut, _MOV_R_R, REG_DEST_POINTER, (isDestIdentGlobal) ? REG_GLOBAL_BASE_POINTER : REG_BASE_POINTER);
						opcode_r_immi(irsOut, _ADD_R_IMMI, REG_DEST_POINTER, destIdentIndex);
						if (destIdent->dataType != DATA_TYPE_GROUP && destIdent->isPointer) {
							opcode_r_mr(irsOut, _MOVP_R_MR, REG_DEST_POINTER, REG_DEST_POINTER);
						}
					}

					const int REG_SRC_POINTER = 2;

					//Get source address
					if (srcExpression.tokens.at(1).operation == OPERATOR_OPEN_ROUND_BRACKET) { //Function call
						//Get Function call tokens
						TokenLine functionCallTokens;
						uint32 srcCounter = 0;
						getFunctionCall(&srcExpression, srcCounter, &functionCallTokens);

						//Get the function call IRs
						opcode_r(irsOut, _PUSH_R, REG_DEST_POINTER);
						const Function *calledFunction = functionCall(&functionCallTokens, function->identsSize, 1, 1, irsOut);
						opcode_r(irsOut, _POP_R, REG_DEST_POINTER);
						opcode_r_r(irsOut, _MOV_R_R, REG_SRC_POINTER, 0);

						//Check if the return value is accessing a group member
						if (srcExpression.tokens.at(++srcCounter).operation == OPERATOR_PERIOD) {
							//Get the group member address
							list<IntermediateRepresentation> groupMemberAddr;
							bool isGrpMemIndexNotValid;
							uint32 grpMemIndex = 0;
							const Ident *groupMember = getGroupMember(&srcExpression, srcCounter, calledFunction->returnTypeStr,
								REG_SRC_POINTER, 3, &groupMemberAddr, &grpMemIndex, &isGrpMemIndexNotValid);

							if (isGrpMemIndexNotValid) {
								irsOut->splice(irsOut->end(), groupMemberAddr);
							} else {
								opcode_r_immi(irsOut, _ADD_R_IMMI, REG_SRC_POINTER, grpMemIndex);
							}
						}
					} else {
						//Get the source ident address
						list<IntermediateRepresentation> srcIdentAddr;
						bool isSrcIdentIndexNotValid, isSrcIdentGlobal;
						uint32 srcIdentIndex = 0, srcCounter = 0;
						const Ident *srcIdent = getIdent(&srcExpression, srcCounter, REG_SRC_POINTER, &srcIdentAddr, &srcIdentIndex,
							&isSrcIdentIndexNotValid, &isSrcIdentGlobal, 3);

						if (isSrcIdentIndexNotValid) {
							irsOut->splice(irsOut->end(), srcIdentAddr);
						} else {
							opcode_r_r(irsOut, _MOV_R_R, REG_SRC_POINTER, (isSrcIdentGlobal) ? REG_GLOBAL_BASE_POINTER : REG_BASE_POINTER);
							opcode_r_immi(irsOut, _ADD_R_IMMI, REG_SRC_POINTER, srcIdentIndex);
							if (srcIdent->isPointer) {
								opcode_r_mr(irsOut, _MOVP_R_MR, REG_SRC_POINTER, REG_SRC_POINTER);
							}
						}
					}

					if (destIdent->dataType == DATA_TYPE_GROUP) {
						//DMA trasfer from src group to destination group
						dmaTransfer(REG_SRC_POINTER, REG_DEST_POINTER, getDataTypeSize(destIdent->dataType, destIdent->dataTypeStr), irsOut);
					} else {
						opcode_r_mr(irsOut, _MOVP_R_MR, REG_SRC_POINTER, REG_SRC_POINTER);
						opcode_mr_r(irsOut, _MOVP_MR_R, REG_DEST_POINTER, REG_SRC_POINTER);
					}
				} else {
					string errMsg = ERR_STR_TYPE_MISMATCH;
					throw LockableException(errMsg);
				}
		}
	}
}

void pointerAssignment(const TokenLine *tokenLine, list<IntermediateRepresentation> *irsOut) {
	uint32 tokenCounter = 0;
	bool isStronglyTyped = (tokenLine->tokens.at(tokenCounter++).operation == OPERATOR_ADD) ? true : false;

	//Get the destination pointer
	uint32 destIndex = 0;
	bool isDestIndexNotValid, isDestGlobal;
	list<IntermediateRepresentation> destPointerAddr;
	const Ident *destPointer = getIdent(tokenLine, tokenCounter, 1, &destPointerAddr, &destIndex, &isDestIndexNotValid, &isDestGlobal, 2);

	if (!destPointer->isPointer) {
		throw LockableException(ERR_STR_DEST_NOT_POINTER);
	}
	//If the ident was indexed, we can't assign a pointer to it
	if (tokenLine->tokens.at(tokenCounter).operation == OPERATOR_CLOSE_SQUARE_BRACKET) {
		throw LockableException(ERR_STR_DEST_NOT_POINTER);
	}

	//Get the destination pointer address
	if (isDestIndexNotValid) {
		//We don't need the value inside the ident, but the address of the ident. So, if the last opcode was MOVP_R_MR then delete it
		list<IntermediateRepresentation>::iterator it = destPointerAddr.end();
		--it;
		if (it->opcode == _MOVP_R_MR) {
			destPointerAddr.erase(it);
		}

		irsOut->splice(irsOut->end(), destPointerAddr);
	} else {
		opcode_r_r(irsOut, _MOV_R_R, 1, (isDestGlobal) ? REG_GLOBAL_BASE_POINTER : REG_BASE_POINTER);
		opcode_r_immi(irsOut, _ADD_R_IMMI, 1, destIndex);
	}

	//Syntax check
	++tokenCounter;
	if (tokenLine->tokens.at(tokenCounter).operation != OPERATOR_ASSIGNMENT) {
		throw LockableException(ERR_STR_SYNTAX_ERROR);
	}
	++tokenCounter;

	//Now get the source ident
	list<IntermediateRepresentation> srcIdentAddr;
	uint32 srcIdentIndex;
	bool isSrcIdentIndexNotValid, isSrcIdentGlobal;
	const Ident *srcIdent = getIdent(tokenLine, tokenCounter, 2, &srcIdentAddr, &srcIdentIndex, &isSrcIdentIndexNotValid, &isSrcIdentGlobal, 3);

	if (isStronglyTyped && srcIdent->dataType != destPointer->dataType) {
		throw LockableException(ERR_STR_POINTER_TYPE_MISMATCH);
	}

	if (isSrcIdentIndexNotValid) {
		irsOut->splice(irsOut->end(), srcIdentAddr);
	} else {
		opcode_r_r(irsOut, _MOV_R_R, 2, (isDestGlobal) ? REG_GLOBAL_BASE_POINTER : REG_BASE_POINTER);
		opcode_r_immi(irsOut, _ADD_R_IMMI, 2, srcIdentIndex);
		if (srcIdent->isPointer) {
			opcode_r_mr(irsOut, _MOVP_R_MR, 2, 2);
		}
	}

	//Special treatment for image pointers being converted into another type (this is a bit of a hack)
	if (srcIdent->dataType == DATA_TYPE_IMAGE && destPointer->dataType != DATA_TYPE_IMAGE) {
		opcode_r_mr(irsOut, _MOVP_R_MR, 2, 2);
	} else if (srcIdent->dataType != DATA_TYPE_IMAGE && destPointer->dataType == DATA_TYPE_IMAGE) {
		if (srcIdent->isPointer) {
			list<IntermediateRepresentation>::iterator it = irsOut->end();
			uint16 opcode;
			do {
				--it;
				opcode = it->opcode;
				irsOut->erase(it);
			} while (opcode != _MOVP_R_MR);
		}
	}

	//Assignment
	opcode_mr_r(irsOut, _MOVP_MR_R, 1, 2);
}

IdentifierDataType memoryAllocation(const TokenLine *allocationLine, string *dataTypeStrOut, list<IntermediateRepresentation> *irsOut) {
	//Get type size
	int typeSize;
	IdentifierDataType dataType;
	typeSize = IdentDataType::kwToDataType(allocationLine->tokens.at(1).keyword, dataType);
	if (dataType == DATA_TYPE_GROUP) {
		const Group *group = getGroup(allocationLine->tokens.at(1).value);
		typeSize = group->size;
		*dataTypeStrOut = group->name;
	}

	//Get the allocation size
	uint32 tokenCounter = 3;
	TokenLine allocationSizeTokens;

	getExpressionInsideBracket(allocationLine, tokenCounter, OPERATOR_OPEN_SQUARE_BRACKET, OPERATOR_CLOSE_SQUARE_BRACKET, &allocationSizeTokens);
	allocationSizeTokens.tokens.push_back(Token(TOKEN_OPERATOR, KW_NONE, OPERATOR_SEMI_COLON, ";"));

	bool isCompare;
	IdentifierDataType sizeDataType = getExpressionDataType(&allocationSizeTokens, 0, isCompare);
	if (isCompare || (sizeDataType != DATA_TYPE_INT && sizeDataType != DATA_TYPE_BYTE)) {
		throw LockableException(ERR_STR_ALLOC_SIZE);
	}

	int allocSizeReg = parseExpression(&allocationSizeTokens, sizeDataType, Token(TOKEN_OPERATOR, KW_NONE, OPERATOR_SEMI_COLON, ";"),
		function->identsSize, 2, 1, false, irsOut);

	//Multiply the type size with allocation size
	opcode_r_immi(irsOut, _MUL_R_IMMI, allocSizeReg, typeSize);

	//Allocate the memory
	opcode_immi_r(irsOut, _OUT_IMMI_R64, 6, allocSizeReg);
	opcode_r_immi(irsOut, _IN_R64_IMMI, 0, 6);

	return dataType;
}

void memoryDeallocation(const TokenLine *deallocationLine, list<IntermediateRepresentation> *irsOut) {
	//Get the ident to deallocate
	uint32 offset = 1, identIndex;
	bool identIndexNotValid, isGlobal;
	list<IntermediateRepresentation> identAddress;
	const Ident *ident = getIdent(deallocationLine, offset, 1, &identAddress, &identIndex, &identIndexNotValid, &isGlobal, 2);

	if (!ident->isPointer) {
		throw LockableException(ERR_STR_DEALLOC_NOT_A_POINTER);
	}

	if (identIndexNotValid) {
		irsOut->splice(irsOut->end(), identAddress);
	} else {
		if (isGlobal) {
			opcode_r_m(irsOut, _MOVP_R_M, 1, identIndex);
		} else {
			opcode_r_r(irsOut, _MOV_R_R, 1, REG_BASE_POINTER);
			opcode_r_immi(irsOut, _ADD_R_IMMI, 1, identIndex);
			opcode_r_mr(irsOut, _MOVP_R_MR, 1, 1);
		}
	}

	//Deallocate the ident
	opcode_immi_r(irsOut, _OUT_IMMI_R64, 14, 1);
}

//tokenOffset should point to the token AFTER the first open round bracket
void compare(const vector<TokenLine> *tokenLines, uint32 tokenOffset, uint32 &lineOffset, int lblEnd, list<IntermediateRepresentation> *irsOut, int lblBreak, int lblContinue) {
	//Get the comparision expression
	TokenLine compareExpression;
	int bracketLevel = 0;
	while (true) {
		if (tokenLines->at(lineOffset).tokens.at(tokenOffset).operation == OPERATOR_OPEN_ROUND_BRACKET) {
			++bracketLevel;
		} else if (tokenLines->at(lineOffset).tokens.at(tokenOffset).operation == OPERATOR_CLOSE_ROUND_BRACKET) {
			--bracketLevel;
			if (bracketLevel < 0) break;
		}
		compareExpression.tokens.push_back(tokenLines->at(lineOffset).tokens.at(tokenOffset++));
	}
	compareExpression.tokens.push_back(Token(TOKEN_OPERATOR, KW_NONE, OPERATOR_SEMI_COLON, ";")); //Terminator

	//Get the data type of the comparision expression
	bool isCompare;
	IdentifierDataType dataType = getExpressionDataType(&compareExpression, 0, isCompare);

	//Error checking
	switch (dataType) {
		case DATA_TYPE_BYTE:
		case DATA_TYPE_INT:
			break;
		case DATA_TYPE_FLOAT:
			if (!isCompare) {
				string errMsg = ERR_STR_INVALID_CMP_DATA;
				throw LockableException(errMsg);
			}
			break;
		default:
			{
				string errMsg = ERR_STR_INVALID_CMP_DATA;
				throw LockableException(errMsg);
			}
	}

	//Parse the compare expression
	int reg = parseExpression(&compareExpression, dataType, Token(TOKEN_OPERATOR, KW_NONE, OPERATOR_SEMI_COLON, ";"), function->identsSize, 1, 1, true, irsOut);

	//Jump to this label is condition is not met
	int lblOut = label++;
	opcode_r_label(irsOut, _JC_R_IMMI, reg, lblOut);

	//Parse body
	parseLines(tokenLines, ++lineOffset, irsOut, lblBreak, lblContinue);

	//After the body part is excuted, jump to this label
	opcode_label(irsOut, _JMP_IMMI, lblEnd);

	//Put the labelOut at the end
	opcode(irsOut, _NOP, lblOut);
}

void lineReturn(const TokenLine *tokenLine, list<IntermediateRepresentation> *irsOut) {
	//If line is returning null
	if (tokenLine->tokens.at(1).type == TOKEN_NUMBER && tokenLine->tokens.at(1).value == "0") {
		opcode_r_immi(irsOut, _MOV_R_IMMI, 0, 0);
		opcode_label(irsOut, _JMP_IMMI, functionEndLabel);
		return;
	}

	//Get expression part
	TokenLine expression;
	uint32 counter = 1;
	do {
		expression.tokens.push_back(tokenLine->tokens.at(counter));
	} while (tokenLine->tokens.at(counter++).operation != OPERATOR_SEMI_COLON);

	//Get the data type of the expression
	bool expIsCompare;
	string expDataTypeStr;
	IdentifierDataType expDataType = getExpressionDataType(&expression, &expDataTypeStr, expIsCompare);

	Token terminator(TOKEN_OPERATOR, KW_NONE, OPERATOR_SEMI_COLON, ";");
	switch (function->returnType) {
		case DATA_TYPE_BYTE:
		case DATA_TYPE_INT:
			switch (expDataType) {
				case DATA_TYPE_BYTE:
				case DATA_TYPE_INT:
				case DATA_TYPE_FLOAT:
					{
						uint32 reg = parseExpression(&expression, expDataType, terminator, function->identsSize, 1, 1, expIsCompare, irsOut);
						if (expDataType == DATA_TYPE_FLOAT && !expIsCompare) {
							opcode_r_fr(irsOut, _FCON_R_FR, 0, reg);
						} else {
							opcode_r_r(irsOut, _MOV_R_R, 0, reg);
						}
					}
					break;
				default:
					{
						string errMsg = ERR_STR_RET_TYPE_MISMATCH;
						throw LockableException(errMsg);
					}
				}
			break;
		case DATA_TYPE_FLOAT:
			if (expIsCompare) {
				string errMsg = ERR_STR_RET_TYPE_MISMATCH;
				throw LockableException(errMsg);
			}
			switch (expDataType) {
				case DATA_TYPE_BYTE:
				case DATA_TYPE_INT:
				case DATA_TYPE_FLOAT:
					{
						uint32 reg = parseExpression(&expression, expDataType, terminator, function->identsSize, 1, 1, false, irsOut);
						if (expDataType == DATA_TYPE_FLOAT) {
							opcode_fr_fr(irsOut, _FMOV_FR_FR, 0, reg);
						} else {
							opcode_fr_r(irsOut, _FCON_FR_R, 0, reg);
						}
					}
					break;
				default:
					{
						string errMsg = ERR_STR_RET_TYPE_MISMATCH;
						throw LockableException(errMsg);
					}
				}
			break;
		case DATA_TYPE_STRING:
			switch (expDataType) {
				case DATA_TYPE_BYTE:
				case DATA_TYPE_INT:
				case DATA_TYPE_FLOAT:
				case DATA_TYPE_STRING:
					{
						//Get that extra string object we created in symbol table creation if the return value was string
						TokenLine dummyLine;
						dummyLine.tokens.push_back(Token(TOKEN_IDENTIFIER, KW_NONE, OPERATOR_NONE, ""));
						dummyLine.tokens.push_back(Token(TOKEN_OPERATOR, KW_NONE, OPERATOR_SEMI_COLON, ";"));
						uint32 dummyOffset = 0;
						const Ident *ident = getIdent(&dummyLine, dummyOffset, 1, 0, 0, 0, 0, 0);

						//Copy the ident address into the 'String Pointer Register'
						opcode_r_r(irsOut, _MOV_R_R, REG_INDEX_POINTER, REG_BASE_POINTER);
						opcode_r_immi(irsOut, _ADD_R_IMMI, REG_INDEX_POINTER, ident->index);
						opcode_r_mr(irsOut, _MOVP_R_MR, REG_STRING_POINTER, REG_INDEX_POINTER);

						//Parse the string expression
						parseString(&expression, 0, 1, 1, function->identsSize, terminator, irsOut);

						//Copy the string from 'String Pointer Register' to 'Return String Register'
						opcode_immi_r(irsOut, _OUT_IMMI_R64, 40, REG_RETURN_STRING);
						opcode_immi_r(irsOut, _OUT_IMMI_R64, 48, REG_STRING_POINTER);
						opcode_immi_immi8(irsOut, _OUT_IMMI_IMMI8, 64, 12);
					}
					break;
				default:
					{
						string errMsg = ERR_STR_RET_TYPE_MISMATCH;
						throw LockableException(errMsg);
					}
			}
			break;
		case DATA_TYPE_FILE:
		case DATA_TYPE_IMAGE:
		case DATA_TYPE_GROUP:
			if (expDataType == function->returnType && expDataTypeStr == function->returnTypeStr) {
				//Get the ident
				uint32 dummyOffset = 0, identIndex;
				list<IntermediateRepresentation> identAddrIRs;
				bool isIdentIndexNotValid, isIdentGlobal;
				const Ident *ident = getIdent(&expression, dummyOffset, 0, &identAddrIRs, &identIndex, &isIdentIndexNotValid, &isIdentGlobal, 1);

				if (isIdentIndexNotValid) {
					irsOut->splice(irsOut->end(), identAddrIRs);
				} else {
					if (isIdentGlobal) {
						opcode_r_r(irsOut, _MOV_R_R, 0, REG_GLOBAL_BASE_POINTER);
						opcode_r_immi(irsOut, _ADD_R_IMMI, 0, identIndex);
					} else {
						opcode_r_r(irsOut, _MOV_R_R, 0, REG_BASE_POINTER);
						opcode_r_immi(irsOut, _ADD_R_IMMI, 0, identIndex);
					}
					if (ident->isPointer) {
						opcode_r_mr(irsOut, _MOVP_R_MR, 0, 0);
					}
				}
			} else {
				string errMsg = ERR_STR_RET_TYPE_MISMATCH;
				throw LockableException(errMsg);
			}
			break;
		default:
			if (expDataType != DATA_TYPE_VOID) {
				string errMsg = ERR_STR_RET_TYPE_MISMATCH;
				throw LockableException(errMsg);
			}
			break;
	}

	//Jump to the end of the function
	opcode_label(irsOut, _JMP_IMMI, functionEndLabel);
}

void ifElseStatement(const vector<TokenLine> *tokenLines,uint32 &lineOffset, list<IntermediateRepresentation> *irsOut, int lblBreak, int lblContinue) {
	//Separate a label for goting out of this 'if else' statement
	int lblOut = label++;

	//Parse the first if block
	compare(tokenLines, 2, lineOffset, lblOut, irsOut, lblBreak, lblContinue);

	//Check if the blocks that follow after are else blocks
	while(tokenLines->at(lineOffset).tokens.at(0).keyword == KW_ELSE) {
		//Check if it is a 'else if' block or an 'else' block
		if (tokenLines->at(lineOffset).tokens.at(1).keyword == KW_IF) {
			compare(tokenLines, 3, lineOffset, lblOut, irsOut, lblBreak, lblContinue);
		} else {
			parseLines(tokenLines, ++lineOffset, irsOut, lblBreak, lblContinue);
		}
	}

	//Put the lblOut at the end
	opcode(irsOut, _NOP, lblOut);
}

void whileLoop(const vector<TokenLine> *tokenLines,uint32 &lineOffset, list<IntermediateRepresentation> *irsOut) {
	//Separate a label for iterating through the body until the condition is met
	int lblLoop = label++;
	//Another label to mark the end of the loop
	int lblBreak = label++;

	//Put the lblLoop on top of the iteration
	opcode(irsOut, _NOP, lblLoop);

	//Parse the loop block
	compare(tokenLines, 2, lineOffset, lblLoop, irsOut, lblBreak, lblLoop);

	opcode(irsOut, _NOP, lblBreak);
}

void startThread(const TokenLine *tokenLine, list<IntermediateRepresentation> *irsOut) {
	//Get the function
	TokenLine functionTokens;
	uint32 offset = 1;
	getFunctionCall(tokenLine, offset, &functionTokens);
	Function *threadFunction = matchFunctionCall(&functionTokens, 0);
	threadFunction->isCalled = true;

	//Check the function definition syntax
	if (threadFunction->parameters.size() > 1 || threadFunction->returnType != DATA_TYPE_VOID) {
		throw LockableException(ERR_STR_THREAD_DEFINITION);
	}
	if (threadFunction->parameters.size()) {
		if (threadFunction->parameters[0].dataType != DATA_TYPE_BYTE && !threadFunction->parameters[0].isPointer) {
			throw LockableException(ERR_STR_THREAD_DEFINITION);
		}

		//Get the parameter
		offset = 3;
		list<IntermediateRepresentation> paramAddr;
		uint32 paramIndex;
		bool isIndexNotValid, isGlobal;
		const Ident *ident = getIdent(tokenLine, offset, 1, &paramAddr, &paramIndex, &isIndexNotValid, &isGlobal, 2);

		if (isIndexNotValid) {
			irsOut->splice(irsOut->end(), paramAddr);
		} else {
			if (isGlobal) {
				opcode_r_m(irsOut, _MOVP_R_M, 1, paramIndex);
			} else {
				opcode_r_r(irsOut, _MOV_R_R, 1, REG_BASE_POINTER);
				opcode_r_immi(irsOut, _ADD_R_IMMI, 1, paramIndex);
				opcode_r_mr(irsOut, _MOVP_R_MR, 1, 1);
			}
		}

		opcode_immi_r(irsOut, _OUT_IMMI_R64, PORT_THREAD_PARAMETER, 1);
	} else {
		opcode_immi_immi64(irsOut, _OUT_IMMI_IMMI64, PORT_THREAD_PARAMETER, 0);
	}

	opcode_immi_function(irsOut, _OUT_IMMI_IMMI32, PORT_THREAD_CREATE, threadFunction->functionID, threadFunction->name);
}

const Function *functionCall(const TokenLine *functionCallTokens, int paramMemOffset, int intRegOffset, int floatRegOffset, list<IntermediateRepresentation> *irsOut) {
	vector<TokenLine> params;
	Function *calledFunction = matchFunctionCall(functionCallTokens, &params);
	calledFunction->isCalled = true;

	//Put values in parameter
	for (uint32 i = 0; i < calledFunction->parameters.size(); ++i) {
		if (calledFunction->parameters.at(i).isPointer) {
			//Get the ident
			uint32 dummyOffset = 0, identIndex;
			list<IntermediateRepresentation> identAddrIRs;
			bool isIdentIndexNotValid, isIdentGlobal;
			const Ident *ident = getIdent(&params.at(i), dummyOffset, intRegOffset + 1, &identAddrIRs, &identIndex, &isIdentIndexNotValid, &isIdentGlobal, intRegOffset + 2);

			//Syntax check
			if (params.at(i).tokens.at(++dummyOffset).operation != OPERATOR_SEMI_COLON) {
				throw LockableException(ERR_STR_SYNTAX_ERROR);
			}

			//Get the parameter address
			opcode_r_r(irsOut, _MOV_R_R, intRegOffset, REG_BASE_POINTER);
			opcode_r_immi(irsOut, _ADD_R_IMMI, intRegOffset, paramMemOffset + calledFunction->parameters.at(i).index);

			//Get the Ident Address
			if (isIdentIndexNotValid) {
				irsOut->splice(irsOut->end(), identAddrIRs);
			} else {
				opcode_r_r(irsOut, _MOV_R_R, intRegOffset + 1, (isIdentGlobal) ? REG_GLOBAL_BASE_POINTER : REG_BASE_POINTER);
				opcode_r_immi(irsOut, _ADD_R_IMMI, intRegOffset + 1, identIndex);
				if (ident->isPointer) {
					opcode_r_mr(irsOut, _MOVP_R_MR, intRegOffset + 1, intRegOffset + 1);
				}
			}

			opcode_mr_r(irsOut, _MOVP_MR_R, intRegOffset, intRegOffset + 1);
		} else {
			switch (calledFunction->parameters.at(i).dataType) {
				case DATA_TYPE_BYTE:
				case DATA_TYPE_FLOAT:
				case DATA_TYPE_INT:
					{
						//Get parameter expression IRs
						int reg = parseExpression(&params.at(i), calledFunction->parameters.at(i).dataType,
							Token(TOKEN_OPERATOR, KW_NONE, OPERATOR_SEMI_COLON, ";"), paramMemOffset + calledFunction->identsSize, intRegOffset, floatRegOffset, false, irsOut);

						//Get the address of parameter
						opcode_r_r(irsOut, _MOV_R_R, REG_INDEX_POINTER, REG_BASE_POINTER);
						opcode_r_immi(irsOut, _ADD_R_IMMI, REG_INDEX_POINTER, paramMemOffset + calledFunction->parameters.at(i).index);

						//Put the result value in parameter
						switch (calledFunction->parameters.at(i).dataType) {
							case DATA_TYPE_BYTE:
								opcode_mr_r(irsOut, _BMOV_BR_R, REG_INDEX_POINTER, reg);
								break;
							case DATA_TYPE_INT:
								opcode_mr_r(irsOut, _MOV_MR_R, REG_INDEX_POINTER, reg);
								break;
							case DATA_TYPE_FLOAT:
								opcode_mfr_fr(irsOut, _FMOV_MFR_FR, REG_INDEX_POINTER, reg);
						}
						break;
					}
				case DATA_TYPE_STRING:
					//Allocate a new string object
					opcode_immi_immi8(irsOut, _OUT_IMMI_IMMI8, 64, 0);
					opcode_r_immi(irsOut, _IN_R64_IMMI, 0, 40);

					//Put the newly created string object 'String Pointer Register'
					opcode_r_r(irsOut, _MOV_R_R, REG_STRING_POINTER, 0);

					//Parse String expression
					parseString(&params.at(i), 0, intRegOffset, floatRegOffset, paramMemOffset + calledFunction->identsSize, Token(TOKEN_OPERATOR, KW_NONE, OPERATOR_SEMI_COLON, ";"), irsOut);
					
					//Get the address of parameter
					opcode_r_r(irsOut, _MOV_R_R, REG_INDEX_POINTER, REG_BASE_POINTER);
					opcode_r_immi(irsOut, _ADD_R_IMMI, REG_INDEX_POINTER, paramMemOffset + calledFunction->parameters.at(i).index);

					//Put the string pointer string in parameter
					opcode_mr_r(irsOut, _MOVP_MR_R, REG_INDEX_POINTER, REG_STRING_POINTER);
					break;
				default:
					{
						uint32 dummyOffset = 0, identIndex;
						list<IntermediateRepresentation> identAddrIRs;
						bool isIdentIndexNotValid, isIdentGlobal;
						const Ident *ident = getIdent(&params.at(i), dummyOffset, intRegOffset + 1, &identAddrIRs, &identIndex, &isIdentIndexNotValid, &isIdentGlobal, intRegOffset + 2);

						//Get the parameter address
						opcode_r_r(irsOut, _MOV_R_R, intRegOffset, REG_BASE_POINTER);
						opcode_r_immi(irsOut, _ADD_R_IMMI, intRegOffset, paramMemOffset + calledFunction->parameters.at(i).index);

						//Get the Ident Address
						if (isIdentIndexNotValid) {
							irsOut->splice(irsOut->end(), identAddrIRs);
						} else {
							opcode_r_r(irsOut, _MOV_R_R, intRegOffset + 1, (isIdentGlobal) ? REG_GLOBAL_BASE_POINTER : REG_BASE_POINTER);
							opcode_r_immi(irsOut, _ADD_R_IMMI, intRegOffset + 1, identIndex);
							if (ident->isPointer) {
								opcode_r_mr(irsOut, _MOVP_R_MR, intRegOffset + 1, intRegOffset + 1);
							}
						}

						if (calledFunction->parameters.at(i).dataType == DATA_TYPE_GROUP) {
							//Put the ident value into the parameter
							dmaTransfer(intRegOffset + 1, intRegOffset, ident->size, irsOut);
						} else {
							opcode_r_mr(irsOut, _MOVP_R_MR, intRegOffset + 1, intRegOffset + 1);
							opcode_mr_r(irsOut, _MOVP_MR_R, intRegOffset, intRegOffset + 1);
						}
					}
			}
		}
	}
	//Increment the base pointer
	opcode_r_immi(irsOut, _ADD_R_IMMI, REG_BASE_POINTER, paramMemOffset);

	//Call the function
	opcode_function(irsOut, _CALL_IMMI, calledFunction->functionID, calledFunction->name);

	//Decrement the base pointer
	opcode_r_immi(irsOut, _SUB_R_IMMI, REG_BASE_POINTER, paramMemOffset);

	return calledFunction;
}