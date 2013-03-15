/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include <fstream>
#include <list>
#include "sizeAndAddressResolver.h"
#include "instructionsSet.h"
#include "lockableException.h"
#include "errorStrings.h"
using namespace std;

int getOpcodeSize(uint16 opcode);
void resolveLabels(FunctionTable *functionTable);

void SizeAndAddressResolver::resolveSizeAndAddress(FunctionTable *functionTable) {
	//Resolve function code size and address
	uint32 address = 0;
	for (FunctionTable::iterator funcIt = functionTable->begin(); funcIt != functionTable->end(); ++funcIt) {
		if (!funcIt->second.isCalled) continue;

		funcIt->second.address = address;

		//Find the code size of functions
		if (funcIt->second.isStdBinLib) {
			ifstream binFile;
			string binPath = "stdbinlib/" + funcIt->second.stdBinDirName + "/bin";
			binFile.open(binPath.c_str(), ios::ate);
			if (binFile.is_open()) {
				int size = binFile.tellg();
				address += size;
			} else {
				LockableException(ERR_STR_STD_BIN_LIB_BIN_NOT_FND);
			}
		} else {
			//Prologue
			for (list<IntermediateRepresentation>::iterator irIt = funcIt->second.prologue.begin(); irIt != funcIt->second.prologue.end(); ++irIt) {
				irIt->address = address;
				address += getOpcodeSize(irIt->opcode);
			}
			//Body
			for (list<IntermediateRepresentation>::iterator irIt = funcIt->second.irs.begin(); irIt != funcIt->second.irs.end(); ++irIt) {
				irIt->address = address;
				address += getOpcodeSize(irIt->opcode);
			}
			//Epilogue
			for (list<IntermediateRepresentation>::iterator irIt = funcIt->second.epilogue.begin(); irIt != funcIt->second.epilogue.end(); ++irIt) {
				irIt->address = address;
				address += getOpcodeSize(irIt->opcode);
			}
		}

		funcIt->second.codeSize = address - funcIt->second.address;
	}

	//Resolve labels
	resolveLabels(functionTable);
}

void resolveLabels(FunctionTable *functionTable) {
	//Get labels and their addresses
	map<int, uint32> labelAddressTree;
	for (FunctionTable::iterator funcIt = functionTable->begin(); funcIt != functionTable->end(); ++funcIt) {
		//Body IRs
		for (list<IntermediateRepresentation>::iterator irIt = funcIt->second.irs.begin(); irIt != funcIt->second.irs.end(); ++irIt) {
			if (irIt->label != 0) {
				labelAddressTree.insert(map<int, uint32>::value_type(irIt->label, irIt->address));
			}
		}
	}

	//Change jump opcodes labels with address
	for (FunctionTable::iterator funcIt = functionTable->begin(); funcIt != functionTable->end(); ++funcIt) {
		//Body IRs
		for (list<IntermediateRepresentation>::iterator irIt = funcIt->second.irs.begin(); irIt != funcIt->second.irs.end(); ++irIt) {
			switch (irIt->opcode) {
				case _JMP_IMMI:	//Absolute Jump
					{
						uint32 labelAddress = labelAddressTree.find(irIt->operands.at(0).value)->second;
						irIt->operands.at(0).value = labelAddress;
					}
					break;
				case _JMPR_IMMI:	//Relative Jump
					{
						uint32 labelAddress = labelAddressTree.find(irIt->operands.at(0).value)->second;
						irIt->operands.at(0).value = labelAddress - irIt->address;
					}
					break;
				case _JC_R_IMMI:	//Conditional Absolute Jump
					{
						uint32 labelAddress = labelAddressTree.find(irIt->operands.at(1).value)->second;
						irIt->operands.at(1).value = labelAddress;
					}
					break;
				case _JCR_R_IMMI:	//Conditional Relative Jump
					{
						uint32 labelAddress = labelAddressTree.find(irIt->operands.at(1).value)->second;
						irIt->operands.at(1).value = labelAddress - irIt->address;
					}
			}
		}
	}
}

