/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include <cstdlib>
#include "groupTable.h"
#include "errorStrings.h"
#include "lockableException.h"
using namespace std;
using namespace IdentDataType;
using namespace IdentTableFunctions;

void closeScope(const Group *group, GroupTable *groupTableOut);
void varDeclare(const TokenLine *tokenLine, Group *group);
void breakDownGroup(GroupTable *groupTable, Group *group, const set<int> *groupIDs);
void resolveInheritance(GroupTable *groupTable, Group *group, set<int> *groupIDs);

static int nextGroupID = 0;

void GrpTable::addGroupToTable(const std::vector<TokenLine> *tokenLines, uint32 &counter, GroupTable *groupTableOut) {
	Group group;
	const TokenLine *tokenLine = &(*tokenLines)[counter++];
	
	group.groupID = nextGroupID++;
	if (tokenLine->tokens.at(1).type != TOKEN_IDENTIFIER) {
		string errMsg =  ERR_STR_SYNTAX_ERROR + tokenLine->toString();
		throw LockableException(errMsg);		
	}
	if (tokenLine->tokens.at(2).operation == OPERATOR_COLON) {
		group.inheritedFrom = tokenLine->tokens.at(3).value;
		group.isInheritanceResolved = false;
	}

	group.name = tokenLine->tokens[1].value;
	group.size = 0;

	while (true) {
		try {
			tokenLine = &tokenLines->at(counter++);
		} catch (out_of_range) {
			throw LockableException(ERR_STR_NO_MODULE_END);
		}

		switch (tokenLine->type) {
			case TOKEN_LINE_CLOSE_SCOPE:
				closeScope(&group, groupTableOut);
				return;
			case TOKEN_LINE_VAR_DECLARE:
				varDeclare(tokenLine, &group);
				break;
			default:
				string errMsg = ERR_STR_SYNTAX_ERROR + tokenLine->toString();
				throw LockableException(errMsg);
		}
	}
}

void GrpTable::resolveGroups(GroupTable *groupTable) {
	GroupTable::iterator gtIt;
	
	//Resolve inheritance
	try {
		for (gtIt = groupTable->begin(); gtIt != groupTable->end(); ++gtIt) {
			if (!gtIt->second.isInheritanceResolved) {
				set<int> groupIDs;
				groupIDs.insert(gtIt->second.groupID);
				resolveInheritance(groupTable, &gtIt->second, &groupIDs);
			}
		}
	} catch (LockableException &e) {
		e.appendMessage(gtIt->second.name);
		throw;
	}
	
	//Breakdown groups
	for (gtIt = groupTable->begin(); gtIt != groupTable->end(); ++gtIt) {
		if (!gtIt->second.isResolved) {
			set<int> groupIDs;
			groupIDs.insert(gtIt->second.groupID);
			breakDownGroup(groupTable, &gtIt->second, &groupIDs);
		}
	}
}

void closeScope(const Group *group, GroupTable *groupTableOut) {
	pair<GroupTable::iterator, bool> it = groupTableOut->insert(GroupTable::value_type(group->name, *group));
	if (!it.second) {
		string errMsg = ERR_STR_GROUP_EXISTS + group->name;
		throw LockableException(errMsg);
	}
}

void varDeclare(const TokenLine *tokenLine, Group *group) {
	uint32 counter = 0;
	Ident ident;
	
	if (tokenLine->tokens[counter].keyword == KW_POINTER) {
		ident.isPointer = true;
		++counter;
	}

	if (tokenLine->tokens.at(counter).type == TOKEN_KEYWORD) {
		ident.size = kwToDataType(tokenLine->tokens.at(counter).keyword, ident.dataType);
		if (ident.size == 0) throw LockableException();
	} else if (tokenLine->tokens.at(counter).type == TOKEN_IDENTIFIER) {
		ident.dataType = DATA_TYPE_GROUP;
		ident.dataTypeStr = tokenLine->tokens.at(counter).value;
		ident.size = 0;	//Unknown size
	} else {
		throw LockableException();		//This is checked by TokenLiner too
	}
	//Override the size if pointer
	if (ident.isPointer) {
		ident.size = 8;
	}

	++counter;
	while (true) {
		if (tokenLine->tokens.at(counter).type == TOKEN_IDENTIFIER) {
			ident.name = tokenLine->tokens.at(counter++).value;

			//If the ident is an array
			int arraySize = 1;
			if (tokenLine->tokens.at(counter).operation == OPERATOR_OPEN_SQUARE_BRACKET) {
				if (ident.isPointer) {
					throw LockableException(ERR_STR_POINTER_ARRAY + tokenLine->toString());
				}
				ident.isArray = true;
				arraySize = atoi(tokenLine->tokens.at(++counter).value.c_str());
				if (arraySize == 0) {
					throw LockableException(ERR_STR_INVALID_ARRAY_SIZE + tokenLine->toString());
				}
				counter += 2;
			}

			if (ident.dataType == DATA_TYPE_GROUP && !ident.isPointer) {
				ident.totalSize = arraySize;	//Will be recalculated later when we breakdown groups
			} else {
				ident.totalSize = ident.size * arraySize;
			}

			insertIdentInLocalTable(&group->members, ident);
			group->orderedMembers.push_back(ident.name);

			if (tokenLine->tokens.at(counter).operation == OPERATOR_COMMA) {
				++counter;
			} else if (tokenLine->tokens.at(counter).operation == OPERATOR_SEMI_COLON) {
				break;
			} else {
				throw LockableException(ERR_STR_SYNTAX_ERROR + tokenLine->toString());
			}
			ident.isArray = false;
		} else {
			throw LockableException(ERR_STR_SYNTAX_ERROR + tokenLine->toString());
		}
	}
}

