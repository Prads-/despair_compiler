/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef ERROR_STRINGS_H
#define ERROR_STRINGS_H

#define ERR_STR_NO_MODULE_END			"E1000 - Couldn't find proper ending to this module"
#define ERR_STR_INVALID_LINE			"E1001 - Invalid Line: "
#define ERR_STR_NO_LINE_TERMINATOR		"E1002 - Couldn't find terminator in line: "
#define ERR_STR_UNEXPECTED_TOKEN		"E1003 - Unexpected Token: "
#define ERR_STR_SYNTAX_ERROR			"E1004 - Syntax Error: "
#define ERR_STR_GROUP_EXISTS			"E1005 - Group with this name already exists: "
#define ERR_STR_NO_GROUP_REFERENCE		"E1006 - Cannot find group \"" + gmIt->second.dataTypeStr + "\" referenced in  group \"" + group->name + "\""
#define ERR_STR_GROUP_CIRCULAR_REF		"E1007 - Circular group reference is not valid. Group reference map: "
#define ERR_STR_AMBIGUOUS_OVERLOAD		"E1008 - Ambiguous overloaded functions: "
#define ERR_STR_DUPLICATE_IDENT			"E1009 - Duplicate identifier is not valid: "
#define ERR_STR_UNKNOWN_TYPE			"E1010 - Unknown data type or group in line: "
#define ERR_STR_INVALID_OPERATOR		"E1011 - Invalid Operator \"" + (*tokens)[tokenCounter].value + "\""
#define ERR_STR_UNDECLARED_IDENT		"E1012 - Undeclared identifier: "
#define ERR_STR_UNDECLARED_FUNCTION		"E1013 - Undeclared Function called: "
#define ERR_STR_UNDECLARED_GROUP		"E1014 - Undeclared Group: "
#define ERR_STR_UNDECLARED_GRP_MEMBER	"E1015 - Group member not defined: "
#define ERR_STR_IS_NOT_A_GROUP			"E1016 - This identifier is not of type group: "
#define ERR_STR_CANNOT_BE_COPIED		"E1017 - File or Image cannot be copied: "
#define ERR_STR_TYPE_MISMATCH			"E1018 - Type mismatch: "
#define ERR_STR_INVALID_CMP_STATEMENT	"E1019 - Invalid comparision statement: "
#define ERR_STR_INVALID_CMP_DATA		"E1020 - Invalid Data Type in a comparision expression: "
#define ERR_STR_GRP_MEM_STR_ACCESS		"E1021 - Accessing string member this way will generate memory corruption: "
#define ERR_STR_INVALID_LOOP_STATEMENT	"E1022 - Invalid loop statement: "
#define ERR_STR_NO_MAIN_FUNCTION		"E1023 - Could not find the main function"
#define ERR_STR_COMPILER_BUG			"E1024 - Compiler Bug Found. Please report it to the dev team. Source: "
#define ERR_STR_NOT_ENOUGH_ARG			"E1025 - Not enough arguments"
#define ERR_STR_SRC_NOT_FOUND			"E1026 - Source code was not found: "
#define ERR_STR_RET_TYPE_MISMATCH		"E1027 - Return type mismatch: "
#define ERR_STR_ILLEGAL_USE_OF_VOID		"E1028 - Illegal use of void"
#define ERR_STR_TYPE_CASTING_INVALID	"E1029 - Cannot cast the expression: "
#define ERR_STR_VOID_POINTER			"E1030 - Cannot create void pointer: "
#define ERR_STR_DEST_NOT_POINTER		"E1031 - Destination is not a pointer in pointer assignment: "
#define ERR_STR_POINTER_TYPE_MISMATCH	"E1032 - Pointer type mismatch: "
#define ERR_STR_STD_BIN_LIB_LST_NOT_FND	"E1033 - Standard Binary Library list was not found"
#define ERR_STR_STD_BIN_LIB_DEF_NOT_FND	"E1034 - Standard Binary Library function definition not found: "
#define ERR_STR_STD_BIN_LIB_BIN_NOT_FND	"E1035 - Standard Binary Library binary not found: "
#define ERR_STR_STD_BIN_LIB_UNKWN_TYPE	"E1036 - Unknown Type in Standard Binary Library definition: "
#define ERR_STR_ALLOC_SIZE				"E1037 - Memory Allocation Size must be of type integer or byte: "
#define ERR_STR_ALLOC_TYPE				"E1038 - Memory Allocation Type mismatch: "
#define ERR_STR_ALLOC_IS_NOT_A_POINTER	"E1039 - Memory Allocation Destination must be a pointer: "
#define ERR_STR_DEALLOC_NOT_A_POINTER	"E1040 - Memory Deallocation Identifier must be a pointer: "
#define ERR_STR_GRP_MEMBER_EXISTS		"E1041 - Group Member with this name already exists: "
#define ERR_STR_THREAD_DEFINITION		"E1042 - Function called as a thread should have definition in either 'void functionName()' or 'void functionName(pointer byte)' format: "
#define ERR_STR_INVALID_ARRAY_SIZE		"E1043 - Invalid array size: "
#define ERR_STR_POINTER_ARRAY			"E1044 - Cannot create array of pointer: "
#define ERR_STR_GROUP_NOT_EXIST			"E1045 - Group does not exist: "
#define ERR_STR_GRP_INHERIT_NOT_EXIST	"E1046 - Group trying to inherit from group that doesn't exist: "
#define ERR_STR_GRP_CIRCULAR_INHERIT	"E1047 - Circular inheritance is not allowed: "

#endif