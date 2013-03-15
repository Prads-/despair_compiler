/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include <cstdlib>
#include "expressionParser.h"
#include "instructionsSet.h"
#include "errorStrings.h"
#include "lockableException.h"
using namespace std;
using namespace IREmitter;

#define OPERATION_TYPE_GENERAL			0
#define OPERATION_TYPE_LOG_AND_OR		1
#define OPERATION_TYPE_COMPARE			2

static void init();
static void loadValToReg();
static void precedenceLevel1();
static void precedenceLevel2();
static void precedenceLevel3();
static void precedenceLevel4();
static void precedenceLevel5();
static void operation(uint16 opcode, int precedence, int type);
static int getRegister(int regCounter, int decrementBy);

static ExpressionToken *terminator;
static uint32 tokenCounter;
static int intRegCounter, floatRegCounter;
static vector<ExpressionToken> *tokens;
static list<IntermediateRepresentation> *irs;
static void (*funcPrecedence[5])();
static bool generateFloatInstructions;	//If true, floating point instructions are generated, else integer instructions are generated

void ExpressionParser::parse(std::list<IntermediateRepresentation> *irOut, std::vector<ExpressionToken> *tks, ExpressionToken parseEnd,
							 int intRegOffset, int floatRegOffset, bool floatExpression) {
	intRegCounter = intRegOffset;
	floatRegCounter = floatRegOffset;
	terminator = &parseEnd;
	tokenCounter = 0;
	tokens = tks;
	irs = irOut;
	generateFloatInstructions = floatExpression;
	funcPrecedence[0] = precedenceLevel1;
	funcPrecedence[1] = precedenceLevel2;
	funcPrecedence[2] = precedenceLevel3;
	funcPrecedence[3] = precedenceLevel4;
	funcPrecedence[4] = precedenceLevel5;
	init();
}

void init() {
	loadValToReg();
	precedenceLevel1();
}

