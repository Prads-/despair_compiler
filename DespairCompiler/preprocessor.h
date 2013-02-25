/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include "tokenModule.h"
#include "groupTable.h"
#include "constTable.h"

namespace Preprocessor {
	void preprocess(TokenModule *tokenModule, const GroupTable *groupTable, const ConstantTable *constTable);
}

#endif