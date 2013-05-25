/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include "switch.h"
#include "lockableException.h"
#include "errorStrings.h"
#include "constantParser.h"
#include <algorithm>
#include "identifierDataType.h"
#include "intermediateRepresentation.h"
#include "instructionsSet.h"
#include "parseHelper.h"
#include "BinaryTree.h"
#include "stringParser.h"
#include "stringExtension.h"
using namespace std;
using namespace ConstantParser;
using namespace IREmitter;
using namespace ParseHelper;
using namespace StringParser;
using namespace StringExtension;

#define SWITCH_BLOCK_TYPE_CASE			1
#define SWITCH_BLOCK_TYPE_DEFAULT		2

struct CaseCondition {
	bool isConstant;
	TokenLine value;
	int constValue;
};

class SwitchBlock {
public:
	bool isFallThrough;
	vector<TokenLine> statements;
	list<IntermediateRepresentation> irs;
	int label;

	virtual int getType() const = 0;
};

class CaseBlock : public SwitchBlock {
public:
	vector<CaseCondition> caseConditions;
	
	int getType() const {
		return SWITCH_BLOCK_TYPE_CASE;
	}
};

struct DefaultBlock : public SwitchBlock {
public:
	int getType() const {
		return SWITCH_BLOCK_TYPE_DEFAULT;
	}
};

struct SwitchStructure {
	vector<SwitchBlock*> switchBlocks;
	int defaultBlockIndex;
	bool hasDefaultBlock;

	SwitchStructure() {
		hasDefaultBlock = false;
	}

	~SwitchStructure() {
		for (SwitchBlock *sb : switchBlocks) {
			delete sb;
		}
	}
};

struct ConstantCondition {
	int constValue;
	uint32 label;
	//Labels below are used for binary search
	uint32 lblBinSearch, lblIfGreater;
	
	ConstantCondition() {
		label = 0;
		lblBinSearch = 0;
		lblIfGreater = 0;
	}

	bool operator < (const ConstantCondition &cc) const {
		return constValue < cc.constValue;
	}
};

void extractSwitchStatement(const vector<TokenLine> *tokenLine, uint32 &index, vector<TokenLine> *switchOut);
void getCaseBlocks(const vector<TokenLine> *switchStatement, SwitchStructure *switchStructure);
void createJumpTable(vector<ConstantCondition> *constantConditions, int reg, uint32 lblOut, uint32 lblBreak, uint32 range, SwitchStructure *switchStructure, list<IntermediateRepresentation> *switchExpIRS, list<IntermediateRepresentation> *irsOut);
void createBinarySearchTable(TreeNode<ConstantCondition> *rootNode, int lblBinSearch, int lblOut, int &label, vector<vector<ConstantCondition*>> *tableOut);
void createStaticBinarySearch(vector<ConstantCondition> *constantConditions, int reg, uint32 lblOut, uint32 lblBreak, int &label, SwitchStructure *switchStructure, list<IntermediateRepresentation> *switchExpIRS, list<IntermediateRepresentation> *irsOut);
void createConstantSerialSearch(vector<ConstantCondition> *constantConditions, int reg, uint32 lblOut, uint32 lblBreak, SwitchStructure *switchStructure, list<IntermediateRepresentation> *switchExpIRS, list<IntermediateRepresentation> *irsOut);
void createSerialSearch(TokenLine *switchExpression, SwitchStructure *switchStructure, IdentifierDataType switchExpDataType, bool switchExpIsCompare, uint32 lblOut, uint32 lblBreak, list<IntermediateRepresentation> *switchExpIRS, list<IntermediateRepresentation> *irsOut);

bool caseConditionCompareFunc(const CaseCondition &cc1, const CaseCondition &cc2) { return cc1.constValue < cc2.constValue; }

//From parseModule.cpp
extern void parseLines(const vector<TokenLine> *tokenLines, uint32 &offset, list<IntermediateRepresentation> *irsOut, int lblBreak = 0, int lblContinue = 0);
extern int parseExpression(const TokenLine *expression, IdentifierDataType dataType, Token terminator, int paramMemOffset, int intRegOffset, int floatRegOffset, bool isCompare, list<IntermediateRepresentation> *irsOut);
extern Function *function;

