/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include "identifierDataType.h"

int IdentDataType::kwToDataType(Keyword kw, IdentifierDataType &dataTypeOut) {
	switch (kw) {
		case KW_BYTE:
			dataTypeOut = DATA_TYPE_BYTE;
			return 1;
		case KW_FILE:
			dataTypeOut = DATA_TYPE_FILE;
			return 8;
		case KW_FLOAT:
			dataTypeOut = DATA_TYPE_FLOAT;
			return 4;
		case KW_IMAGE:
			dataTypeOut = DATA_TYPE_IMAGE;
			return 8;
		case KW_INT:
			dataTypeOut = DATA_TYPE_INT;
			return 4;
		case KW_STRING:
			dataTypeOut = DATA_TYPE_STRING;
			return 8;
		case KW_VOID:
			dataTypeOut = DATA_TYPE_VOID;
			return 0;
		default:
			dataTypeOut = DATA_TYPE_GROUP;
			return 0;
	}
}