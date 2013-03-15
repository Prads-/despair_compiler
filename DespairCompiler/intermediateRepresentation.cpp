/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include "intermediateRepresentation.h"
using namespace std;

void IREmitter::opcode_r_r(list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 reg1, uint32 reg2) {
	IntermediateRepresentation ir;
	Operand operand;
	
	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_REGISTER;
	operand.value = reg1;
	ir.operands.push_back(operand);
	operand.value = reg2;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_r_immi(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 reg, uint32 immi) {
	IntermediateRepresentation ir;
	Operand operand;
	
	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_REGISTER;
	operand.value = reg;
	ir.operands.push_back(operand);
	operand.type = OPERAND_IMMI;
	operand.value = immi;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_mr_r(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 mreg, uint32 reg) {
	IntermediateRepresentation ir;
	Operand operand;
	
	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_REGISTER;
	operand.value = reg;
	ir.operands.push_back(operand);
	operand.type = OPERAND_MEMORY_REGISTER;
	operand.value = mreg;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_function(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 functionID, std::string functionName) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_FUNCTION;
	operand.value = functionID;
	operand.str = functionName;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_mr_fr(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 reg, uint32 freg) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_MEMORY_REGISTER;
	operand.value = reg;
	ir.operands.push_back(operand);
	operand.type = OPERAND_FLOAT_REGISTER;
	operand.value = freg;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_fr_mfr(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 fReg, uint32 mfReg) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_FLOAT_MEMORY_REGISTER;
	operand.value = mfReg;
	ir.operands.push_back(operand);
	operand.type = OPERAND_FLOAT_REGISTER;
	operand.value = fReg;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_fr_mr(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 fReg, uint32 mReg) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_MEMORY_REGISTER;
	operand.value = mReg;
	ir.operands.push_back(operand);
	operand.type = OPERAND_FLOAT_REGISTER;
	operand.value = fReg;
	ir.operands.push_back(operand);
	
	irsOut->push_back(ir);
}

void IREmitter::opcode_r_mfr(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 reg, uint32 mfReg) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_REGISTER;
	operand.value = reg;
	ir.operands.push_back(operand);
	operand.type = OPERAND_FLOAT_MEMORY_REGISTER;
	operand.value = mfReg;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_r_mr(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 reg, uint32 mReg) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_REGISTER;
	operand.value = reg;
	ir.operands.push_back(operand);
	operand.type = OPERAND_MEMORY_REGISTER;
	operand.value = mReg;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_mfr_fr(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 mfReg, uint32 fReg) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_FLOAT_MEMORY_REGISTER;
	operand.value = mfReg;
	ir.operands.push_back(operand);
	operand.type = OPERAND_FLOAT_REGISTER;
	operand.value = fReg;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_immi_r(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 immi, uint32 reg) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;
	
	operand.type = OPERAND_REGISTER;
	operand.value = reg;
	ir.operands.push_back(operand);
	operand.type = OPERAND_IMMI;
	operand.value = immi;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_immi_immi(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 immi1, uint32 immi2) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_IMMI;
	operand.value = immi1;
	ir.operands.push_back(operand);
	operand.type = OPERAND_IMMI;
	operand.value = immi2;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_immi_fr(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 immi, float32 freg) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_FLOAT_REGISTER;
	operand.value = freg;
	ir.operands.push_back(operand);
	operand.type = OPERAND_IMMI;
	operand.value = immi;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_r_m(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 reg, uint32 mem) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_REGISTER;
	operand.value = reg;
	ir.operands.push_back(operand);
	operand.type = OPERAND_MEMORY;
	operand.value = mem;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_fr_m(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 freg, uint32 mem) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_FLOAT_REGISTER;
	operand.value = freg;
	ir.operands.push_back(operand);
	operand.type = OPERAND_MEMORY;
	operand.value = mem;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_r(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 reg) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_REGISTER;
	operand.value = reg;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_immi_fimmi(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 immi, float32 fimmi) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_IMMI;
	operand.value = immi;
	ir.operands.push_back(operand);
	operand.type = OPERAND_FIMMI;
	operand.fValue = fimmi;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_m_fr(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 mem, uint32 freg) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_FLOAT_REGISTER;
	operand.value = freg;
	ir.operands.push_back(operand);
	operand.type = OPERAND_MEMORY;
	operand.value = mem;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_m_r(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 mem, uint32 reg) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_REGISTER;
	operand.value = reg;
	ir.operands.push_back(operand);
	operand.type = OPERAND_MEMORY;
	operand.value = mem;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_fm_fr(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 fmem, uint32 freg) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_FLOAT_REGISTER;
	operand.value = freg;
	ir.operands.push_back(operand);
	operand.type = OPERAND_FLOAT_MEMORY;
	operand.value = fmem;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_fm_r(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 fmem, uint32 reg) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_REGISTER;
	operand.value = reg;
	ir.operands.push_back(operand);
	operand.type = OPERAND_FLOAT_MEMORY;
	operand.value = fmem;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_mfr_r(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 mfReg, uint32 reg) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_REGISTER;
	operand.value = reg;
	ir.operands.push_back(operand);
	operand.type = OPERAND_FLOAT_MEMORY_REGISTER;
	operand.value = mfReg;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, int label) {
	IntermediateRepresentation ir;
	
	ir.address = 0;
	ir.label = label;
	ir.opcode = opcode;
	
	irsOut->push_back(ir);
}

void IREmitter::opcode_r_label(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 reg, uint32 label) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_REGISTER;
	operand.value = reg;
	ir.operands.push_back(operand);
	operand.type = OPERAND_LABEL;
	operand.value = label;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_label(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, int label) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_LABEL;
	operand.value = label;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_immi_immi8(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 immi, uint8 immi8) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_IMMI;
	operand.value = immi;
	ir.operands.push_back(operand);
	operand.type = OPERAND_IMMI8;
	operand.value = immi8;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_fr_fm(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 fReg, uint32 fMem) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_FLOAT_REGISTER;
	operand.value = fReg;
	ir.operands.push_back(operand);
	operand.type = OPERAND_FLOAT_MEMORY;
	operand.value = fMem;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_r_fm(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 reg, uint32 fMem) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_REGISTER;
	operand.value = reg;
	ir.operands.push_back(operand);
	operand.type = OPERAND_FLOAT_MEMORY;
	operand.value = fMem;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_fr_r(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 fReg, uint32 reg) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_REGISTER;
	operand.value = reg;
	ir.operands.push_back(operand);
	operand.type = OPERAND_FLOAT_REGISTER;
	operand.value = fReg;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_r_fr(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 reg, uint32 fReg) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_REGISTER;
	operand.value = reg;
	ir.operands.push_back(operand);
	operand.type = OPERAND_FLOAT_REGISTER;
	operand.value = fReg;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_fr_fr(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 fReg1, uint32 fReg2) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_FLOAT_REGISTER;
	operand.value = fReg1;
	ir.operands.push_back(operand);
	operand.type = OPERAND_FLOAT_REGISTER;
	operand.value = fReg2;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_immi_immi64(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 immi, uint64 immi64) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_IMMI;
	operand.value = immi;
	ir.operands.push_back(operand);
	operand.type = OPERAND_IMMI64;
	operand.value = immi64;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_immi_function(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 immi, uint32 functionID, string functionName) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_IMMI;
	operand.value = immi;
	ir.operands.push_back(operand);
	operand.type = OPERAND_FUNCTION;
	operand.value = functionID;
	operand.str = functionName;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_r_mr_immi(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 reg, uint32 mReg, uint32 immi) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_REGISTER;
	operand.value = reg;
	ir.operands.push_back(operand);
	operand.type = OPERAND_MEMORY_REGISTER;
	operand.value = mReg;
	ir.operands.push_back(operand);
	operand.type = OPERAND_IMMI;
	operand.value = immi;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}

void IREmitter::opcode_fr_mfr_immi(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 fReg, uint32 mfReg, uint32 immi) {
	IntermediateRepresentation ir;
	Operand operand;

	ir.address = 0;
	ir.label = 0;
	ir.opcode = opcode;

	operand.type = OPERAND_FLOAT_MEMORY_REGISTER;
	operand.value = mfReg;
	ir.operands.push_back(operand);
	operand.type = OPERAND_FLOAT_REGISTER;
	operand.value = fReg;
	ir.operands.push_back(operand);
	operand.type = OPERAND_IMMI;
	operand.value = immi;
	ir.operands.push_back(operand);

	irsOut->push_back(ir);
}