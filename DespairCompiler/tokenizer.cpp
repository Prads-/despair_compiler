/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include "tokenizer.h"
#include "errorStrings.h"
#include "lockableException.h"
using namespace std;

void getSource(const string *path, string *source);
void tokenize(const string *source, vector<Token> *tokenOut, ConstCharTable *constCharTableOut);
Keyword getKeywordType(const string *word);
Operator getOperatorType(char c, char &nextChar, int &counter);
void getIdentifier(const string *source, int &counter, vector<Token> *tokenOut);
void getNumber(const string *source, int &counter, vector<Token> *tokenOut);
void getOperator(const string *source, int &counter, vector<Token> *tokenOut);
string getConstChar(const string *source, int &counter, vector<Token> *tokenOut);

void Tokenizer::getTokens(std::string path, std::vector<Token> *tokenOut, ConstCharTable *constCharTableOut) {
	string source;
	getSource(&path, &source);
	tokenize(&source, tokenOut, constCharTableOut);
}

void getSource(const string *path, string *source) {
	ifstream sourceFile(path->c_str());
	*source = "";
	if (sourceFile.is_open()) {
		string srcLine;
		while(getline(sourceFile, srcLine)) {
			for (uint32 i = 0; i < srcLine.size(); ++i) {
				if (srcLine[i] == '/' && srcLine[i + 1] == '/') {
					break;
				} else {
					source->push_back(srcLine[i]);
				}
			}
		}
	} else {
		string errMsg = ERR_STR_SRC_NOT_FOUND + *path;
		throw LockableException(errMsg);
	}
}

void tokenize(const string *source, vector<Token> *tokenOut, ConstCharTable *constCharTableOut) {
	int counter = 0;
	while (counter < source->size()) {
		if ((*source)[counter] == ' ' || (*source)[counter] == '\t' || (*source)[counter] == '\r' || (*source)[counter] == '\n') {
			++counter;
		} else if (isalpha((*source)[counter])) {
			getIdentifier(source, counter, tokenOut);
		} else if (isdigit((*source)[counter])) {
			getNumber(source, counter, tokenOut);
		} else {
			if ((*source)[counter] == '-' && isdigit(source->at(counter + 1))) {
				getNumber(source, counter, tokenOut);
			} else if ((*source)[counter] == '"') {
				string constCharStr = getConstChar(source, ++counter, tokenOut);
				ConstChar constChar(constCharStr);
				constCharTableOut->insert(ConstCharTable::value_type(constCharStr, constChar));
			} else {
				getOperator(source, counter, tokenOut);
			}
		}
	}
}

Keyword getKeywordType(const string *word) {
	if (*word == "int")
		return KW_INT;
	else if (*word == "float")
		return KW_FLOAT;
	else if (*word == "byte")
		return KW_BYTE;
	else if (*word == "string")
		return KW_STRING;
	else if (*word == "const")
		return KW_CONST;
	else if (*word == "file")
		return KW_FILE;
	else if (*word == "image")
		return KW_IMAGE;
	else if (*word == "group")
		return KW_GROUP;
	else if (*word == "if")
		return KW_IF;
	else if (*word == "switch")
		return KW_SWITCH;
	else if (*word == "while")
		return KW_WHILE;
	else if (*word == "for")
		return KW_FOR;
	else if (*word == "case")
		return KW_CASE;
	else if (*word == "else")
		return KW_ELSE;
	else if (*word == "return")
		return KW_RETURN;
	else if (*word == "void")
		return KW_VOID;
	else if (*word == "pointer")
		return KW_POINTER;
	else if (*word == "new")
		return KW_NEW;
	else if (*word == "delete")
		return KW_DELETE;
	else if (*word == "startThread")
		return KW_START_THREAD;
	else if (*word == "public")
		return KW_PUBLIC;
	else if (*word == "weakPtr")
		return KW_WEAK_PTR;
	else
		return KW_NONE;
}

