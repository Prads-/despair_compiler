/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include <vector>
#include "preprocessor.h"
#include "lockableException.h"
#include "errorStrings.h"
#include "identifierDataType.h"
#include "stringExtension.h"
using namespace std;
using namespace IdentDataType;
using namespace StringExtension;

void replaceSizeOf(TokenLine *tokenLine, size_t tokenIndex, const GroupTable *groupTable);

void Preprocessor::preprocess(TokenModule *tokenModule, const GroupTable *groupTable, const ConstantTable *constTable) {
	for (TokenModule::iterator tModIt = tokenModule->begin(); tModIt != tokenModule->end(); ++tModIt) {
		for (size_t lineIndex = 0; lineIndex != tModIt->second.tokenLines.size(); ++lineIndex) {
			TokenLine *line = &tModIt->second.tokenLines[lineIndex];
			
			for (size_t i = 0; i < line->tokens.size(); ++i) {
				if (line->tokens[i].type == TOKEN_IDENTIFIER) {
					if (line->tokens[i].value == "sizeof") {
						replaceSizeOf(line, i, groupTable);
					} else {
						ConstantTable::const_iterator ctIt = constTable->find(line->tokens[i].value.c_str());
						if (ctIt != constTable->end()) {
							line->tokens[i] = ctIt->second;
						}
					}
				}
			}
		}
	}
}

void replaceSizeOf(TokenLine *tokenLine, size_t tokenIndex, const GroupTable *groupTable) {
	int size;
	IdentifierDataType dataType;
	size = kwToDataType(tokenLine->tokens.at(tokenIndex + 2).keyword, dataType);
	if (dataType == DATA_TYPE_GROUP) {
		string groupName = tokenLine->tokens.at(tokenIndex + 2).value;
		GroupTable::const_iterator group = groupTable->find(groupName);
		if (group == groupTable->end()) {
			throw LockableException(ERR_STR_GROUP_NOT_EXIST	+ groupName + " in line " + tokenLine->toString());
		}
		size = group->second.size;
	}
	tokenLine->tokens.erase(tokenLine->tokens.begin() + tokenIndex, tokenLine->tokens.begin() + tokenIndex + 4);
	string sizeStr = "";
	strAppendInteger(size, &sizeStr);
	tokenLine->tokens.insert(tokenLine->tokens.begin() + tokenIndex, Token(TOKEN_NUMBER, KW_NONE, OPERATOR_NONE, sizeStr));
}