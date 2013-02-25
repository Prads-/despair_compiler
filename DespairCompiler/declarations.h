/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef DECLARATIONS_H
#define DECLARATIONS_H

typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef char int8;
typedef short int int16;
typedef int int32;
typedef long long int64;

typedef float float32;

#define _DEBUG_MODE

#define REG_STRING_REGISTER				250	//String register, only one that exists
#define REG_RETURN_STRING				251	//Holds strings returned by a function
#define REG_STRING_POINTER				252	//Points to string object
#define REG_INDEX_POINTER				253	//Points to variables
#define REG_GLOBAL_BASE_POINTER			254	//Points to start of global memory	(built in despair architecture)
#define REG_BASE_POINTER				255	//Points to start of local memory - data space (built in despair architecture)

#endif