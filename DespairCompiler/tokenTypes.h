/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#ifndef TOKEN_TYPES_H
#define TOKEN_TYPES_H

enum TokenType {
	TOKEN_KEYWORD, TOKEN_IDENTIFIER, TOKEN_NUMBER, TOKEN_FLOAT_NUMBER, TOKEN_OPERATOR,
	//This token comes in spotlight much later in the compiling process and is used in ExpressionToken structure (in parsing process)
	TOKEN_REGISTER, TOKEN_CONST_CHAR
};

enum Keyword {
	KW_NONE, KW_INT, KW_FLOAT, KW_BYTE, KW_STRING, KW_CONST, KW_FILE, KW_IMAGE, KW_GROUP, KW_IF, KW_SWITCH, KW_WHILE, KW_FOR,
	KW_CASE, KW_ELSE, KW_RETURN, KW_VOID, KW_POINTER, KW_NEW, KW_DELETE, KW_START_THREAD, KW_PUBLIC, KW_WEAK_PTR
};

enum Operator {
	OPERATOR_NONE, OPERATOR_ADD, OPERATOR_SUBSTRACT, OPERATOR_MULTIPLY, OPERATOR_DIVIDE, OPERATOR_NOT, OPERATOR_NOT_EQUAL, OPERATOR_MOD, 
	OPERATOR_XOR, OPERATOR_AND,	OPERATOR_OR, OPERATOR_COMPLEMENT, OPERATOR_ASSIGNMENT, OPERATOR_SHR, OPERATOR_SHL, OPERATOR_EQUAL, 
	OPERATOR_GREATER, OPERATOR_GREATER_EQUAL, OPERATOR_LESS, OPERATOR_LESS_EQUAL, OPERATOR_LOG_AND, OPERATOR_LOG_OR, 
	OPERATOR_OPEN_ROUND_BRACKET, OPERATOR_CLOSE_ROUND_BRACKET, OPERATOR_OPEN_CURLY_BRACKET, OPERATOR_CLOSE_CURLY_BRACKET, 
	OPERATOR_OPEN_SQUARE_BRACKET, OPERATOR_CLOSE_SQUARE_BRACKET, OPERATOR_SEMI_COLON, OPERATOR_COMMA, OPERATOR_PERIOD, OPERATOR_COLON
};

#endif