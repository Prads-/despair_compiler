/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include "functionPrologueEpilogue.h"
#include "instructionsSet.h"
using namespace std;
using namespace IREmitter;

void createStringObject(uint32 index, list<IntermediateRepresentation> *prologueOut, list<uint64> *stringObjectIndexOut, bool isGlobal);
void deleteStringObject(list<IntermediateRepresentation> *epilogueOut, const list<uint64> *stringObjectIndex, bool isGlobal);
void checkGroupForString(string groupName, const GroupTable *grpTable, uint32 index, list<IntermediateRepresentation> *prologueOut, list<uint64> *stringObjectIndexOut, bool isGlobal);

void FunctionPrologueEpilogue::addPrologueEpilogue(FunctionTable *funcTable, const GroupTable *grpTable) {
	for (FunctionTable::iterator funcIt = funcTable->begin(); funcIt != funcTable->end(); ++funcIt) {
		//If we find any string object, we will put its index in this list
		list<uint64> stringObjectIndex;

		//Check if the function is a global module
		bool isGlobal = (funcIt->second.name == "") ? true : false;

		//We won't create a string object if the string object was declared in parameter
		//also if the group is declared in parameter, all the string members in the group need not be allocated either
		//So we create a set for all the string objects in parameter
		set<string> paramObjects;
		for (uint32 i = 0; i < funcIt->second.parameters.size(); ++i) {
			if (!funcIt->second.parameters[i].isPointer && (funcIt->second.parameters[i].dataType == DATA_TYPE_STRING || 
				funcIt->second.parameters[i].dataType == DATA_TYPE_GROUP)) {
					paramObjects.insert(funcIt->second.parameters[i].name);
			}
		}

		//Check for string objects in function and instantiate them
		for (IdentTable::const_iterator identIt = funcIt->second.identTable.begin(); identIt != funcIt->second.identTable.end(); ++identIt) {
			if (identIt->second.isPointer) continue;

			//Check if it is a parameter
			set<string>::iterator paramIt = paramObjects.find(identIt->second.name);
			if (paramIt != paramObjects.end()) {
				if (identIt->second.dataType == DATA_TYPE_STRING) {
					stringObjectIndex.push_back(identIt->second.index);	//We still need to delete this object
				}
				continue;
			}

			if (identIt->second.dataType == DATA_TYPE_STRING) {
				createStringObject(identIt->second.index, &funcIt->second.prologue, &stringObjectIndex, isGlobal);
			} else if (identIt->second.dataType == DATA_TYPE_GROUP) {
				checkGroupForString(identIt->second.dataTypeStr, grpTable, identIt->second.index, &funcIt->second.prologue, &stringObjectIndex, isGlobal);
			}
		}

		deleteStringObject(&funcIt->second.epilogue, &stringObjectIndex, isGlobal);

		//Return
		opcode(&funcIt->second.epilogue, _RET, 0);
	}
}

void createStringObject(uint32 index, list<IntermediateRepresentation> *prologueOut, list<uint64> *stringObjectIndexOut, bool isGlobal) {
	//Instantiate a new string object
	opcode_immi_immi8(prologueOut, _OUT_IMMI_IMMI8, 64, 0);
	opcode_r_immi(prologueOut, _IN_R64_IMMI, 0, 40);
				
	if (isGlobal) {
		//Put the newly created string object in the ident address
		opcode_m_r(prologueOut, _MOVP_M_R, index, 0);
	} else {
		//Get the ident address
		opcode_r_r(prologueOut, _MOV_R_R, REG_INDEX_POINTER, REG_BASE_POINTER);
		opcode_r_immi(prologueOut, _ADD_R_IMMI, REG_INDEX_POINTER, index);

		//Put the newly created string object in the ident address
		opcode_mr_r(prologueOut, _MOVP_MR_R, REG_INDEX_POINTER, 0);
	}

	//Put the ident index to the list
	stringObjectIndexOut->push_back(index);
}

void deleteStringObject(list<IntermediateRepresentation> *epilogueOut, const list<uint64> *stringObjectIndex, bool isGlobal) {
	//Delete all the string objects we created in prologue
	for (list<uint64>::const_iterator strObjIt = stringObjectIndex->begin(); strObjIt != stringObjectIndex->end(); ++strObjIt) {
		if (isGlobal) {
			//Get the pointer to the string object
			opcode_r_m(epilogueOut, _MOVP_R_M, REG_STRING_POINTER, *strObjIt);
		} else {
			//Get the address of string object
			opcode_r_r(epilogueOut, _MOV_R_R, REG_INDEX_POINTER, REG_BASE_POINTER);
			opcode_r_immi(epilogueOut, _ADD_R_IMMI, REG_INDEX_POINTER, *strObjIt);

			//Get the pointer to string object
			opcode_r_mr(epilogueOut, _MOVP_R_MR, REG_STRING_POINTER, REG_INDEX_POINTER);
		}

		//Delete the string object
		opcode_immi_r(epilogueOut, _OUT_IMMI_R64, 40, REG_STRING_POINTER);
		opcode_immi_immi8(epilogueOut, _OUT_IMMI_IMMI8, 64, 1);
	}
}

void checkGroupForString(string groupName, const GroupTable *grpTable, uint32 index, list<IntermediateRepresentation> *prologueOut, list<uint64> *stringObjectIndexOut, bool isGlobal) {
	//Get the group
	GroupTable::const_iterator group = grpTable->find(groupName);
	
	//Iterate through the group members to see if there are any string object
	for (IdentTable::const_iterator gMIt = group->second.members.begin(); gMIt != group->second.members.end(); ++gMIt) {
		if (gMIt->second.dataType == DATA_TYPE_STRING) {
			createStringObject(index + gMIt->second.index, prologueOut, stringObjectIndexOut, isGlobal);
		} else if (gMIt->second.dataType == DATA_TYPE_GROUP) {
			checkGroupForString(gMIt->second.dataTypeStr, grpTable, index + gMIt->second.index, prologueOut, stringObjectIndexOut, isGlobal);
		}
	}
}