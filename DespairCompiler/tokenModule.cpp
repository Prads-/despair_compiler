/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include "tokenModule.h"
#include "groupTable.h"
#include "errorStrings.h"
#include "lockableException.h"
using namespace std;
using namespace GrpTable;

void createSubModule(vector<TokenLine> *tokenLines, TokenModule *tokenModuleOut, uint32 &counter, uint32 moduleID);

void TokenMod::createTokenModule(vector<vector<TokenLine> > *tokenLines, TokenModule *tokenModuleOut, GroupTable *groupTableOut) {
	uint32 lineCounter;
	TokenSubModule tokenGlobalSubModule;
	tokenGlobalSubModule.name = "";
	tokenGlobalSubModule.subModID = 0;
	for (uint32 moduleCounter = 0; moduleCounter < tokenLines->size(); ++moduleCounter) {
		lineCounter = 0;
		while (lineCounter < (*tokenLines)[moduleCounter].size()) {
			switch ((*tokenLines)[moduleCounter][lineCounter].type) {
				case TOKEN_LINE_FUNCTION_DECLARE:
					createSubModule(&(*tokenLines)[moduleCounter], tokenModuleOut, lineCounter, moduleCounter);
					break;
				case TOKEN_LINE_GROUP:
					addGroupToTable(&(*tokenLines)[moduleCounter], lineCounter, groupTableOut);
					break;
				case TOKEN_LINE_VAR_DECLARE:
					(*tokenLines)[moduleCounter][lineCounter].moduleID = moduleCounter;
					tokenGlobalSubModule.tokenLines.push_back((*tokenLines)[moduleCounter][lineCounter++]);
					break;
				default:
					{
						throw LockableException(ERR_STR_INVALID_LINE + (*tokenLines)[moduleCounter][lineCounter].toString());
					}
			}
		}
	}
	tokenModuleOut->insert(TokenModule::value_type(tokenGlobalSubModule.name, tokenGlobalSubModule));
	resolveGroups(groupTableOut);
}

void createSubModule(vector<TokenLine> *tokenLines, TokenModule *tokenModuleOut, uint32 &counter, uint32 moduleID) {
	static uint32 subModID = 1;
	uint32 scopeLevel = 0;
	TokenSubModule tokenSubModule;
	tokenSubModule.subModID = subModID++;
	tokenSubModule.moduleID = moduleID;
	if ((*tokenLines)[counter].tokens[0].keyword == KW_PUBLIC)
		tokenSubModule.name = (*tokenLines)[counter].tokens[2].value;
	else
		tokenSubModule.name = (*tokenLines)[counter].tokens[1].value;
	tokenSubModule.tokenLines.push_back((*tokenLines)[counter++]);

	try {
		while (true) {
			switch (tokenLines->at(counter).type) {
				case TOKEN_LINE_COMPARE:
				case TOKEN_LINE_LOOP:
				case TOKEN_LINE_SWITCH:
					++scopeLevel;
				case TOKEN_LINE_VAR_DECLARE:
				case TOKEN_LINE_FUNCTION_CALL:
				case TOKEN_LINE_ASSIGNMENT:
				case TOKEN_LINE_RETURN:
				case TOKEN_LINE_POINTER_ASSIGNMENT:
				case TOKEN_LINE_DEALLOCATION:
				case TOKEN_LINE_THREAD_START:
				case TOKEN_LINE_BREAK:
				case TOKEN_LINE_CASE:
				case TOKEN_LINE_FCASE:
				case TOKEN_LINE_DEFAULT:
				case TOKEN_LINE_FDEFAULT:
				case TOKEN_LINE_CONTINUE:
					tokenSubModule.tokenLines.push_back((*tokenLines)[counter++]);
					break;
				case TOKEN_LINE_CLOSE_SCOPE:
					tokenSubModule.tokenLines.push_back((*tokenLines)[counter++]);
					if (scopeLevel == 0) {
						tokenModuleOut->insert(TokenModule::value_type(tokenSubModule.name, tokenSubModule));
						return;
					} else {
						--scopeLevel;
					}
					break;
				default:
					{
						string errMsg;
						errMsg = ERR_STR_INVALID_LINE + (*tokenLines)[counter].toString();
						throw LockableException(errMsg);
					}
			}
		}
	} catch (out_of_range) {
		throw LockableException(ERR_STR_NO_MODULE_END);
	}
}