void loadValToReg() {
	if ((*tokens)[tokenCounter].operation == OPERATOR_OPEN_ROUND_BRACKET) {
		++tokenCounter;
		init();
	} else {
		IntermediateRepresentation ir;
		Operand operand;
		ir.address = 0;
		ir.label = 0;
		switch ((*tokens)[tokenCounter].type) {
			case TOKEN_NUMBER:
				if (generateFloatInstructions) {
					ir.opcode = _FCON_FR_IMMI;
					operand.type = OPERAND_FLOAT_REGISTER;
					operand.value = floatRegCounter++;
				} else {
					ir.opcode = _MOV_R_IMMI;
					operand.type = OPERAND_REGISTER;
					operand.value = intRegCounter++;
				}

				ir.operands.push_back(operand);

				operand.type = OPERAND_IMMI;
				operand.value = atoi((*tokens)[tokenCounter].strValue.c_str());
				ir.operands.push_back(operand);
				irs->push_back(ir);
				break;
			case TOKEN_FLOAT_NUMBER:
				if (generateFloatInstructions) {
					ir.opcode = _FMOV_FR_FIMMI;
					operand.type = OPERAND_FLOAT_REGISTER;
					operand.value = floatRegCounter++;
				} else {
					ir.opcode = _FCON_R_FIMMI;
					operand.type = OPERAND_REGISTER;
					operand.value = intRegCounter++;
				}

				ir.operands.push_back(operand);

				operand.type = OPERAND_FIMMI;
				operand.fValue = atof((*tokens)[tokenCounter].strValue.c_str());
				ir.operands.push_back(operand);
				irs->push_back(ir);
				break;
			case TOKEN_IDENTIFIER:
				{
					if (tokens->at(tokenCounter).isGlobalIdent) {
						if (tokens->at(tokenCounter).isPointer) {
							opcode_r_m(irs, _MOVP_R_M, REG_INDEX_POINTER, tokens->at(tokenCounter).value);
						}
						if (generateFloatInstructions) {
							switch (tokens->at(tokenCounter).dataType) {
								case DATA_TYPE_BYTE:
									if (tokens->at(tokenCounter).isPointer) {
										opcode_r_mr(irs, _BMOV_R_BR, intRegCounter, REG_INDEX_POINTER);
									} else {
										opcode_r_m(irs, _BMOV_R_BM, intRegCounter, tokens->at(tokenCounter).value);
									}
									opcode_fr_r(irs, _FCON_FR_R, floatRegCounter++, intRegCounter);
									break;
								case DATA_TYPE_INT:
									if (tokens->at(tokenCounter).isPointer) {
										opcode_r_mr(irs, _MOV_R_MR, intRegCounter, REG_INDEX_POINTER);
										opcode_fr_r(irs, _FCON_FR_R, floatRegCounter++, intRegCounter);
									} else {
										opcode_r_m(irs, _MOV_R_MR, intRegCounter, tokens->at(tokenCounter).value);
										opcode_fr_r(irs, _FCON_FR_R, floatRegCounter++, intRegCounter);
									}
									break;
								case DATA_TYPE_FLOAT:
									if (tokens->at(tokenCounter).isPointer) {
										opcode_fr_mfr(irs, _FMOV_FR_MFR, floatRegCounter++, REG_INDEX_POINTER);
									} else {
										opcode_fr_fm(irs, _FMOV_FR_FM, floatRegCounter++, tokens->at(tokenCounter).value);
									}
							}
						} else {
							switch (tokens->at(tokenCounter).dataType) {
								case DATA_TYPE_BYTE:
									if (tokens->at(tokenCounter).isPointer) {
										opcode_r_mr(irs, _BMOV_R_BR, intRegCounter++, REG_INDEX_POINTER);
									} else {
										opcode_r_m(irs, _BMOV_R_BM, intRegCounter++, tokens->at(tokenCounter).value);
									}
									break;
								case DATA_TYPE_INT:
									if (tokens->at(tokenCounter).isPointer) {
										opcode_r_mr(irs, _MOV_R_MR, intRegCounter++, REG_INDEX_POINTER);
									} else {
										opcode_r_m(irs, _MOV_R_M, intRegCounter++, tokens->at(tokenCounter).value);
									}
									break;
								case DATA_TYPE_FLOAT:
									if (tokens->at(tokenCounter).isPointer) {
										opcode_fr_mfr(irs, _FMOV_FR_MFR, floatRegCounter, REG_INDEX_POINTER);
										opcode_r_fr(irs, _FCON_R_FR, intRegCounter++, floatRegCounter);
									} else {
										opcode_fr_fm(irs, _FMOV_FR_FM, floatRegCounter, tokens->at(tokenCounter).value);
										opcode_r_fr(irs, _FCON_R_FR, intRegCounter++, floatRegCounter);
									}
							}
						}
					} else {
						bool isPointer = false;
						if (tokens->at(tokenCounter).isPointer) {
							opcode_r_mr_immi(irs, _MOVP_R_MR_IMMI, REG_INDEX_POINTER, REG_BASE_POINTER, tokens->at(tokenCounter).value);
							isPointer = true;
						}

						if (generateFloatInstructions) {
							switch (tokens->at(tokenCounter).dataType) {
								case DATA_TYPE_BYTE:
									if (isPointer) {
										opcode_r_mr(irs, _BMOV_R_BR, intRegCounter, REG_INDEX_POINTER);
									} else {
										opcode_r_mr_immi(irs, _BMOV_R_BR_IMMI, intRegCounter, REG_BASE_POINTER, tokens->at(tokenCounter).value);
									}
									opcode_fr_r(irs, _FCON_FR_R, floatRegCounter++, intRegCounter);
									break;
								case DATA_TYPE_INT:
									if (isPointer) {
										opcode_r_mr(irs, _MOV_R_MR, intRegCounter, REG_INDEX_POINTER);
									} else {
										opcode_r_mr_immi(irs, _MOV_R_MR_IMMI, intRegCounter, REG_BASE_POINTER, tokens->at(tokenCounter).value);
									}
									opcode_fr_r(irs, _FCON_FR_R, floatRegCounter++, intRegCounter);
									break;
								case DATA_TYPE_FLOAT:
									if (isPointer) {
										opcode_fr_mfr(irs, _FMOV_FR_MFR, floatRegCounter++, REG_INDEX_POINTER);
									} else {
										opcode_fr_mfr_immi(irs, _FMOV_FR_MFR_IMMI, floatRegCounter++, REG_BASE_POINTER, tokens->at(tokenCounter).value);
									}
							}
						} else {
							switch (tokens->at(tokenCounter).dataType) {
								case DATA_TYPE_BYTE:
									if (isPointer) {
										opcode_r_mr(irs, _BMOV_R_BR, intRegCounter++, REG_INDEX_POINTER);
									} else {
										opcode_r_mr_immi(irs, _BMOV_R_BR_IMMI, intRegCounter++, REG_BASE_POINTER, tokens->at(tokenCounter).value);
									}
									break;
								case DATA_TYPE_INT:
									if (isPointer) {
										opcode_r_mr(irs, _MOV_R_MR, intRegCounter++, REG_INDEX_POINTER);
									} else {
										opcode_r_mr_immi(irs, _MOV_R_MR_IMMI, intRegCounter++, REG_BASE_POINTER, tokens->at(tokenCounter).value);
									}
									break;
								case DATA_TYPE_FLOAT:
									if (isPointer) {
										opcode_fr_mfr(irs, _FMOV_FR_MFR, floatRegCounter, REG_INDEX_POINTER);
									} else {
										opcode_fr_mfr_immi(irs, _FMOV_FR_MFR_IMMI, floatRegCounter, REG_BASE_POINTER, tokens->at(tokenCounter).value);
									}
									opcode_r_fr(irs, _FCON_R_FR, intRegCounter++, floatRegCounter);
							}
						}
					}
				}
				break;
			case TOKEN_REGISTER:
				if (generateFloatInstructions) {
					if (tokens->at(tokenCounter).dataType == DATA_TYPE_FLOAT) {
						if (tokens->at(tokenCounter).isPointer) {
							opcode_fr_mfr(irs, _FMOV_FR_MFR, floatRegCounter++, tokens->at(tokenCounter).value);
						} else {
							opcode_fr_fr(irs, _FMOV_FR_FR, floatRegCounter++, tokens->at(tokenCounter).value);
						}
					} else {
						if (tokens->at(tokenCounter).isPointer) {
							if (tokens->at(tokenCounter).dataType == DATA_TYPE_BYTE) {
								opcode_r_mr(irs, _BMOV_R_BR, intRegCounter, tokens->at(tokenCounter).value);
								opcode_fr_r(irs, _FCON_FR_R, floatRegCounter++, intRegCounter);
							} else {
								opcode_r_mr(irs, _MOV_R_MR, intRegCounter, tokens->at(tokenCounter).value);
								opcode_fr_r(irs, _FCON_FR_R, floatRegCounter++, intRegCounter);
							}
						} else {
							opcode_fr_r(irs, _FCON_FR_R, floatRegCounter++, tokens->at(tokenCounter).value);
						}
					}
				} else {
					if (tokens->at(tokenCounter).dataType == DATA_TYPE_FLOAT) {
						if (tokens->at(tokenCounter).isPointer) {
							opcode_fr_mfr(irs, _FMOV_FR_MFR, floatRegCounter, tokens->at(tokenCounter).value);
							opcode_r_fr(irs, _FCON_R_FR, intRegCounter++, floatRegCounter);
						} else {
							opcode_r_fr(irs, _FCON_R_FR, intRegCounter++, tokens->at(tokenCounter).value);
						}
					} else {
						if (tokens->at(tokenCounter).isPointer) {
							if (tokens->at(tokenCounter).dataType == DATA_TYPE_BYTE) {
								opcode_r_mr(irs, _BMOV_R_BR, intRegCounter++, tokens->at(tokenCounter).value);
							} else {
								opcode_r_mr(irs, _MOV_R_MR, intRegCounter++, tokens->at(tokenCounter).value);
							}
						} else {
							opcode_r_r(irs, _MOV_R_R, intRegCounter++, tokens->at(tokenCounter).value);
						}
					}
				}
				break;
			default:
				{
					string errMsg = ERR_STR_SYNTAX_ERROR;
					throw  LockableException(errMsg);
				}
		}
		++tokenCounter;
	}
}

