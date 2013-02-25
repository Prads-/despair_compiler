/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef IDENTIFIER_DATA_TYPE_H
#define IDENTIFIER_DATA_TYPE_H

#include "tokenTypes.h"

enum IdentifierDataType {
	DATA_TYPE_VOID = 0, DATA_TYPE_BYTE = 1, DATA_TYPE_INT = 2, DATA_TYPE_FLOAT = 3, DATA_TYPE_STRING = 4, 
	DATA_TYPE_FILE = 5, DATA_TYPE_IMAGE = 6, DATA_TYPE_GROUP = 7
};

namespace IdentDataType {
	int kwToDataType(Keyword kw, IdentifierDataType &dataTypeOut);
}

#endif