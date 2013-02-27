/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include <cstdlib>
#include "symbolTable.h"
#include "lockableException.h"
#include "stdBinLib.h"
using namespace std;
using namespace IdentDataType;
using namespace StdBinLib;
using namespace IdentTableFunctions;

void createFunctionTable(const TokenModule *tokenModule, const GroupTable *groupTable, FunctionTable *functionTable);
void getParameters(const TokenLine *tokenLines, const GroupTable *groupTable, Function *function, uint32 counter);
void checkFunctionDefinition(const TokenModule *tokenModule, const FunctionTable *functionTable);

uint32 resolveConstCharIndex(ConstCharTable *constCharTable, int globalIndexOffset);
void createIdentTable(TokenModule *tokenModule, const GroupTable *groupTable, FunctionTable *functionTable, uint32 globalIndexOffset);
void getIdents(vector<TokenLine> *tokenLines, const GroupTable *groupTable, IdentTable *identTableOut, uint32 &index, bool globalModule);

void SymbolTable::createSymbolTable(TokenModule *tokenModule, const GroupTable *groupTable, FunctionTable *functionTable, ConstCharTable *constCharTable, int globalIndexOffset) {
	createFunctionTable(tokenModule, groupTable, functionTable);
	createIdentTable(tokenModule, groupTable, functionTable, resolveConstCharIndex(constCharTable, globalIndexOffset));
}

void createFunctionTable(const TokenModule *tokenModule, const GroupTable *groupTable, FunctionTable *functionTable) {
	for (TokenModule::const_iterator it = tokenModule->begin(); it != tokenModule->end(); ++it) {
		Function function;
		function.functionID = it->second.subModID;
		function.moduleID = it->second.moduleID;
		function.name = it->second.name;
		function.address = 0;	//Will be resolved in code generation stage
		if (it->second.name != "") { //Not a Global Sub Module
			uint32 counter;
			if (it->second.tokenLines[0].tokens.at(0).keyword == KW_PUBLIC) {
				counter = 4;
				function.isPublic = true;
				kwToDataType(it->second.tokenLines[0].tokens.at(1).keyword, function.returnType);
				if (function.returnType == DATA_TYPE_GROUP) function.returnTypeStr = it->second.tokenLines[0].tokens.at(1).value;
			} else {
				counter = 3;
				kwToDataType(it->second.tokenLines[0].tokens.at(0).keyword, function.returnType);
				if (function.returnType == DATA_TYPE_GROUP) function.returnTypeStr = it->second.tokenLines[0].tokens.at(0).value;
			}
			getParameters(&it->second.tokenLines[0], groupTable, &function, counter);
		} else {
			function.returnType = DATA_TYPE_VOID;
			function.isCalled = true;
		}
		functionTable->insert(FunctionTable::value_type(function.name, function));
	}

	getStdBinLib(functionTable);
	
	checkFunctionDefinition(tokenModule, functionTable);
}

//Reads all the parameters in function definition, calculates their size and index, and puts it in function structure
void getParameters(const TokenLine *tokenLines, const GroupTable *groupTable, Function *function, uint32 counter) {
	try {
		Ident param;
		param.isPointer = false;
		uint32 index = 0;

		//Get the params
		while (tokenLines->tokens.at(counter).operation != OPERATOR_CLOSE_ROUND_BRACKET) {
			if (tokenLines->tokens.at(counter).type == TOKEN_KEYWORD) {
				if (tokenLines->tokens.at(counter).keyword == KW_POINTER) {
					param.isPointer = true;
					++counter;
					continue;
				}
				param.size = kwToDataType(tokenLines->tokens.at(counter).keyword, param.dataType);
				if (param.size == 0) throw LockableException();
				param.dataTypeStr = "";
				param.index = index;
			} else if (tokenLines->tokens.at(counter).type == TOKEN_IDENTIFIER) { //Group
				GroupTable::const_iterator group = groupTable->find(tokenLines->tokens.at(counter).value);
				if (group == groupTable->end()) throw LockableException();
				param.dataType = DATA_TYPE_GROUP;
				param.dataTypeStr = group->second.name;
				param.size = group->second.size;
				param.index = index;
			}
			if (tokenLines->tokens.at(++counter).type != TOKEN_IDENTIFIER) throw LockableException();
			//Override the size if param is pointer
			if (param.isPointer) param.size = 8;
			index += param.size;
			param.name = tokenLines->tokens.at(counter).value;
			param.totalSize = param.size;
			function->parameters.push_back(param);
			param.isPointer = false;
			++counter;
			if (tokenLines->tokens.at(counter).operation == OPERATOR_COMMA) {
				++counter;
			}
		}
	} catch (LockableException &e) {
		string errMsg;
		errMsg = ERR_STR_SYNTAX_ERROR + tokenLines->toString();
		e.appendMessage(errMsg);
		throw e;
	}
}

