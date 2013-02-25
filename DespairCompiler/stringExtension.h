/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef STRING_EXTENSION_H
#define STRING_EXTENSION_H

#include <string>
#include <fstream>

namespace StringExtension {
	void strAppendInteger(int integer, std::string *strOut);
	bool getLineFromFile(std::ifstream *file, std::string *lineOut);
}

#endif