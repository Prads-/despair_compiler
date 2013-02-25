/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef CODE_CONTAINER_H
#define CODE_CONTAINER_H

#include <cstdlib>
#include "declarations.h"

class CodeContainer {
private:
	uint8 *code;
	uint32 size, counter;
	void checkBoundary(int cSize);
public:
	CodeContainer();
	~CodeContainer();

	template <typename Type>
	void writeBack(Type val);

	void writeBig(uint8 *srcCode, int cSize);

	const uint8 *getCode() const;
	uint32 getSize() const;
};

#endif