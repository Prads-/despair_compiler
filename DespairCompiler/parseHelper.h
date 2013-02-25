/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef PARSE_HELPER_H
#define PARSE_HELPER_H

#include <list>
#include "declarations.h"
#include "identTable.h"
#include "tokenLiner.h"
#include "intermediateRepresentation.h"
#include "groupTable.h"
#include "functionTable.h"

namespace ParseHelper {
	void getFunctionCall(const TokenLine *tokenLine, uint32 &offset, TokenLine *funcCallOut);
	IdentifierDataType getExpressionDataType(const TokenLine *expression, std::string *optDataTypeStrOut, bool &isCompare);
	const Ident *getIdent(const TokenLine *tokenLine, uint32 &offset, int identReg, std::list<IntermediateRepresentation> *optIRSout, uint32 *optIndexOut, bool *optIsIndexNotValid, bool *optIsGlobal, int intRegOffset);
	const Group *getGroup(std::string groupName);
	Function *matchFunctionCall(const TokenLine *functionCall, std::vector<TokenLine> *optParamsOut);
	const Ident *getGroupMember(const TokenLine *tokenLine, uint32 &offset, std::string groupName, int grpMemAddrReg, int intRegOffset, std::list<IntermediateRepresentation> *optIRSout, uint32 *optIndexOut, bool *optIsIndexNotValid);
	void dmaTransfer(int regSrc, int regDest, uint32 size, std::list<IntermediateRepresentation> *irsOut);
	int getDataTypeSize(IdentifierDataType dataType, std::string dataTypeStr);
	void getExpressionInsideBracket(const TokenLine *tokenLine, uint32 &offset, Operator openBracket, Operator closeBracket, TokenLine *tlOut);
}

#endif