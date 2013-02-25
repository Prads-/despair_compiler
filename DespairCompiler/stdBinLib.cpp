/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include <fstream>
#include "stdBinLib.h"
#include "errorStrings.h"
#include "lockableException.h"
#include "identifierDataType.h"
#include "stringExtension.h"
using namespace std;
using namespace StringExtension;

IdentifierDataType strToDataType(string str, int &size) {
	if (str == "void") {
		size = 0;
		return DATA_TYPE_VOID;
	} else if (str == "byte") {
		size = 1;
		return DATA_TYPE_BYTE;
	} else if (str == "int") {
		size = 4;
		return DATA_TYPE_INT;
	} else if (str == "float") {
		size = 4;
		return DATA_TYPE_FLOAT;
	} else if (str == "string") {
		size = 8;
		return DATA_TYPE_STRING;
	} else if (str == "file") {
		size = 8;
		return DATA_TYPE_FILE;
	} else if (str == "image") {
		size = 8;
		return DATA_TYPE_IMAGE;
	} else {
		throw LockableException(ERR_STR_STD_BIN_LIB_UNKWN_TYPE + str);
	}
}

void StdBinLib::getStdBinLib(FunctionTable *funcTable) {
	//Get the highest functionID in function table
	uint32 functionIDCounter = 0;
	for (FunctionTable::const_iterator it = funcTable->begin(); it != funcTable->end(); ++it) {
		if (functionIDCounter < it->second.functionID) functionIDCounter = it->second.functionID;
	}
	++functionIDCounter;

	ifstream stdBinLibListFile("stdbinlib/stdbinlib");	//Std Bin Lib list file
	if (stdBinLibListFile.is_open()) {
		string binLibDir;
		//Get the Standard Binary Library directory names and definitions
		while(getLineFromFile(&stdBinLibListFile, &binLibDir)) {
			if (binLibDir[binLibDir.size() - 1] == '\r') {
				binLibDir.resize(binLibDir.size() - 1);
			}
			
			Function function;
			string definitionPath = "stdbinlib/" + binLibDir + "/definition";
			ifstream definition(definitionPath.c_str());

			//Get return type name and parameters
			if (definition.is_open()) {
				string dataType, name;

				getLineFromFile(&definition, &dataType);
				getLineFromFile(&definition, &name);

				int size;
				function.returnType = strToDataType(dataType, size);
				function.name = name;
				function.stdBinDirName = binLibDir;

				//Get parameters
				int paramCounter = 0;
				string paramLine;
				int index = 0;
				while(getLineFromFile(&definition, &paramLine)) {
					Ident param;
					string token = "";

					for (int i = 0; i < paramLine.size(); ++i) {
						if (paramLine[i] != ' ') {
							token += paramLine[i];
						} else {
							if (token == "pointer") {
								param.isPointer = true;
								token = "";
							} else {
								break;
							}
						}
					}

					param.dataType = strToDataType(token, size);
					param.name = "p";
					strAppendInteger(paramCounter, &param.name);
					if (param.isPointer == true) {
						param.size = 8;
					} else {
						param.size = size;
					}
					param.index = index;
					index += param.size;

					function.functionID = functionIDCounter++;
					function.parameters.push_back(param);
				}

				function.isStdBinLib = true;
				function.isParsed = true;
				function.isPublic = true;
				funcTable->insert(FunctionTable::value_type(function.name, function));
			} else {
				throw LockableException(ERR_STR_STD_BIN_LIB_DEF_NOT_FND + binLibDir);
			}

			definition.close();
		}
	} else {
		throw LockableException(ERR_STR_STD_BIN_LIB_LST_NOT_FND);
	}
}