//Checks whether there are any ambiguous function definition
void checkFunctionDefinition(const TokenModule *tokenModule, const FunctionTable *functionTable) {
	for (TokenModule::const_iterator tModIt = tokenModule->begin(); tModIt != tokenModule->end(); ++tModIt) {
		if (tModIt->second.name == "") continue;
		uint32 count = functionTable->count(tModIt->second.name);
		if (count > 1) {
			pair<FunctionTable::const_iterator, FunctionTable::const_iterator> fTRangeIt;
			fTRangeIt = functionTable->equal_range(tModIt->second.name);
			for (FunctionTable::const_iterator it1 = fTRangeIt.first; count > 1; --count, ++it1) {
				FunctionTable::const_iterator it2 = it1;
				for (++it2; it2 != fTRangeIt.second; ++it2) {
					if (it1->second.moduleID == it2->second.moduleID && it1->second == it2->second) {
						string errMsg = ERR_STR_AMBIGUOUS_OVERLOAD + it1->second.name;
						throw LockableException(errMsg);
					}
				}
			}
		}
		
	}
}

//Give appropriate index to constant characters
uint32 resolveConstCharIndex(ConstCharTable *constCharTable, int globalIndexOffset) {
	uint32 index = globalIndexOffset;

	for (ConstCharTable::iterator it = constCharTable->begin(); it != constCharTable->end(); ++it) {
		it->second.index = index;
		index += it->second.val.size() + 1;
	}

	return index;
}

void createIdentTable(TokenModule *tokenModule, const GroupTable *groupTable, FunctionTable *functionTable, uint32 globalIndexOffset) {
	for (TokenModule::iterator tModIt = tokenModule->begin(); tModIt != tokenModule->end(); ++tModIt) {
		uint32 index = 0;
		Function *function;
		pair<FunctionTable::iterator, FunctionTable::iterator> fTRangeIt;
		fTRangeIt = functionTable->equal_range(tModIt->second.name);
		for (FunctionTable::iterator it = fTRangeIt.first; it != fTRangeIt.second; ++it) {
			if (it->second.functionID == tModIt->second.subModID) {
				function = &it->second;
				break;
			}
		}

		//If ident reside in global address space
		if (function->name == "") {
			index = globalIndexOffset;
		}

		//Put all parameters in ident table
		for (uint32 i = 0; i < function->parameters.size(); ++i) {
			insertIdentInLocalTable(&function->identTable, function->parameters[i]);
		}
		if (function->parameters.size() != 0) {
			index = function->parameters[function->parameters.size() - 1].index + function->parameters[function->parameters.size() - 1].size;
		}

		//If the return type of the function is string, create an extra string object
		if (function->returnType == DATA_TYPE_STRING) {
			Ident ident;
			ident.dataType = DATA_TYPE_STRING;
			ident.isPointer = false;
			ident.name = "";
			ident.size = 8;
			ident.index = index;
			index += 8;
			function->identTable.insert(IdentTable::value_type(ident.name, ident));
		}

		//Get rest of the idents declared in the function
		getIdents(&tModIt->second.tokenLines, groupTable, &function->identTable, index, (function->name == "") ? true : false);
		
		//Calculate the total size of all idents in this function
		function->identsSize = 0;
		for (IdentTable::const_iterator it = function->identTable.begin(); it != function->identTable.end(); ++it) {
			function->identsSize += it->second.totalSize;
		}
	}
}

