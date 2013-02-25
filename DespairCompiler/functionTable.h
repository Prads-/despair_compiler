/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef FUNCTION_TABLE_H
#define FUNCTION_TABLE_H

#include <string>
#include <map>
#include <vector>
#include <list>
#include "identifierDataType.h"
#include "declarations.h"
#include "identTable.h"
#include "intermediateRepresentation.h"

struct Function {
	uint32 functionID, moduleID;
	IdentifierDataType returnType;
	std::string returnTypeStr, name, stdBinDirName;
	uint32 address, identsSize, codeSize;
	bool isCalled, isParsed, isStdBinLib, isPublic;
	std::vector<Ident> parameters;
	IdentTable identTable;
	std::list<IntermediateRepresentation> prologue, irs, epilogue;

	Function() {
		isCalled = false;
		isParsed = false;
		isStdBinLib = false;
		isPublic = false;
		returnTypeStr = "";
		identsSize = 0;
	}

	bool operator == (const Function &function) const {
		if (this->name != function.name) return false; 
		if (function.parameters.size() != this->parameters.size()) return false;
		for (uint32 i = 0; i < this->parameters.size(); ++i) {
			if (this->parameters[i].ptrIsWeak && function.parameters[i].isPointer) {
				continue;
			}
			if (this->parameters[i].dataType != function.parameters[i].dataType) {
					return false;
			}
			if (this->parameters[i].dataType == DATA_TYPE_GROUP) {
				if (this->parameters[i].dataTypeStr != function.parameters[i].dataTypeStr) {
					return false;
				}
			}
		}
		return true;
	}
};

typedef std::multimap<std::string, Function> FunctionTable;

#endif