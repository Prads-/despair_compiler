/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include <cstdlib>
#include "stringParser.h"
#include "instructionsSet.h"
#include "identifierDataType.h"
#include "parseHelper.h"
#include "errorStrings.h"
#include "lockableException.h"
#include "constCharTable.h"
#include "portAddress.h"
using namespace std;
using namespace IREmitter;
using namespace ParseHelper;

//From parseModule.cpp
extern Function *function;
extern const ConstCharTable *constCharTable;
extern const Function *functionCall(const TokenLine *functionCallTokens, int paramMemOffset, int intRegOffset, int floatRegOffset, list<IntermediateRepresentation> *irsOut);

void StringParser::parseString(const TokenLine *tokenLine, uint32 offset, int intRegOffset, int floatRegOffset, Token terminator, std::list<IntermediateRepresentation> *irsOut) {
	//Clear the string at 'String Pointer Register'
	opcode_immi_r(irsOut, _OUT_IMMI_R64, 40, REG_STRING_POINTER);
	opcode_immi_immi8(irsOut, _OUT_IMMI_IMMI8, 64, 9);

	IdentifierDataType dataType;
	bool isLiteral;

	while (true) {
		switch (tokenLine->tokens.at(offset).type) {
			case TOKEN_IDENTIFIER:
				switch (tokenLine->tokens.at(offset + 1).operation) {
					case OPERATOR_OPEN_ROUND_BRACKET:	//Function
						{
							//Get function call
							TokenLine functionCallTokens;
							getFunctionCall(tokenLine, offset, &functionCallTokens);
							++offset;

							//Push the 'String Pointer Register' into the stack
							opcode_r(irsOut, _PUSH_R, REG_STRING_POINTER);

							//Get function call IRs
							const Function *calledFunction = functionCall(&functionCallTokens, function->identsSize, intRegOffset, floatRegOffset, irsOut);
							dataType = calledFunction->returnType;

							//Pop the 'String Pointer Register' from the stack and put it in 'String Object' Port
							opcode_r(irsOut, _POP_R, REG_STRING_POINTER);
							opcode_immi_r(irsOut, _OUT_IMMI_R64, 40, REG_STRING_POINTER);

							//Check if the function call returns group and if it does, get the group member
							if (dataType == DATA_TYPE_GROUP) {
								//Get the group Member
								uint32 index;
								bool isIndexNotValid;
								list<IntermediateRepresentation> groupMemberAddress;
								const Ident *ident = getGroupMember(tokenLine, offset, calledFunction->returnTypeStr, 0, intRegOffset, &groupMemberAddress, &index, &isIndexNotValid);
								dataType = ident->dataType;
								++offset;

								if (dataType == DATA_TYPE_STRING && !ident->isPointer) {
									string errMsg = ERR_STR_GRP_MEM_STR_ACCESS;
									throw LockableException(errMsg);
								}

								//Get the group member address
								if (isIndexNotValid) {
									irsOut->splice(irsOut->end(), groupMemberAddress);
								} else {
									opcode_r_immi(irsOut, _ADD_R_IMMI, 0, index);
									if (ident->isPointer) {
										opcode_r_mr(irsOut, _MOVP_R_MR, 0, 0);
									}
								}

								//Put the value in group member to register 0
								switch (dataType) {
									case DATA_TYPE_BYTE:
										opcode_r_mr(irsOut, _BMOV_R_BR, 0, 0);
										break;
									case DATA_TYPE_INT:
										opcode_r_mr(irsOut, _MOV_R_MR, 0, 0);
										break;
									case DATA_TYPE_FLOAT:
										opcode_fr_mfr(irsOut, _FMOV_FR_MFR, 0, 0);
										break;
									case DATA_TYPE_STRING:
										opcode_r_mr(irsOut, _MOVP_R_MR, 0, 0);

								}
							} else if (dataType == DATA_TYPE_STRING) {
								//If function returns string, put the return value to the register 0
								opcode_r_r(irsOut, _MOV_R_R, 0, REG_RETURN_STRING);
							}
						}
						break;
					default:	//Variable and Group
						{
							//Get the ident
							uint32 index;
							bool isIndexNotValid, isGlobal;
							list<IntermediateRepresentation> groupMemberAddress;
							const Ident *ident = getIdent(tokenLine, offset, REG_INDEX_POINTER, &groupMemberAddress, &index, &isIndexNotValid, &isGlobal, intRegOffset);
							dataType = ident->dataType;
							++offset;

							if (isGlobal) {
								if (ident->isPointer) {
									opcode_r_m(irsOut, _MOVP_R_M, REG_INDEX_POINTER, index);
								}

								//Put the value in ident to register 0
								switch (dataType) {
									case DATA_TYPE_BYTE:
										if (ident->isPointer) {
											opcode_r_mr(irsOut, _BMOV_R_BR, 0, REG_INDEX_POINTER);
										} else {
											opcode_r_m(irsOut, _BMOV_R_BM, 0, index);
										}
										break;
									case DATA_TYPE_INT:
										if (ident->isPointer) {
											opcode_r_mr(irsOut, _MOV_R_MR, 0, REG_INDEX_POINTER);
										} else {
											opcode_r_m(irsOut, _MOV_R_M, 0, index);
										}
										break;
									case DATA_TYPE_FLOAT:
										if (ident->isPointer) {
											opcode_r_mr(irsOut, _FMOV_FR_MFR, 0, REG_INDEX_POINTER);
										} else {
											opcode_r_m(irsOut, _FMOV_FR_FM, 0, index);
										}
										break;
									case DATA_TYPE_STRING:
										if (ident->isPointer) {
											opcode_r_mr(irsOut, _MOVP_R_MR, 0, REG_INDEX_POINTER);
										} else {
											opcode_r_m(irsOut, _MOVP_R_M, 0, index);
										}
								}
							} else {
								//Get ident address
								if (isIndexNotValid) {
									irsOut->splice(irsOut->end(), groupMemberAddress);
								} else {
									opcode_r_r(irsOut, _MOV_R_R, REG_INDEX_POINTER, REG_BASE_POINTER);
									opcode_r_immi(irsOut, _ADD_R_IMMI, REG_INDEX_POINTER, index);
									if (ident->isPointer) {
										opcode_r_mr(irsOut, _MOVP_R_MR, REG_INDEX_POINTER, REG_INDEX_POINTER);
									}
								}

								//Put the value in ident to register 0
								switch (dataType) {
									case DATA_TYPE_BYTE:
										opcode_r_mr(irsOut, _BMOV_R_BR, 0, REG_INDEX_POINTER);
										break;
									case DATA_TYPE_INT:
										opcode_r_mr(irsOut, _MOV_R_MR, 0, REG_INDEX_POINTER);
										break;
									case DATA_TYPE_FLOAT:
										opcode_fr_mfr(irsOut, _FMOV_FR_MFR, 0, REG_INDEX_POINTER);
										break;
									case DATA_TYPE_STRING:
										opcode_r_mr(irsOut, _MOVP_R_MR, 0, REG_INDEX_POINTER);
								}
							}
						}
				}
				isLiteral = false;
				break;
			case TOKEN_FLOAT_NUMBER:
				opcode_immi_fimmi(irsOut, _FOUT_IMMI_FIMMI, 48, atof(tokenLine->tokens.at(offset++).value.c_str()));
				opcode_immi_immi8(irsOut, _OUT_IMMI_IMMI8, 64, 8);
				isLiteral = true;
				break;
			case TOKEN_NUMBER:
				opcode_immi_immi(irsOut, _OUT_IMMI_IMMI32, 48, atoi(tokenLine->tokens.at(offset++).value.c_str()));
				opcode_immi_immi8(irsOut, _OUT_IMMI_IMMI8, 64, 7);
				isLiteral = true;
				break;
			case TOKEN_CONST_CHAR:
				{
					//Get the const char
					const ConstChar *constChar = &constCharTable->find(tokenLine->tokens.at(offset++).value)->second;

					//Get the address
					opcode_r_r(irsOut, _MOV_R_R, 0, REG_GLOBAL_BASE_POINTER);
					opcode_r_immi(irsOut, _ADD_R_IMMI, 0, constChar->index);

					//Append const char
					opcode_immi_r(irsOut, _OUT_IMMI_R64, PORT_STRING_IO_1, 0);
					opcode_immi_immi8(irsOut, _OUT_IMMI_IMMI8, PORT_STRING_COMMAND, 5);
				}
				isLiteral = true;
				break;
			case TOKEN_KEYWORD:
				switch (tokenLine->tokens.at(offset).keyword) {
					case KW_BYTE:
					case KW_INT:
					case KW_FLOAT:
					case KW_STRING:
						++offset;
						continue;
				}
			default:
				{
					string errMsg = ERR_STR_SYNTAX_ERROR;
					throw LockableException(errMsg);
				}
		}

		if (!isLiteral) {
			switch (dataType) {
				case DATA_TYPE_BYTE:
				case DATA_TYPE_INT:
					opcode_immi_r(irsOut, _OUT_IMMI_R32, 48, 0);
					opcode_immi_immi8(irsOut, _OUT_IMMI_IMMI8, 64, 7);
					break;
				case DATA_TYPE_FLOAT:
					opcode_immi_fr(irsOut, _FOUT_IMMI_FR, 48, 0);
					opcode_immi_immi8(irsOut, _OUT_IMMI_IMMI8, 64, 8);
					break;
				case DATA_TYPE_STRING:
					opcode_immi_r(irsOut, _OUT_IMMI_R64, 48, 0);
					opcode_immi_immi8(irsOut, _OUT_IMMI_IMMI8, 64, 6);
			}
		}

		//Syntax check
		if (tokenLine->tokens.at(offset).operation == OPERATOR_ADD) {
			++offset;
		} else if (tokenLine->tokens.at(offset) == terminator) {
			break;
		} else {
			string errMsg = ERR_STR_SYNTAX_ERROR;
			throw LockableException(errMsg);
		}
	}
}