static void precedenceLevel1() {
	while (true) {
		precedenceLevel2();
		switch (tokens->at(tokenCounter).operation) {
			case OPERATOR_LOG_AND:
				operation(_AND_R_R, 1, OPERATION_TYPE_LOG_AND_OR);
				break;
			case OPERATOR_LOG_OR:
				operation(_OR_R_R, 1, OPERATION_TYPE_LOG_AND_OR);
				break;
			case OPERATOR_CLOSE_ROUND_BRACKET:
				++tokenCounter;
				return;
			default:
				if ((*tokens)[tokenCounter] != *terminator) {
					string errMsg = ERR_STR_SYNTAX_ERROR;
					throw  LockableException(errMsg);
				}
				return;
		}
	}
}

static void precedenceLevel2() {
	precedenceLevel3();
	while (true) {
		switch (tokens->at(tokenCounter).operation) {
			case OPERATOR_GREATER:
				operation((generateFloatInstructions) ? _CMPG_R_FR_FR : _CMPG_R_R, 2, OPERATION_TYPE_COMPARE);
				break;
			case OPERATOR_LESS:
				operation((generateFloatInstructions) ? _CMPL_R_FR_FR : _CMPL_R_R, 2, OPERATION_TYPE_COMPARE);
				break;
			case OPERATOR_GREATER_EQUAL:
				operation((generateFloatInstructions) ? _CMPGE_R_FR_FR : _CMPGE_R_R, 2, OPERATION_TYPE_COMPARE);
				break;
			case OPERATOR_LESS_EQUAL:
				operation((generateFloatInstructions) ? _CMPLE_R_FR_FR :_CMPLE_R_R, 2, OPERATION_TYPE_COMPARE);
				break;
			case OPERATOR_EQUAL:
				operation((generateFloatInstructions) ? _CMPE_R_FR_FR : _CMPE_R_R, 2, OPERATION_TYPE_COMPARE);
				break;
			case OPERATOR_NOT_EQUAL:
				operation((generateFloatInstructions) ? _CMPNE_R_FR_FR :_CMPNE_R_R, 2, OPERATION_TYPE_COMPARE);
				break;
			default:
				return;
		}
	}
}