void SwitchParser::parseSwitch(const std::vector<TokenLine> *tokenLines, uint32 &index, int &label, int lblContinue, std::list<IntermediateRepresentation> *irsOut) {
	//Get switch statement
	vector<TokenLine> switchStatement;
	extractSwitchStatement(tokenLines, index, &switchStatement);
	
	//Get case blocks 
	SwitchStructure switchStructure;
	getCaseBlocks(&switchStatement, &switchStructure);

	//Give each switch block a label
	for(SwitchBlock *sb : switchStructure.switchBlocks) {
		if (sb->getType() == SWITCH_BLOCK_TYPE_CASE) {
			sb->label = label++;
		}
	}
	int lblOut = label++;	//Marks the end of switch statement or to the default block if one exists
	int lblBreak; //Marks the end of switch statement
	if (switchStructure.hasDefaultBlock) {
		switchStructure.switchBlocks.at(switchStructure.defaultBlockIndex)->label = lblOut;
		lblBreak = label++;
	} else {
		lblBreak = lblOut;
	}

	//Parse the switch blocks
	for (SwitchBlock *sb : switchStructure.switchBlocks) {
		if (sb->statements.size() == 0) {
			LockableException e = LockableException(ERR_STR_CASE_BLOCK_EMPTY);
			e.lock();
			throw e;
		}
		if (!sb->isFallThrough && sb->statements.at(sb->statements.size() - 1).type != TOKEN_LINE_BREAK) {
			LockableException e = LockableException(ERR_STR_CASE_NO_FALLTHROUGH);
			e.lock();
			throw e;
		}

		uint32 offset = 0;
		opcode(&sb->irs, _NOP, (sb->getType() == SWITCH_BLOCK_TYPE_CASE) ? sb->label : lblOut);
		parseLines(&sb->statements, offset, &sb->irs, lblBreak, lblContinue);
	}

	//Get the switch expression
	TokenLine switchExpression;
	uint32 offset = 2;
	getExpressionInsideBracket(&switchStatement[0], offset, OPERATOR_OPEN_ROUND_BRACKET, OPERATOR_CLOSE_ROUND_BRACKET, &switchExpression);
	switchExpression.tokens.push_back(Token(TOKEN_OPERATOR, KW_NONE, OPERATOR_SEMI_COLON, ";"));
	bool isCompare = false;
	IdentifierDataType switchExpDataType = getExpressionDataType(&switchExpression, 0, isCompare);

	//Check to see if all case conditions are constants
	bool allCasesAreConstant = true;
	if (switchExpDataType == DATA_TYPE_INT || switchExpDataType == DATA_TYPE_BYTE) {
		for (const SwitchBlock *sb : switchStructure.switchBlocks) {
			if (sb->getType() == SWITCH_BLOCK_TYPE_CASE) {
				const CaseBlock *caseBlock = (const CaseBlock*)sb;
				for (const CaseCondition &caseCondition : caseBlock->caseConditions) {
					allCasesAreConstant = caseCondition.isConstant;
					if (!allCasesAreConstant) {
						goto skip_Constant_Condition_Check_Loop;
					}
				}
			}
		}
	} else {
		allCasesAreConstant = false;
	}
skip_Constant_Condition_Check_Loop:

	list<IntermediateRepresentation> switchExpIRS;
	if (allCasesAreConstant) {
		//Parse the switch expression
		uint32 offset = 0;
		int reg = parseExpression(&switchExpression, switchExpDataType, Token(TOKEN_OPERATOR, KW_NONE, OPERATOR_SEMI_COLON, ";"), 
			function->identsSize, 1, 1, isCompare,	&switchExpIRS);
		
		//Get all constant conditions and sort them
		vector<ConstantCondition> constantConditions;
		for (const SwitchBlock *sb : switchStructure.switchBlocks) {
			if (sb->getType() == SWITCH_BLOCK_TYPE_CASE) {
				const CaseBlock *caseBlock = (CaseBlock*)sb;
				ConstantCondition constantCondition;
				constantCondition.label = caseBlock->label;
				for (const CaseCondition &caseCondition : caseBlock->caseConditions) {
					constantCondition.constValue = caseCondition.constValue;
					constantConditions.push_back(constantCondition);
				}
			}
		}
		sort(constantConditions.begin(), constantConditions.end());
		
		//Check if the constant conditions are unique
		for (size_t i = 0; i < constantConditions.size() - 1; ++i) {
			if (constantConditions[i].constValue == constantConditions[i + 1].constValue) {
				LockableException e(ERR_STR_CASE_NOT_UNIQUE);
				e.lock();
				throw e;
			}
		}

		if (constantConditions.size() <= 4) { //Serial Search
			createConstantSerialSearch(&constantConditions, reg, lblOut, lblBreak, &switchStructure, &switchExpIRS, irsOut);
		} else {
			uint32 range = constantConditions[constantConditions.size() - 1].constValue - constantConditions[0].constValue + 1;

			if (range > 1000) { //Static Binary Search
				createStaticBinarySearch(&constantConditions, reg, lblOut, lblBreak, label, &switchStructure, &switchExpIRS, irsOut);
			} else { //Jump Table
				createJumpTable(&constantConditions, reg, lblOut, lblBreak, range, &switchStructure, &switchExpIRS, irsOut);
			}
		}
	} else { //Non constant case conditions, serial search
		createSerialSearch(&switchExpression, &switchStructure, switchExpDataType, isCompare, lblOut, lblBreak, &switchExpIRS, irsOut);
	}
}