int getOpcodeSize(uint16 opcode) {
	switch(opcode) {
		case _MOV_MR_IMMI_R:
			return 8;
		case _MOV_R_MR_IMMI:
			return 8;
		case _MOV_MR_IMMI_MR_IMMI:
			return 12;
		case _MOV_MR_IMMI_IMMI:
			return 11;
		case _MOV_R_M:
			return 7;
		case _MOV_M_R:
			return 7;
		case _MOV_R_R:
			return 4;
		case _MOV_M_M:
			return 10;
		case _MOV_MR_R:
			return 4;
		case _MOV_R_MR:
			return 4;
		case _MOV_MR_M:
			return 7;
		case _MOV_M_MR:
			return 7;
		case _MOV_MR_MR:
			return 4;
		case _MOV_R_IMMI:
			return 7;
		case _MOV_M_IMMI:
			return 10;
		case _MOV_MR_IMMI:
			return 7;
		case _ADD_R_R:
			return 4;
		case _ADD_R_IMMI:
			return 7;
		case _SUB_R_R:
			return 4;
		case _SUB_R_IMMI:
			return 7;
		case _MUL_R_R:
			return 4;
		case _MUL_R_IMMI:
			return 7;
		case _DIV_R_R:
			return 4;
		case _DIV_R_IMMI:
			return 7;
		case _MOD_R_R:
			return 4;
		case _MOD_R_IMMI:
			return 7;
		case _AND_R_R:
			return 4;
		case _AND_R_IMMI:
			return 7;
		case _OR_R_R:
			return 4;
		case _OR_R_IMMI:
			return 7;
		case _XOR_R_R:
			return 4;
		case _XOR_R_IMMI:
			return 7;
		case _SHL_R_IMMI8:
			return 4;
		case _SHL_R_R:
			return 4;
		case _SHR_R_IMMI8:
			return 4;
		case _SHR_R_R:
			return 4;
		case _NOP:
			return 2;
		case _JMP_IMMI:
			return 6;
		case _JMPR_IMMI:
			return 6;
		case _JC_R_IMMI:
			return 7;
		case _MOVP_MR_IMMI_R:
			return 8;
		case _MOVP_R_MR_IMMI:
			return 8;
		case _MOVP_MR_IMMI_MR_IMMI:
			return 12;
		case _MOVP_R_M:
			return 7;
		case _MOVP_M_R:
			return 7;
		case _MOVP_R_MR:
			return 4;
		case _MOVP_MR_R:
			return 4;
		case _CALL_IMMI:
			return 6;
		case _RET:
			return 2;
		case _PUSH_R:
			return 3;
		case _POP_R:
			return 3;
		case _DRW_R_R_MR:
			return 3;
		case _OUT_R_IMMI8:
			return 4;
		case _OUT_R_IMMI16:
			return 5;
		case _OUT_R_IMMI32:
			return 7;
		case _OUT_R_IMMI64:
			return 11;
		case _OUT_IMMI_R8:
			return 7;
		case _OUT_IMMI_R16:
			return 7;
		case _OUT_IMMI_R32:
			return 7;
		case _OUT_IMMI_R64:
			return 7;
		case _OUT_R_R8:
			return 4;
		case _OUT_R_R16:
			return 4;
		case _OUT_R_R32:
			return 4;
		case _OUT_R_R64:
			return 4;
		case _OUT_IMMI_IMMI8:
			return 7;
		case _OUT_IMMI_IMMI16:
			return 8;
		case _OUT_IMMI_IMMI32:
			return 10;
		case _OUT_IMMI_IMMI64:
			return 14;
		case _IN_R8_IMMI:
			return 7;
		case _IN_R16_IMMI:
			return 7;
		case _IN_R32_IMMI:
			return 7;
		case _IN_R64_IMMI:
			return 7;
		case _IN_R8_R:
			return 4;
		case _IN_R16_R:
			return 4;
		case _IN_R32_R:
			return 4;
		case _IN_R64_R:
			return 4;
		case _FCON_R_FR:
			return 4;
		case _FCON_FR_R:
			return 4;
		case _FCON_FR_IMMI:
			return 7;
		case _FMOV_MFR_IMMI_FR:
			return 8;
		case _FMOV_FR_MFR_IMMI:
			return 8;
		case _FMOV_MFR_IMMI_MFR_IMMI:
			return 12;
		case _FMOV_MFR_IMMI_FIMMI:
			return 11;
		case _FMOV_FR_FR:
			return 4;
		case _FMOV_FR_FM:
			return 7;
		case _FMOV_FM_FR:
			return 7;
		case _FMOV_FR_MFR:
			return 4;
		case _FMOV_MFR_FR:
			return 4;
		case _FMOV_FM_MFR:
			return 7;
		case _FMOV_MFR_FM:
			return 7;
		case _FMOV_MFR_MFR:
			return 4;
		case _FMOV_FM_FM:
			return 10;
		case _FMOV_FR_FIMMI:
			return 7;
		case _FMOV_FM_FIMMI:
			return 10;
		case _FMOV_MFR_FIMMI:
			return 7;
		case _FADD_FR_FR:
			return 4;
		case _FADD_R_FR:
			return 4;
		case _FADD_FR_R:
			return 4;
		case _FADD_FR_FIMMI:
			return 7;
		case _FADD_R_FIMMI:
			return 7;
		case _FSUB_FR_FR:
			return 4;
		case _FSUB_R_FR:
			return 4;
		case _FSUB_FR_R:
			return 4;
		case _FSUB_FR_FIMMI:
			return 7;
		case _FSUB_R_FIMMI:
			return 7;
		case _FMUL_FR_FR:
			return 4;
		case _FMUL_R_FR:
			return 4;
		case _FMUL_FR_R:
			return 4;
		case _FMUL_FR_FIMMI:
			return 7;
		case _FMUL_R_FIMMI:
			return 7;
		case _FDIV_FR_FR:
			return 4;
		case _FDIV_R_FR:
			return 4;
		case _FDIV_FR_R:
			return 4;
		case _FDIV_FR_FIMMI:
			return 7;
		case _FDIV_R_FIMMI:
			return 7;
		case _FMOD_FR_FR:
			return 4;
		case _FMOD_R_FR:
			return 4;
		case _FMOD_FR_R:
			return 4;
		case _FMOD_FR_FIMMI:
			return 7;
		case _FMOD_R_FIMMI:
			return 7;
		case _BMOV_BR_IMMI_R:
			return 8;
		case _BMOV_R_BR_IMMI:
			return 8;
		case _BMOV_BR_IMMI_BR_IMMI:
			return 12;
		case _BMOV_BR_IMMI_IMMI8:
			return 8;
		case _BMOV_R_BM:
			return 7;
		case _BMOV_BM_R:
			return 7;
		case _BMOV_R_BR:
			return 4;
		case _BMOV_BR_R:
			return 4;
		case _BMOV_BR_BR:
			return 4;
		case _BMOV_BM_BM:
			return 10;
		case _BMOV_BR_IMMI8:
			return 4;
		case _BMOV_BM_IMMI8:
			return 7;
		case _CMPE_R_R:
			return 4;
		case _CMPNE_R_R:
			return 4;
		case _CMPG_R_R:
			return 4;
		case _CMPL_R_R:
			return 4;
		case _CMPGE_R_R:
			return 4;
		case _CMPLE_R_R:
			return 4;
		case _CMPE_R_FR_FR:
			return 5;
		case _CMPNE_R_FR_FR:
			return 5;
		case _CMPG_R_FR_FR:
			return 5;
		case _CMPL_R_FR_FR:
			return 5;
		case _CMPGE_R_FR_FR:
			return 5;
		case _CMPLE_R_FR_FR:
			return 5;
		case _FOUT_IMMI_FR:
			return 7;
		case _FIN_IMMI_FR:
			return 7;
		case _FOUT_IMMI_FIMMI:
			return 10;
		case _PUSHES_R_R:
			return 4;
		case _POPS_R_R:
			return 4;
		case _FPUSHES_FR_FR:
			return 4;
		case _FPOPS_FR_FR:
			return 4;
		case _FPUSH_FR:
			return 3;
		case _FPOP_FR:
			return 3;
	}
}
