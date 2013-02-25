/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef GROUP_TABLE_H
#define GROUP_TABLE_H

#include <string>
#include <map>
#include <vector>
#include <set>
#include <list>
#include "declarations.h"
#include "tokenLiner.h"
#include "identifierDataType.h"
#include "identTable.h"

struct Group {
	int groupID;
	std::string name, inheritedFrom;
	uint32 size;
	bool isResolved, isInheritanceResolved;
	IdentTable members;
	std::list<std::string> orderedMembers;	//This keeps members in sequential order so that the index of any member can be predicted

	Group() {
		isResolved = false;
		isInheritanceResolved = true;
	}
};

typedef std::map<std::string, Group> GroupTable;

namespace GrpTable {
	void addGroupToTable(const std::vector<TokenLine> *tokenLines, uint32 &counter, GroupTable *groupTableOut);
	void resolveGroups(GroupTable *groupTable);
}

#endif