void extractSwitchStatement(const vector<TokenLine> *tokenLine, uint32 &index, vector<TokenLine> *switchOut) {
	uint32 curlyBacketLevel = 1;
	switchOut->push_back(tokenLine->at(index++));	//The first line is switch declaration
	bool switchStatements = true;

	while (switchStatements) {
		switch (tokenLine->at(index).type) {
			case TOKEN_LINE_LOOP:
			case TOKEN_LINE_COMPARE:
				++curlyBacketLevel;
				break;
			case TOKEN_LINE_CLOSE_SCOPE:
				--curlyBacketLevel;
				if (curlyBacketLevel == 0) {
					switchStatements = false;
				}
		}
		switchOut->push_back(tokenLine->at(index++));
	}
}

void getBlockStatements(const vector<TokenLine> *switchStatement, size_t &index, SwitchBlock *sb) {
	while (switchStatement->at(index).type != TOKEN_LINE_CASE && switchStatement->at(index).type != TOKEN_LINE_DEFAULT &&
			switchStatement->at(index).type != TOKEN_LINE_FCASE && switchStatement->at(index).type != TOKEN_LINE_FDEFAULT) {
		if (switchStatement->at(index).type == TOKEN_LINE_CLOSE_SCOPE && index == switchStatement->size() - 1) {
			break;
		} else {
			sb->statements.push_back(switchStatement->at(index++));
		}
	}
}

void getCaseBlock(const vector<TokenLine> *switchStatement, size_t &index, CaseBlock *caseBlock) {
	//Lets get all the case conditions first
	bool exitLoop = false;
	int tokenIndex = 1;
	const TokenLine *caseTokens = &switchStatement->at(index++);

	while (!exitLoop) {
		CaseCondition caseCondition;
		TokenLine caseConditionValue;
		bool isConstant = true;

		while (true) {
			if (caseTokens->tokens.at(tokenIndex).type != TOKEN_NUMBER && caseTokens->tokens.at(tokenIndex).type != TOKEN_OPERATOR) {
				isConstant = false;
			}

			caseConditionValue.tokens.push_back(caseTokens->tokens.at(tokenIndex++));

			if (caseTokens->tokens.at(tokenIndex).operation == OPERATOR_COLON) {
				exitLoop = true;
				break;
			} else if (caseTokens->tokens.at(tokenIndex).operation == OPERATOR_COMMA) {
				++tokenIndex;
				break;
			}
		}

		if (caseConditionValue.tokens.size() != 0) {
			caseConditionValue.tokens.push_back(Token(TOKEN_OPERATOR, KW_NONE, OPERATOR_SEMI_COLON, ";"));
			caseCondition.isConstant = isConstant;
			
			//Get the constant value
			if (isConstant) {
				caseCondition.constValue = constantToInteger(&caseConditionValue);
			}

			caseCondition.value = caseConditionValue;
			caseBlock->caseConditions.push_back(caseCondition);
		}
	}

	//Make sure that case has atleast one condition
	if (caseBlock->caseConditions.size() == 0) {
		throw LockableException(ERR_STR_ZERO_CASE_CONDITION);
	}

	//Get case statements
	getBlockStatements(switchStatement, index, (SwitchBlock*)caseBlock);
}

