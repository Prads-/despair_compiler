/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef CONST_CHAR_TABLE_H
#define CONST_CHAR_TABLE_H

#include <map>
#include <string>
#include "declarations.h"

struct ConstChar {
	std::string val;
	uint32 index;

	ConstChar(std::string val) {
		this->val = val;
	}
};

typedef std::map<std::string, ConstChar> ConstCharTable;

#endif