static void precedenceLevel3() {
	precedenceLevel4();
	while (true) {
		switch (tokens->at(tokenCounter).operation) {
			case OPERATOR_ADD:
				if (generateFloatInstructions)
					operation(_FADD_FR_FR, 3, OPERATION_TYPE_GENERAL);
				else
					operation(_ADD_R_R, 3, OPERATION_TYPE_GENERAL);
				break;
			case OPERATOR_SUBSTRACT:
				if (generateFloatInstructions)
					operation(_FSUB_FR_FR, 3, OPERATION_TYPE_GENERAL);
				else
					operation(_SUB_R_R, 3, OPERATION_TYPE_GENERAL);
				break;
			case OPERATOR_AND:
				if (generateFloatInstructions) {
					string errMsg = ERR_STR_SYNTAX_ERROR;
					throw LockableException(errMsg);
				} else {
					operation(_AND_R_R, 3, OPERATION_TYPE_GENERAL);
				}
				break;
			case OPERATOR_OR:
				if (generateFloatInstructions) {
					string errMsg = ERR_STR_SYNTAX_ERROR;
					throw LockableException(errMsg);
				} else {
					operation(_OR_R_R, 3, OPERATION_TYPE_GENERAL);
				}
				break;
			case OPERATOR_XOR:
				if (generateFloatInstructions) {
					string errMsg = ERR_STR_SYNTAX_ERROR;
					throw LockableException(errMsg);
				} else {
					operation(_XOR_R_R, 3, OPERATION_TYPE_GENERAL);
				}
				break;
			case OPERATOR_SHL:
				if (generateFloatInstructions) {
					string errMsg = ERR_STR_SYNTAX_ERROR;
					throw LockableException(errMsg);
				} else {
					operation(_SHL_R_R, 3, OPERATION_TYPE_GENERAL);
				}
				break;
			case OPERATOR_SHR:
				if (generateFloatInstructions) {
					string errMsg = ERR_STR_SYNTAX_ERROR;
					throw LockableException(errMsg);
				} else {
					operation(_SHR_R_R, 3, OPERATION_TYPE_GENERAL);
				}
				break;
			default:
				return;
		}
	}
}