void getDefaultBlock(const vector<TokenLine> *switchStatement, size_t &index, DefaultBlock *defaultBlock) {
	getBlockStatements(switchStatement, index, (SwitchBlock*)defaultBlock);
}

void getCaseBlocks(const vector<TokenLine> *switchStatement, SwitchStructure *switchStructure) {
	uint32 caseID = 0;

	for (size_t i = 1; i < switchStatement->size() - 1;) {
		if ((*switchStatement)[i].type == TOKEN_LINE_CASE || (*switchStatement)[i].type == TOKEN_LINE_FCASE) {
			SwitchBlock *caseBlock = new CaseBlock();
			caseBlock->isFallThrough = (*switchStatement)[i].type == TOKEN_LINE_FCASE;
			getCaseBlock(switchStatement, i, (CaseBlock*)caseBlock);
			switchStructure->switchBlocks.push_back(caseBlock);
		} else if ((*switchStatement)[i].type == TOKEN_LINE_DEFAULT || (*switchStatement)[i].type == TOKEN_LINE_FDEFAULT) {
			SwitchBlock *defaultBlock = new DefaultBlock();
			defaultBlock->isFallThrough = (*switchStatement)[i].type == TOKEN_LINE_FDEFAULT; 
			getDefaultBlock(switchStatement, ++i, (DefaultBlock*)defaultBlock);
			switchStructure->switchBlocks.push_back(defaultBlock);
			if (!switchStructure->hasDefaultBlock) {
				switchStructure->hasDefaultBlock = true;
				switchStructure->defaultBlockIndex = switchStructure->switchBlocks.size() - 1;
			} else {
				LockableException le = LockableException(ERR_STR_MULTIPLE_DEFAULT);
				le.lock();
				throw le;
			}
		} else {
			LockableException le = LockableException(ERR_STR_SYNTAX_ERR_SWITCH + (*switchStatement)[i].toString());
			le.lock();
			throw le;
		}
	}
}

void createJumpTable(vector<ConstantCondition> *constantConditions, int reg, uint32 lblOut, uint32 lblBreak, uint32 range, SwitchStructure *switchStructure, list<IntermediateRepresentation> *switchExpIRS, list<IntermediateRepresentation> *irsOut) {
	//Check to see if there are negative constant
	int negConstant = 0;
	if ((*constantConditions)[0].constValue != 0) {
		negConstant = -(*constantConditions)[0].constValue;
	}

	//Make sure constants start from 0
	if (negConstant) {
		for (ConstantCondition &cc : *constantConditions) {
			cc.constValue += negConstant;
		}
	}

	//Process the expression
	if (negConstant) {
		opcode_r_immi(switchExpIRS, _ADD_R_IMMI, reg, negConstant);
	}

	//range checks
	opcode_r_r(switchExpIRS, _MOV_R_R, reg + 1, reg);
	opcode_r_r(switchExpIRS, _MOV_R_R, reg + 2, reg);
	opcode_r_immi(switchExpIRS, _CMPGE_R_IMMI, reg + 1, constantConditions->at(0).constValue);
	opcode_r_immi(switchExpIRS, _CMPLE_R_IMMI, reg + 2, constantConditions->at(constantConditions->size() - 1).constValue);
	opcode_r_r(switchExpIRS, _AND_R_R, reg + 1, reg + 2);
	opcode_r_label(switchExpIRS, _JC_R_IMMI, reg + 1, lblOut);

	//The size of jump instructions in jump table is 6
	opcode_r_immi(switchExpIRS, _MUL_R_IMMI, reg, 6);

	//Jump to the jump table
	opcode_r(switchExpIRS, _JMPR_R, reg);

	//Create jump table
	vector<IntermediateRepresentation> jumpTable;
	IntermediateRepresentation jmpOutIR;
	jmpOutIR.opcode = _JMP_IMMI;
	jmpOutIR.address = 0;
	jmpOutIR.label = 0;
	jmpOutIR.operands.push_back(Operand(OPERAND_LABEL, lblOut, 0, ""));
	for (uint32 i = 0; i < range; ++i) {
		jumpTable.push_back(jmpOutIR);
	}
	//Put condition jumps in correct place in jump table
	for (ConstantCondition &cc : *constantConditions) {
		vector<Operand> *operands = &jumpTable.at(cc.constValue).operands;
		operands->at(0).value = cc.label;
	}

	//Now connect all the pieces together
	//First the switch expression
	irsOut->splice(irsOut->end(), *switchExpIRS);
	//Next the jump table
	copy(jumpTable.begin(), jumpTable.end(), back_inserter(*irsOut));
	//Lastly, all the switch blocks
	for (SwitchBlock *sb : switchStructure->switchBlocks) {
		irsOut->splice(irsOut->end(), sb->irs);
	}
			
	//Mark the end of the switch statement
	opcode(irsOut, _NOP, lblBreak);
}

