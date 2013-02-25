/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include "declarations.h"

#ifdef _DEBUG_MODE

#include "debug_disassembler.h"
using namespace std;

std::string Debug_Disassembler::disassemble(const std::list<IntermediateRepresentation> *irs) {
	stringstream disassembly;

	for (list<IntermediateRepresentation>::const_iterator irsIt = irs->begin(); irsIt != irs->end(); ++irsIt) {
		if (irsIt->label) disassembly << irsIt->label << ":" << endl;
		disassembly << opToStr.debug_opcodeToStr(irsIt->opcode);
		for (uint32 i = 0; i < irsIt->operands.size(); ++i) {
			disassembly << "\t";
			switch (irsIt->operands[i].type) {
				case OPERAND_FIMMI:
					disassembly << " float " << irsIt->operands[i].fValue;
					break;
				case OPERAND_FLOAT_MEMORY:
					disassembly << " float (" << irsIt->operands[i].value << ")";
					break;
				case OPERAND_FLOAT_MEMORY_REGISTER:
					disassembly << " float (reg[" << irsIt->operands[i].value << "])";
					break;
				case OPERAND_FLOAT_REGISTER:
					disassembly << " freg[" << irsIt->operands[i].value << "]";
					break;
				case OPERAND_FUNCTION:
					disassembly << " " << irsIt->operands[i].str;
					break;
				case OPERAND_IMMI:
					disassembly << " " << irsIt->operands[i].value;
					break;
				case OPERAND_IMMI16:
					disassembly << " short " << irsIt->operands[i].value;
					break;
				case OPERAND_IMMI64:
					disassembly << " long " << irsIt->operands[i].value;
					break;
				case OPERAND_IMMI8:
					disassembly << " byte " << irsIt->operands[i].value;
					break;
				case OPERAND_LABEL:
					disassembly << " " << irsIt->operands[i].value;
					break;
				case OPERAND_MEMORY:
					disassembly << " (" << irsIt->operands[i].value << ")";
					break;
				case OPERAND_MEMORY_REGISTER:
					disassembly << " (reg[" << irsIt->operands[i].value << "])";
					break;
				case OPERAND_REGISTER:
					disassembly << " reg[" << irsIt->operands[i].value << "]";
			}
			if (i < irsIt->operands.size() - 1) disassembly << ",";
		}
		disassembly << endl;
	}

	return disassembly.str();
}

#endif