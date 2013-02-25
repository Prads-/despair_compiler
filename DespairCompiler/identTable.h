/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef IDENT_TABLE_H
#define IDENT_TABLE_H

#include <string>
#include <map>
#include "identifierDataType.h"
#include "declarations.h"

struct Ident {
	IdentifierDataType dataType;
	std::string dataTypeStr, name;
	uint32 size, totalSize;	//size = element size, totalSize = element size * size of array (size of array is 1 if ident is not an array)
	uint32 index;
	bool isPointer, isStatic, isArray;
	bool ptrIsWeak; //This boolean is used for telling the compiler that the pointer argument is passed as weakly typed
	
	//Used only when the ident is global
	bool isPublic;
	uint32 moduleID;

	Ident() {
		isPointer = false;
		isStatic = false;
		isArray = false;
		isPublic = false;
		ptrIsWeak = false;
		dataTypeStr = "";
	}
};

typedef std::multimap<std::string, Ident> IdentTable;

namespace IdentTableFunctions {
	void insertIdentInLocalTable(IdentTable *identTableOut, Ident ident);
	void insertIdentInGlobalTable(IdentTable *identTableOut, Ident ident);
}

#endif