void createBinarySearchTable(TreeNode<ConstantCondition> *rootNode, int lblBinSearch, int lblOut, int &label, vector<vector<ConstantCondition*>> *tableOut) {
	vector<ConstantCondition*> searchLine;
	TreeNode<ConstantCondition> *currentNode = rootNode;
	rootNode->getParent()->lblBinSearch = lblBinSearch;

	while(currentNode) {
		searchLine.push_back(currentNode->getParent());
		if (currentNode->getRightNode()) {
			int currentLabel = label++;
			currentNode->getParent()->lblIfGreater = currentLabel;
			createBinarySearchTable(currentNode->getRightNode(), currentLabel, lblOut, label, tableOut);
		} else {
			currentNode->getParent()->lblIfGreater = lblOut;
		}
		currentNode = currentNode->getLeftNode();
	}

	tableOut->push_back(searchLine);
}

void createStaticBinarySearch(vector<ConstantCondition> *constantConditions, int reg, uint32 lblOut, uint32 lblBreak, int &label, SwitchStructure *switchStructure, list<IntermediateRepresentation> *switchExpIRS, list<IntermediateRepresentation> *irsOut) {
	//Create a binary tree stucture from sorted case conditions
	BinaryTree<ConstantCondition> binaryTree(constantConditions);

	//Create a binary search table
	vector<vector<ConstantCondition*>> binarySearchTable;
	createBinarySearchTable(binaryTree.getRootNode(), 0, lblOut, label, &binarySearchTable);

	//Generate binary search table IRS
	list<IntermediateRepresentation> binarySearchIRS;
	for (int i = binarySearchTable.size() - 1; i >= 0; --i) {
		for (ConstantCondition *cc : binarySearchTable.at(i)) {
			opcode_r_r(&binarySearchIRS, _MOV_R_R, reg + 1, reg);
			list<IntermediateRepresentation>::iterator lastAdded = binarySearchIRS.end();
			(--lastAdded)->label = cc->lblBinSearch;
			opcode_r_immi(&binarySearchIRS, _CMPNE_R_IMMI, reg + 1, cc->constValue);
			opcode_r_label(&binarySearchIRS, _JC_R_IMMI, reg + 1, cc->label);
			opcode_r_r(&binarySearchIRS, _MOV_R_R, reg + 1, reg);
			opcode_r_immi(&binarySearchIRS, _CMPL_R_IMMI, reg + 1, cc->constValue);
			opcode_r_label(&binarySearchIRS, _JC_R_IMMI, reg + 1, cc->lblIfGreater);
		}
		opcode_label(&binarySearchIRS, _JMP_IMMI, lblOut);
	}

	//Connect all IRS together
	irsOut->splice(irsOut->end(), *switchExpIRS);
	irsOut->splice(irsOut->end(), binarySearchIRS);
	for (SwitchBlock *sb : switchStructure->switchBlocks) {
		irsOut->splice(irsOut->end(), sb->irs);
	}

	//End of switch statement
	opcode(irsOut, _NOP, lblBreak);
}

