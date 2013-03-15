/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef INTERMEDIATE_REPRESENTATION_H
#define INTERMEDIATE_REPRESENTATION_H

#include <vector>
#include <list>
#include <string>
#include "declarations.h"

enum OperandType {
	OPERAND_REGISTER, OPERAND_MEMORY_REGISTER, 
	OPERAND_FLOAT_REGISTER, OPERAND_FLOAT_MEMORY_REGISTER,
	OPERAND_MEMORY, OPERAND_FLOAT_MEMORY,
	OPERAND_IMMI8, OPERAND_IMMI16, OPERAND_IMMI, OPERAND_IMMI64, OPERAND_FIMMI,
	OPERAND_FUNCTION, OPERAND_LABEL
};

struct Operand {
	OperandType type;
	uint64 value;
	float32 fValue;
	std::string str;		//Used to store function name in OPERAND_FUNCTION
};

struct IntermediateRepresentation {
	uint16 opcode;
	uint32 address;
	int label;
	std::vector<Operand> operands;
};

namespace IREmitter {
	void opcode_r_r(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 reg1, uint32 reg2);
	void opcode_r_immi(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 reg, uint32 immi);
	void opcode_mr_r(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 mreg, uint32 reg);
	void opcode_function(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 functionID, std::string functionName);
	void opcode_mr_fr(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 reg, uint32 freg);
	void opcode_fr_mfr(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 fReg, uint32 mfReg);
	void opcode_fr_mr(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 fReg, uint32 mReg);
	void opcode_r_mfr(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 reg, uint32 mfReg);
	void opcode_r_mr(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 reg, uint32 mReg);
	void opcode_mfr_fr(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 mfReg, uint32 fReg);
	void opcode_immi_r(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 immi, uint32 reg);
	void opcode_immi_immi(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 immi1, uint32 immi2);
	void opcode_immi_fr(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 immi, float32 freg);
	void opcode_r_m(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 reg, uint32 mem);
	void opcode_fr_m(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 freg, uint32 mem);
	void opcode_r(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 reg);
	void opcode_immi_fimmi(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 immi, float32 fimmi);
	void opcode_m_fr(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 mem, uint32 freg);
	void opcode_m_r(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 mem, uint32 reg);
	void opcode_fm_fr(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 fmem, uint32 freg);
	void opcode_fm_r(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 fmem, uint32 reg);
	void opcode_mfr_r(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 mfReg, uint32 reg);
	void opcode(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, int label);
	void opcode_r_label(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 reg, uint32 label);
	void opcode_label(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, int label);
	void opcode_immi_immi8(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 immi, uint8 immi8);
	void opcode_fr_fm(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 fReg, uint32 fMem);
	void opcode_r_fm(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 reg, uint32 fMem);
	void opcode_fr_r(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 fReg, uint32 reg);
	void opcode_r_fr(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 reg, uint32 fReg);
	void opcode_fr_fr(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 fReg1, uint32 fReg2);
	void opcode_immi_immi64(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 immi, uint64 immi64);
	void opcode_immi_function(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 immi, uint32 functionID, std::string functionName);
	void opcode_r_mr_immi(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 reg, uint32 mReg, uint32 immi);
	void opcode_fr_mfr_immi(std::list<IntermediateRepresentation> *irsOut, uint16 opcode, uint32 fReg, uint32 mfReg, uint32 immi);
}

#endif