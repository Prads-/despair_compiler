/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include "codeGenerator.h"
#include "sizeAndAddressResolver.h"
#include "errorStrings.h"
#include "lockableException.h"
#include <fstream>
using namespace std;
using namespace SizeAndAddressResolver;
using namespace HeaderGenerator;

void writeOperand(const vector<Operand> *operands, const FunctionTable *functionTable, CodeContainer *codeContainer);

void CodeGenerator::generateCode(FunctionTable *functionTable, uint32 mainFunctionID, CodeContainer *codeOut, HeaderGenerator::DPVMHeader *header, const ConstCharTable *constCharTable, char *font, int fontSize) {
	//Resolve function's size, IRs address and labels
	resolveSizeAndAddress(functionTable);

	uint32 globalDataSize, codeOffset, sizeCheck = 0;
	for (FunctionTable::const_iterator funcIt = functionTable->begin(); funcIt != functionTable->end(); ++funcIt) {
		if (funcIt->second.name == "") {
			//Visual C++ compiler is broken? If I turn optimization "/O2", it generates wrong code for line below. Disabled the optimization("/Od")
			globalDataSize = funcIt->second.identsSize;
		}
		if (!funcIt->second.isCalled) continue;

		//If main function, get its address
		if (funcIt->second.functionID == mainFunctionID) {
			codeOffset = funcIt->second.address;
		}

		sizeCheck += funcIt->second.codeSize;

		//Generate code
		if (funcIt->second.isStdBinLib) {
			ifstream binFile;
			string binPath = "stdbinlib/" + funcIt->second.stdBinDirName + "/bin";
			binFile.open(binPath.c_str(), ios::binary);
			uint8 *code = new uint8[funcIt->second.codeSize];
			binFile.read((char*)code, funcIt->second.codeSize);
			codeOut->writeBig(code, funcIt->second.codeSize);
		} else {
			//Prologue
			for (list<IntermediateRepresentation>::const_iterator irIt = funcIt->second.prologue.begin(); irIt != funcIt->second.prologue.end(); ++irIt) {
				codeOut->writeBack(irIt->opcode);
				writeOperand(&irIt->operands, functionTable, codeOut);
			}
			//Body
			for (list<IntermediateRepresentation>::const_iterator irIt = funcIt->second.irs.begin(); irIt != funcIt->second.irs.end(); ++irIt) {
				codeOut->writeBack(irIt->opcode);
				writeOperand(&irIt->operands, functionTable, codeOut);
			}
			//Epilogue
			for (list<IntermediateRepresentation>::const_iterator irIt = funcIt->second.epilogue.begin(); irIt != funcIt->second.epilogue.end(); ++irIt) {
				codeOut->writeBack(irIt->opcode);
				writeOperand(&irIt->operands, functionTable, codeOut);
			}
		}
	}

	//Check size. If they don't match, there is a compiler bug
	if (sizeCheck != codeOut->getSize()) {
		string errMsg = ERR_STR_COMPILER_BUG;
		errMsg += "Code Generation";
		throw LockableException(errMsg);
	}

	//Generate Header
	generateHeader(codeOut, globalDataSize, codeOffset, constCharTable, font, fontSize, header);
}

void writeOperand(const vector<Operand> *operands, const FunctionTable *functionTable, CodeContainer *codeContainer) {
	for (uint32 i = 0; i < operands->size(); ++i) {
		switch ((*operands)[i].type) {
			case OPERAND_FIMMI:
				codeContainer->writeBack((*operands)[i].fValue);
				break;
			case OPERAND_FLOAT_MEMORY:
			case OPERAND_MEMORY:
			case OPERAND_IMMI:
			case OPERAND_LABEL:
				codeContainer->writeBack<uint32>((*operands)[i].value);
				break;
			case OPERAND_FLOAT_MEMORY_REGISTER:
			case OPERAND_FLOAT_REGISTER:
			case OPERAND_MEMORY_REGISTER:
			case OPERAND_REGISTER:
			case OPERAND_IMMI8:
				codeContainer->writeBack<uint8>((*operands)[i].value);
				break;
			case OPERAND_IMMI16:
				codeContainer->writeBack<uint16>((*operands)[i].value);
				break;
			case OPERAND_IMMI64:
				codeContainer->writeBack<uint64>((*operands)[i].value);
				break;
			case OPERAND_FUNCTION:
				{
					//Get function address
					uint32 functionAddress;
					pair<FunctionTable::const_iterator, FunctionTable::const_iterator> rangeIt;
					rangeIt = functionTable->equal_range((*operands)[i].str);
					for (FunctionTable::const_iterator it = rangeIt.first; it != rangeIt.second; ++it) {
						if (it->second.functionID == (*operands)[i].value) {
							functionAddress = it->second.address;
						}
					}
					codeContainer->writeBack(functionAddress);
				}
		}
	}
}