static void precedenceLevel4() {
	precedenceLevel5();
	while (true) {
		switch (tokens->at(tokenCounter).operation) {
			case OPERATOR_MOD:
				if (generateFloatInstructions)
					operation(_FMOD_FR_FR, 4, OPERATION_TYPE_GENERAL);
				else
					operation(_MOD_R_R, 4, OPERATION_TYPE_GENERAL);
				break;
			default:
				return;
		}
	}
}

static void precedenceLevel5() {
	while (true) {
		switch (tokens->at(tokenCounter).operation) {
			case OPERATOR_MULTIPLY:
				if (generateFloatInstructions)
					operation(_FMUL_FR_FR, 5, OPERATION_TYPE_GENERAL);
				else
					operation(_MUL_R_R, 5, OPERATION_TYPE_GENERAL);
				break;
			case OPERATOR_DIVIDE:
				if (generateFloatInstructions)
					operation(_FDIV_FR_FR, 5, OPERATION_TYPE_GENERAL);
				else
					operation(_DIV_R_R, 5, OPERATION_TYPE_GENERAL);
				break;
			default:
				return;
		}
	}
}

static void operation(uint16 opcode, int precedence, int type) {
	++tokenCounter;
	loadValToReg();
	if (precedence != 5) funcPrecedence[precedence]();

	IntermediateRepresentation ir;
	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	Operand operand;

	if (generateFloatInstructions && type != OPERATION_TYPE_LOG_AND_OR) {
		if (type == OPERATION_TYPE_GENERAL) {
			operand.type = OPERAND_FLOAT_REGISTER;
			operand.value = getRegister(floatRegCounter, 2);
			ir.operands.push_back(operand);
			operand.value = getRegister(floatRegCounter, 1);
			ir.operands.push_back(operand);
			--floatRegCounter;
		} else {
			operand.type = OPERAND_REGISTER;
			operand.value = intRegCounter++;
			ir.operands.push_back(operand);
			operand.type = OPERAND_FLOAT_REGISTER;
			operand.value = getRegister(floatRegCounter, 2);
			ir.operands.push_back(operand);
			operand.value = getRegister(floatRegCounter, 1);
			ir.operands.push_back(operand);
			floatRegCounter -= 2;
		}
	} else {
		operand.type = OPERAND_REGISTER;
		operand.value = getRegister(intRegCounter, 2);
		ir.operands.push_back(operand);
		operand.value = getRegister(intRegCounter, 1);
		ir.operands.push_back(operand);
		--intRegCounter;
	}

	irs->push_back(ir);
}

static int getRegister(int regCounter, int decrementBy) {
	int retVal;

	retVal = regCounter - decrementBy;
	if (retVal < 0) {
		string errMsg = ERR_STR_SYNTAX_ERROR;
		throw LockableException(errMsg);
	}

	return retVal;
}