void breakDownGroup(GroupTable *groupTable, Group *group, const set<int> *groupIDs) {
	uint32 index = 0;
	group->size = 0;

	try {
		for (list<string>::iterator orderedGMIt = group->orderedMembers.begin(); orderedGMIt != group->orderedMembers.end(); ++orderedGMIt) {
			IdentTable::iterator gmIt = group->members.find(*orderedGMIt);
			if (gmIt->second.dataType == DATA_TYPE_GROUP && !gmIt->second.isPointer) {
				GroupTable::iterator memberGroupIt = groupTable->find(gmIt->second.dataTypeStr);
				if (memberGroupIt == groupTable->end()) {
					string errMsg = ERR_STR_NO_GROUP_REFERENCE;
					throw LockableException(errMsg);
				}
				set<int>::const_iterator it = groupIDs->find(memberGroupIt->second.groupID);
				if (it != groupIDs->end()) {
					string errMsg = ERR_STR_GROUP_CIRCULAR_REF + memberGroupIt->second.name;
					throw LockableException(errMsg);
				}
				
				if (!memberGroupIt->second.isResolved) {
					set<int> newGroupIDs = *groupIDs;
					newGroupIDs.insert(group->groupID);
					breakDownGroup(groupTable, &memberGroupIt->second, &newGroupIDs);
				}
				gmIt->second.size = memberGroupIt->second.size;
				gmIt->second.totalSize *= gmIt->second.size;
			}
			group->size += gmIt->second.totalSize;
			gmIt->second.index = index;
			index += gmIt->second.totalSize;
		}
	} catch (LockableException &e) {
		e.appendMessage(" <- " + group->name);
		throw;
	}

	group->isResolved = true;
}

void resolveInheritance(GroupTable *groupTable, Group *group, set<int> *groupIDs) {
	GroupTable::iterator inheritedFrom = groupTable->find(group->inheritedFrom);
	string inheritedGroupName = "";

	try {	
		if (inheritedFrom == groupTable->end()) {
			LockableException e(ERR_STR_GRP_INHERIT_NOT_EXIST + group->name + " trying to inherit from " + group->inheritedFrom);
			e.lock();
			throw e;
		}

		inheritedGroupName = inheritedFrom->second.name;

		set<int>::iterator groupIdIt = groupIDs->find(inheritedFrom->second.groupID);
		if (groupIdIt != groupIDs->end()) {
			throw LockableException(ERR_STR_GRP_CIRCULAR_INHERIT);
		}

		//If it is inheriting from other group
		if (!inheritedFrom->second.isInheritanceResolved) {
			groupIDs->insert(inheritedFrom->second.groupID);
			resolveInheritance(groupTable, &inheritedFrom->second, groupIDs);
		}

		//Put members in start of the group, in ordered way
		group->orderedMembers.insert(group->orderedMembers.begin(), inheritedFrom->second.orderedMembers.begin(), inheritedFrom->second.orderedMembers.end());
		
		//Now we have to put member in the group one by one. And if any of them already exists, puke out an exception
		for (IdentTable::iterator inheritedMemIt = inheritedFrom->second.members.begin(); inheritedMemIt != inheritedFrom->second.members.end(); ++inheritedMemIt) {
			try {
				insertIdentInLocalTable(&group->members, inheritedMemIt->second);
			} catch (LockableException &e) {
				e.appendMessageAndLock("Group Name:'" + group->name + "' Member Name: '" + inheritedMemIt->second.name + "'(inherited from '" + inheritedGroupName + "')"); 
				throw;
			}
		}
	} catch (LockableException &e) {
		e.appendMessage(inheritedGroupName + " -> ");
		throw;
	}
}
