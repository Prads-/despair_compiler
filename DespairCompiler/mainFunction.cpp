/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include "mainFunction.h"
#include "errorStrings.h"
#include "lockableException.h"
#include "intermediateRepresentation.h"
#include "instructionsSet.h"
#include "portAddress.h"
using namespace std;
using namespace IREmitter;

uint32 MainFunction::findMainFunction(const FunctionTable *functionTable) {
	//Make a dummy main Function
	Function dummyMainFunc;
	dummyMainFunc.name = "main";
	dummyMainFunc.returnType = DATA_TYPE_VOID;
	dummyMainFunc.returnTypeStr = "";

	//Search for main function in function table
	pair<FunctionTable::const_iterator, FunctionTable::const_iterator> rangeIt;
	rangeIt = functionTable->equal_range("main");
	for (FunctionTable::const_iterator fIt = rangeIt.first; fIt != rangeIt.second; ++fIt) {
		if (dummyMainFunc == fIt->second && fIt->second.returnType == DATA_TYPE_VOID) {
			return fIt->second.functionID;
		}
	}

	throw LockableException(ERR_STR_NO_MAIN_FUNCTION);
}

Function *MainFunction::getMainFunction(FunctionTable *functionTable, uint32 mainFunctionID) {
	for (FunctionTable::iterator it = functionTable->begin(); it != functionTable->end(); ++it) {
		if (it->second.functionID == mainFunctionID) {
			return &it->second;
		}
	}
}

void MainFunction::fixMainFunction(FunctionTable *functionTable, uint32 mainFuncID) {
	Function *mainFunction = getMainFunction(functionTable, mainFuncID);
	
	//Get global module
	FunctionTable::iterator globalModule = functionTable->find("");

	string mName = mainFunction->name;
	string gName = globalModule->second.name;

	//Delete the return IR from main function
	list<IntermediateRepresentation>::iterator irIt = mainFunction->epilogue.end();
	--irIt;
	mainFunction->epilogue.erase(irIt);
	
	//Create string objects for REG_RETURN_STRING
	opcode_immi_immi8(&mainFunction->prologue, _OUT_IMMI_IMMI8, PORT_STRING_COMMAND, 0);
	opcode_r_immi(&mainFunction->prologue, _IN_R64_IMMI, REG_RETURN_STRING, PORT_STRING_OBJECT);
	
	//Destroy string objects for REG_RETURN_STRING
	opcode_immi_r(&mainFunction->epilogue, _OUT_IMMI_R64, PORT_STRING_OBJECT, REG_RETURN_STRING);
	opcode_immi_immi8(&mainFunction->epilogue, _OUT_IMMI_IMMI8, PORT_STRING_COMMAND, 1);

	//Add prologue, epilogue and body of global module to main function
	if (globalModule->second.prologue.size() != 0) {
		mainFunction->prologue.splice(mainFunction->prologue.begin(), globalModule->second.prologue);
	}
	if (globalModule->second.irs.size() != 0) {
		mainFunction->irs.splice(mainFunction->irs.begin(), globalModule->second.irs);
	}
	if (globalModule->second.epilogue.size() != 0) {
		mainFunction->epilogue.splice(mainFunction->epilogue.end(), globalModule->second.epilogue);
	}
}