void createConstantSerialSearch(vector<ConstantCondition> *constantConditions, int reg, uint32 lblOut, uint32 lblBreak, SwitchStructure *switchStructure, list<IntermediateRepresentation> *switchExpIRS, list<IntermediateRepresentation> *irsOut) {
	//Create serial search IRS
	list<IntermediateRepresentation> serialSearchIRS;
	for (ConstantCondition &cc : *constantConditions) {
		opcode_r_r(&serialSearchIRS, _MOV_R_R, reg + 1, reg);
		opcode_r_immi(&serialSearchIRS, _CMPNE_R_IMMI, reg + 1, cc.constValue);
		opcode_r_label(&serialSearchIRS, _JC_R_IMMI, reg + 1, cc.label);
	}
	opcode_label(&serialSearchIRS, _JMP_IMMI, lblOut);

	//Connect all pieces together
	irsOut->splice(irsOut->end(), *switchExpIRS);
	irsOut->splice(irsOut->end(), serialSearchIRS);
	for (SwitchBlock *sb : switchStructure->switchBlocks) {
		irsOut->splice(irsOut->end(), sb->irs);
	}

	//Mark the end of switch statement
	opcode(irsOut, _NOP, lblBreak);
}

void createSerialSearch(TokenLine *switchExpression, SwitchStructure *switchStructure, IdentifierDataType switchExpDataType, bool switchExpIsCompare, uint32 lblOut, uint32 lblBreak, list<IntermediateRepresentation> *switchExpIRS, list<IntermediateRepresentation> *irsOut) {
	if (switchExpDataType > DATA_TYPE_STRING) {
		LockableException e(ERR_STR_INVALID_SWITCH_EXP + switchExpression->toString());
		e.lock();
		throw e;
	}
		
	//Parse switch Expression
	int reg;
	int switchExpStringPointer = 1, conditionStringPointer = 2;	//Will be used only if switch expression was of string type
	if (switchExpDataType != DATA_TYPE_STRING) {
		reg = parseExpression(switchExpression, switchExpDataType, Token(TOKEN_OPERATOR, KW_NONE, OPERATOR_SEMI_COLON, ";"), 
			function->identsSize, 1, 1, switchExpIsCompare, switchExpIRS);
	} else {
		//Create two string object for switch expression and case expression
		opcode_immi_immi8(switchExpIRS, _OUT_IMMI_IMMI8, 64, 0);
		opcode_r_immi(switchExpIRS, _IN_R64_IMMI, switchExpStringPointer, 40);
		opcode_immi_immi8(switchExpIRS, _OUT_IMMI_IMMI8, 64, 0);
		opcode_r_immi(switchExpIRS, _IN_R64_IMMI, conditionStringPointer, 40);
		//Let's push these register so we can delete the string object later
		opcode_r(switchExpIRS, _PUSH_R, switchExpStringPointer);
		opcode_r(switchExpIRS, _PUSH_R, conditionStringPointer);

		opcode_r_r(switchExpIRS, _MOV_R_R, REG_STRING_POINTER, switchExpStringPointer);
		parseString(switchExpression, 0, 2, 1, function->identsSize, Token(TOKEN_OPERATOR, KW_NONE, OPERATOR_SEMI_COLON, ";"),
			switchExpIRS);
		opcode_r_r(switchExpIRS, _MOV_R_R, REG_STRING_POINTER, conditionStringPointer);
	}

	if (switchExpDataType == DATA_TYPE_STRING) {
		for (SwitchBlock *switchBlock : switchStructure->switchBlocks) {
			if (switchBlock->getType() == SWITCH_BLOCK_TYPE_CASE) {
				CaseBlock *caseBlock = (CaseBlock*)switchBlock;
				for (CaseCondition &cc : caseBlock->caseConditions) {
					if (cc.isConstant) {
						//We need to create a token line
						TokenLine constantExp;
						constantExp.tokens.push_back(Token(TOKEN_NUMBER, KW_NONE, OPERATOR_NONE, intToStr(cc.constValue)));
						constantExp.tokens.push_back(Token(TOKEN_OPERATOR, KW_NONE, OPERATOR_SEMI_COLON, ";"));

						//Now parse it as string
						parseString(&constantExp, 0, 3, 1, function->identsSize, 
							Token(TOKEN_OPERATOR, KW_NONE, OPERATOR_SEMI_COLON, ";"), switchExpIRS);
					} else {
						parseString(&cc.value, 0, 3, 1, function->identsSize, 
							Token(TOKEN_OPERATOR, KW_NONE, OPERATOR_SEMI_COLON, ";"), switchExpIRS);
					}

					//Compare strings in switchExpStringPointer and conditionStringPointer
					opcode_immi_r(switchExpIRS, _OUT_IMMI_R64, 40, switchExpStringPointer);
					opcode_immi_r(switchExpIRS, _OUT_IMMI_R64, 48, conditionStringPointer);
					opcode_immi_immi8(switchExpIRS, _OUT_IMMI_IMMI8, 64, 11);
					opcode_r_immi(switchExpIRS, _IN_R8_IMMI, 3, 56);

					//Jump if equal
					opcode_r_immi(switchExpIRS, _XOR_R_IMMI, 3, 1);
					opcode_r_immi(switchExpIRS, _JC_R_IMMI, 3, caseBlock->label);
				}
			}
		}
	} else {
		for (SwitchBlock *switchBlock : switchStructure->switchBlocks) {
			if (switchBlock->getType() == SWITCH_BLOCK_TYPE_CASE) {
				CaseBlock *caseBlock = (CaseBlock*)switchBlock;
				for (CaseCondition &cc : caseBlock->caseConditions) {
					
					TokenLine constantExp;
					if (cc.isConstant) {
						opcode_r_r(switchExpIRS, _MOV_R_R, reg + 1, reg);
						opcode_r_immi(switchExpIRS, _CMPNE_R_IMMI, reg + 1, cc.constValue);
						opcode_r_immi(switchExpIRS, _JC_R_IMMI, reg + 1, caseBlock->label);
					} else {
						//Get the data type of the case expression
						bool conditionIsCompare;
						IdentifierDataType caseExpDataType = getExpressionDataType(&cc.value, 0, switchExpIsCompare);

						if (caseExpDataType > switchExpDataType) {
							LockableException e(ERR_STR_INVALID_CASE_EXP + cc.value.toString());
							e.lock();
							throw e;
						}

						//Parse the expression
						int conditionReg = parseExpression(&cc.value, switchExpDataType, 
							Token(TOKEN_OPERATOR, KW_NONE, OPERATOR_SEMI_COLON, ";"),
							function->identsSize, 2, 1, conditionIsCompare, switchExpIRS);

						//Do the comparision
						if (switchExpDataType == DATA_TYPE_FLOAT) {
							opcode_r_fr_fr(switchExpIRS, _CMPNE_R_FR_FR, 1, reg, conditionReg);
							opcode_r_immi(switchExpIRS, _JC_R_IMMI, 1, caseBlock->label);
						} else {
							opcode_r_r(switchExpIRS, _CMPNE_R_R, conditionReg, reg);
							opcode_r_immi(switchExpIRS, _JC_R_IMMI, conditionReg, caseBlock->label);
						}
					}
				}
			}
		}
	}
	opcode_label(switchExpIRS, _JMP_IMMI, lblOut);

	//Connect all pieces together
	irsOut->splice(irsOut->end(), *switchExpIRS);
	for (SwitchBlock *sb : switchStructure->switchBlocks) {
		irsOut->splice(irsOut->end(), sb->irs);
	}

	//Mark the end of switch statement
	opcode(irsOut, _NOP, lblBreak);

	if (switchExpDataType == DATA_TYPE_STRING) {
		//We have to delete the two string objects that we created earlier to store switch expression and condition expression
		opcode_r(irsOut, _POP_R, switchExpStringPointer);
		opcode_r(irsOut, _POP_R, conditionStringPointer);
		opcode_immi_r(irsOut, _OUT_IMMI_R64, 40, switchExpStringPointer);
		opcode_immi_immi8(irsOut, _OUT_IMMI_IMMI8, 64, 1);
		opcode_immi_r(irsOut, _OUT_IMMI_R64, 40, conditionStringPointer);
		opcode_immi_immi8(irsOut, _OUT_IMMI_IMMI8, 64, 1);
	}
}