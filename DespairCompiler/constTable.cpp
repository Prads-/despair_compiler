/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include "constTable.h"
#include "lockableException.h"
#include "errorStrings.h"
using namespace std;

void ConstTable::createConstantTable(ConstantTable *tblOut, std::vector<std::vector<TokenLine> > *tokenLines) {
	TokenLine *tLine;
	
	for (size_t lineIndex = 0; lineIndex < tokenLines->size(); ++lineIndex) {
		for (size_t subModLineIndex = 0; subModLineIndex < (*tokenLines)[lineIndex].size(); ++subModLineIndex) {
			if ((*tokenLines)[lineIndex][subModLineIndex].type == TOKEN_LINE_CONST_DECLARE) {
				try {
					tLine = &(*tokenLines)[lineIndex][subModLineIndex];
					tblOut->insert(ConstantTable::value_type(tLine->tokens.at(1).value, tLine->tokens.at(3)));
					(*tokenLines)[lineIndex].erase((*tokenLines)[lineIndex].begin() + subModLineIndex);
					--subModLineIndex;
				} catch (out_of_range) {
					throw LockableException(ERR_STR_SYNTAX_ERROR + string(" in line ") + tLine->toString());
				}
			}
		}
	}
}