Operator getOperatorType(char c, char &nextChar, int &counter) {
	switch (c) {
		case '+':
			nextChar = 0;
			return OPERATOR_ADD;
			break;
		case '-':
			nextChar = 0;
			return OPERATOR_SUBSTRACT;
			break;
		case '*':
			nextChar = 0;
			return OPERATOR_MULTIPLY;
			break;
		case '/':
			nextChar = 0;
			return OPERATOR_DIVIDE;
			break;
		case '!':
			if (nextChar == '=') {
				++counter;
				return OPERATOR_NOT_EQUAL;
			}
			nextChar = 0;
			return OPERATOR_NOT;
			break;
		case '%':
			nextChar = 0;
			return OPERATOR_MOD;
			break;
		case '^':
			nextChar = 0;
			return OPERATOR_XOR;
			break;
		case '&':
			if (nextChar == '&'){
				++counter;
				return OPERATOR_LOG_AND;
			}
			nextChar = 0;
			return OPERATOR_AND;
			break;
		case '|':
			if (nextChar == '|'){
				++counter;
				return OPERATOR_LOG_OR;
			}
			nextChar = 0;
			return OPERATOR_OR;
			break;
		case '~':
			nextChar = 0;
			return OPERATOR_COMPLEMENT;
			break;
		case '=':
			if (nextChar == '=') {
				++counter;
				return OPERATOR_EQUAL;
			}
			nextChar = 0;
			return OPERATOR_ASSIGNMENT;
			break;
		case '>':
			if (nextChar == '>') {
				++counter;
				return OPERATOR_SHR;
			} else if (nextChar == '=') {
				++counter;
				return OPERATOR_GREATER_EQUAL;
			}
			nextChar = 0;
			return OPERATOR_GREATER;
			break;
		case '<':
			if (nextChar == '<') {
				++counter;
				return OPERATOR_SHL;
			} else if (nextChar == '=') {
				++counter;
				return OPERATOR_LESS_EQUAL;
			}
			nextChar = 0;
			return OPERATOR_LESS;
			break;
		case ',':
			nextChar = 0;
			return OPERATOR_COMMA;
		case '.':
			nextChar = 0;
			return OPERATOR_PERIOD;
		case ':':
			nextChar = 0;
			return OPERATOR_COLON;
		case ';':
			nextChar = 0;
			return OPERATOR_SEMI_COLON;
		case '(':
			nextChar = 0;
			return OPERATOR_OPEN_ROUND_BRACKET;
		case ')':
			nextChar = 0;
			return OPERATOR_CLOSE_ROUND_BRACKET;
		case '{':
			nextChar = 0;
			return OPERATOR_OPEN_CURLY_BRACKET;
		case '}':
			nextChar = 0;
			return OPERATOR_CLOSE_CURLY_BRACKET;
		case '[':
			nextChar = 0;
			return OPERATOR_OPEN_SQUARE_BRACKET;
		case ']':
			nextChar = 0;
			return OPERATOR_CLOSE_SQUARE_BRACKET;
		default:
			nextChar = 0;
			return OPERATOR_NONE;
	}
}

void getIdentifier(const string *source, int &counter, vector<Token> *tokenOut) {
	uint32 sourceSize = source->size();
	Token token;

	token.value = "";
	do {
		if (isalnum((*source)[counter]) || (*source)[counter] == '_')
			token.value += (*source)[counter];
		else
			break;
	} while (++counter < sourceSize);
	token.keyword = getKeywordType(&token.value);
	token.operation = OPERATOR_NONE;
	if (token.keyword != KW_NONE) {
		token.type = TOKEN_KEYWORD;
	} else {
		token.type = TOKEN_IDENTIFIER;
	}
	tokenOut->push_back(token);
}

void getNumber(const string *source, int &counter, vector<Token> *tokenOut) {
	uint32 sourceSize = source->size();
	Token token;
	TokenType type = TOKEN_NUMBER;

	if ((*source)[counter] == '-') {
		token.value += (*source)[counter++];
	}

	do {
		if  ((*source)[counter] == '.') {
			type = TOKEN_FLOAT_NUMBER;
		} else if (!isdigit((*source)[counter])) {
			break;
		}
		token.value += (*source)[counter];
	} while (++counter < sourceSize);

	token.keyword = KW_NONE;
	token.operation = OPERATOR_NONE;
	token.type = type;
	tokenOut->push_back(token);
}

void getOperator(const string *source, int &counter, vector<Token> *tokenOut) {
	Token token;
	char c = (*source)[counter++], nextChar = 0;
	if (counter < source->size()) {
		if (!isalnum((*source)[counter])) nextChar = (*source)[counter];
	}

	token.operation = getOperatorType(c, nextChar, counter);
	if (token.operation == OPERATOR_NONE) {
		string errMsg;
		errMsg = "Invalid Symbol: ";
		errMsg += c;
		if (nextChar) errMsg += nextChar;
		throw LockableException(errMsg);
	}
	token.value = c;
	if (nextChar) token.value += nextChar;
	token.keyword = KW_NONE;
	token.type = TOKEN_OPERATOR;
	tokenOut->push_back(token);
}

string getConstChar(const string *source, int &counter, vector<Token> *tokenOut) {
	string constStr = "";
	
	while(source->at(counter) != '"') {
		constStr.push_back(source->at(counter++));
	}
	++counter;

	tokenOut->push_back(Token(TOKEN_CONST_CHAR, KW_NONE, OPERATOR_NONE, constStr));
	return constStr;
}