void getIdents(vector<TokenLine> *tokenLines, const GroupTable *groupTable, IdentTable *identTableOut, uint32 &index, bool globalModule) {
	uint32 i = (globalModule) ? 0 : 1;
	while (i < tokenLines->size()) {
		try {
			if ((*tokenLines)[i].type == TOKEN_LINE_VAR_DECLARE) {
				uint32 counter = 0;
				Ident ident;
				
				if (globalModule) {
					ident.moduleID =  (*tokenLines)[i].moduleID;
					if ((*tokenLines)[i].tokens[counter].keyword == KW_PUBLIC) {
						ident.isPublic = true;
						++counter;
					}
				}
				
				if ((*tokenLines)[i].tokens[counter].keyword == KW_POINTER) {
					ident.isPointer = true;
					++counter;
				}

				ident.size = kwToDataType((*tokenLines)[i].tokens[counter].keyword, ident.dataType);
				if (ident.size == 0) {		//if ident is of group type
					GroupTable::const_iterator group = groupTable->find((*tokenLines)[i].tokens[counter].value);
					if (group == groupTable->end()) {
						string errMsg = ERR_STR_UNKNOWN_TYPE + (*tokenLines)[i].toString();
						throw LockableException(errMsg);
					}
					ident.dataType = DATA_TYPE_GROUP;
					ident.dataTypeStr = group->second.name;
					ident.size = group->second.size;
				}
				if (ident.isPointer) ident.size = 8; //Override the size if pointer
				++counter;
				
				//Get all identifiers declared in a line
				while (true) {
					if ((*tokenLines)[i].tokens.at(counter).type == TOKEN_IDENTIFIER) {
						ident.name = (*tokenLines)[i].tokens[counter++].value;

						//Is it an array
						int arraySize = 1;
						if ((*tokenLines)[i].tokens[counter].operation == OPERATOR_OPEN_SQUARE_BRACKET) {
							if (ident.isPointer) {
								throw LockableException(ERR_STR_POINTER_ARRAY + (*tokenLines)[i].toString());
							}
							ident.isArray = true;
							arraySize = atoi((*tokenLines)[i].tokens[++counter].value.c_str());
							if (arraySize == 0) {
								throw LockableException(ERR_STR_INVALID_ARRAY_SIZE + (*tokenLines)[i].toString());
							}
							counter += 2;
						}

						ident.index = index;
						ident.totalSize = arraySize * ident.size;
						index += ident.totalSize;
						
						if (globalModule) {
							insertIdentInGlobalTable(identTableOut, ident);
						} else {
							insertIdentInLocalTable(identTableOut, ident);
						}
						
						if ((*tokenLines)[i].tokens.at(counter).operation == OPERATOR_ASSIGNMENT) {
							TokenLine tokenLine;
							tokenLine.type = TOKEN_LINE_ASSIGNMENT;
							--counter;
							while (true) {
								if ((*tokenLines)[i].tokens.at(counter).operation == OPERATOR_COMMA || 
									(*tokenLines)[i].tokens.at(counter).operation == OPERATOR_SEMI_COLON) {
									break;
								} else if ((*tokenLines)[i].tokens.at(counter).operation == OPERATOR_OPEN_ROUND_BRACKET) {
									int bracketLevel = 0;
									while (true) {
										tokenLine.tokens.push_back((*tokenLines)[i].tokens.at(counter));
										if ((*tokenLines)[i].tokens.at(counter).operation == OPERATOR_OPEN_ROUND_BRACKET) {
											++bracketLevel;
										} else if ((*tokenLines)[i].tokens.at(counter).operation == OPERATOR_CLOSE_ROUND_BRACKET) {
											--bracketLevel;
											if (bracketLevel == 0) {
												++counter;
												break;
											}
										}
										++counter;
									}
								} else {
									tokenLine.tokens.push_back((*tokenLines)[i].tokens.at(counter++));
								}
							}
							tokenLine.tokens.push_back(Token(TOKEN_OPERATOR, KW_NONE, OPERATOR_SEMI_COLON, ";"));
							tokenLines->insert(tokenLines->begin() + i, tokenLine);
							++i;
						} 
						if ((*tokenLines)[i].tokens.at(counter).operation == OPERATOR_COMMA) {
							++counter;
						} else if ((*tokenLines)[i].tokens.at(counter).operation == OPERATOR_SEMI_COLON) {
							tokenLines->erase(tokenLines->begin() + i);
							break;
						} else {
							throw out_of_range("");
						}
						ident.isArray = false;
					} else {
						throw out_of_range("");
					}
				}
			} else {
				++i;
			}
		} catch (out_of_range) {
			string errMsg = ERR_STR_SYNTAX_ERROR + (*tokenLines)[i].toString();
			throw LockableException(errMsg);
		}
	}
}
