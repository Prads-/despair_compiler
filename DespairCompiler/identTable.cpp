/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include "identTable.h"
#include "lockableException.h"
#include "errorStrings.h"
using namespace std;

void IdentTableFunctions::insertIdentInLocalTable(IdentTable *identTableOut, Ident ident) {
	IdentTable::const_iterator it = identTableOut->find(ident.name);
	if (it != identTableOut->end()) {
		throw LockableException(ERR_STR_DUPLICATE_IDENT);
	}
	identTableOut->insert(IdentTable::value_type(ident.name, ident));
}

void IdentTableFunctions::insertIdentInGlobalTable(IdentTable *identTableOut, Ident ident) {
	pair<IdentTable::const_iterator, IdentTable::const_iterator> rangeIt;
	rangeIt = identTableOut->equal_range(ident.name);
	for (IdentTable::const_iterator it = rangeIt.first; it != rangeIt.second; ++it) {
		if (it->second.isPublic || ident.isPublic || it->second.moduleID == ident.moduleID) {
			throw LockableException(ERR_STR_DUPLICATE_IDENT);
		}
	}
	identTableOut->insert(IdentTable::value_type(ident.name, ident));
}