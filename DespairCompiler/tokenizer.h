/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <vector>
#include <string>
#include <fstream>
#include "declarations.h"
#include "token.h"
#include "constCharTable.h"

namespace Tokenizer {
	void getTokens(std::string path, std::vector<Token> *tokenOut, ConstCharTable *constCharTableOut);
}

#endif
