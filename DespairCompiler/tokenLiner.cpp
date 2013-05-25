/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include "tokenLiner.h"
#include "errorStrings.h"
#include "lockableException.h"
using namespace std;

void lookForTerminator(const vector<Token> *tokens, uint32 &counter, Operator terminator, TokenLine *tLineOut);

void TokenLiner::getTokenLines(const std::vector<Token> *tokens, std::vector<TokenLine> *tLineOut) {
	uint32 counter = 0;
	TokenLine tokenLine;
	try {
		while (counter < tokens->size()) {
			if (tokens->at(counter).type == TOKEN_KEYWORD) {
				if (tokens->at(counter).keyword == KW_PUBLIC) {
					tokenLine.tokens.push_back(tokens->at(counter++));
					continue;
				}
				switch (tokens->at(counter).keyword) {
					case KW_BYTE:
					case KW_FILE:
					case KW_FLOAT:
					case KW_IMAGE:
					case KW_INT:
					case KW_STRING:
					case KW_VOID:
						if (tokens->at(counter + 2).operation == OPERATOR_OPEN_ROUND_BRACKET) {
							lookForTerminator(tokens, counter, OPERATOR_OPEN_CURLY_BRACKET, &tokenLine);
							tokenLine.type = TOKEN_LINE_FUNCTION_DECLARE;
						} else if (tokens->at(counter).keyword != KW_VOID) {
							lookForTerminator(tokens, counter, OPERATOR_SEMI_COLON, &tokenLine);
							tokenLine.type = TOKEN_LINE_VAR_DECLARE;
						} else {
							string errMsg = ERR_STR_ILLEGAL_USE_OF_VOID;
							throw LockableException(errMsg);
						}
						break;
					case KW_CONST:
						lookForTerminator(tokens, counter, OPERATOR_SEMI_COLON, &tokenLine);
						tokenLine.type = TOKEN_LINE_CONST_DECLARE;
						break;
					case KW_POINTER:
						if (tokens->at(counter + 1).keyword == KW_VOID) {
							string errMsg = ERR_STR_VOID_POINTER;
							throw LockableException(errMsg);
						}
						lookForTerminator(tokens, counter, OPERATOR_SEMI_COLON, &tokenLine);
						tokenLine.type = TOKEN_LINE_VAR_DECLARE;
						break;
					case KW_GROUP:
						lookForTerminator(tokens, counter, OPERATOR_OPEN_CURLY_BRACKET, &tokenLine);
						tokenLine.type = TOKEN_LINE_GROUP;
						break;
					case KW_CASE:
						lookForTerminator(tokens, counter, OPERATOR_COLON, &tokenLine);
						tokenLine.type = TOKEN_LINE_CASE;
						break;
					case KW_FCASE:
						lookForTerminator(tokens, counter, OPERATOR_COLON, &tokenLine);
						tokenLine.type = TOKEN_LINE_FCASE;
						break;
					case KW_DEFAULT:
						lookForTerminator(tokens, counter, OPERATOR_COLON, &tokenLine);
						tokenLine.type = TOKEN_LINE_DEFAULT;
						break;
					case KW_FDEFAULT:
						lookForTerminator(tokens, counter, OPERATOR_COLON, &tokenLine);
						tokenLine.type = TOKEN_LINE_FDEFAULT;
						break;
					case KW_FOR:
					case KW_WHILE:
						lookForTerminator(tokens, counter, OPERATOR_OPEN_CURLY_BRACKET, &tokenLine);
						tokenLine.type = TOKEN_LINE_LOOP;
						break;
					case KW_IF:
					case KW_ELSE:
						lookForTerminator(tokens, counter, OPERATOR_OPEN_CURLY_BRACKET, &tokenLine);
						tokenLine.type = TOKEN_LINE_COMPARE;
						break;
					case KW_SWITCH:
						lookForTerminator(tokens, counter, OPERATOR_OPEN_CURLY_BRACKET, &tokenLine);
						tokenLine.type = TOKEN_LINE_SWITCH;
						break;
					case KW_RETURN:
						lookForTerminator(tokens, counter, OPERATOR_SEMI_COLON, &tokenLine);
						tokenLine.type = TOKEN_LINE_RETURN;
						break;
					case KW_BREAK:
						lookForTerminator(tokens, counter, OPERATOR_SEMI_COLON, &tokenLine);
						tokenLine.type = TOKEN_LINE_BREAK;
						break;
					case KW_DELETE:
						lookForTerminator(tokens, counter, OPERATOR_SEMI_COLON, &tokenLine);
						tokenLine.type = TOKEN_LINE_DEALLOCATION;
						break;
					case KW_START_THREAD:
						lookForTerminator(tokens, counter, OPERATOR_SEMI_COLON, &tokenLine);
						tokenLine.type = TOKEN_LINE_THREAD_START;
						break;
					case KW_CONTINUE:
						lookForTerminator(tokens, counter, OPERATOR_SEMI_COLON, &tokenLine);
						tokenLine.type = TOKEN_LINE_CONTINUE;
				}
			} else if (tokens->at(counter).type == TOKEN_IDENTIFIER) {
				switch (tokens->at(counter + 1).operation) {
					case OPERATOR_OPEN_ROUND_BRACKET:
						tokenLine.type = TOKEN_LINE_FUNCTION_CALL;
						break;
					case OPERATOR_ASSIGNMENT:
					case OPERATOR_OPEN_SQUARE_BRACKET:
						tokenLine.type = TOKEN_LINE_ASSIGNMENT;
						break;
					case OPERATOR_PERIOD:
						tokenLine.type = TOKEN_LINE_ASSIGNMENT;
						break;
					default:
						if (tokens->at(counter + 1).type == TOKEN_IDENTIFIER) {
							if (tokens->at(counter + 2).operation == OPERATOR_OPEN_ROUND_BRACKET) {
								tokenLine.type = TOKEN_LINE_FUNCTION_DECLARE;
							} else {
								tokenLine.type = TOKEN_LINE_VAR_DECLARE;
							}
						} else {
							string errMsg;
							errMsg = ERR_STR_UNEXPECTED_TOKEN + tokens->at(counter + 1).value;
							throw LockableException(errMsg);
						}
				}
				lookForTerminator(tokens, counter, (tokenLine.type == TOKEN_LINE_FUNCTION_DECLARE) ? OPERATOR_OPEN_CURLY_BRACKET : OPERATOR_SEMI_COLON, &tokenLine);
			} else {
				switch (tokens->at(counter).operation) {
					case OPERATOR_CLOSE_CURLY_BRACKET:
						tokenLine.type = TOKEN_LINE_CLOSE_SCOPE;
						tokenLine.tokens.push_back(tokens->at(counter++));
						break;
					case OPERATOR_ADD:
					case OPERATOR_SUBSTRACT:
						tokenLine.type = TOKEN_LINE_POINTER_ASSIGNMENT;
						lookForTerminator(tokens, counter, OPERATOR_SEMI_COLON, &tokenLine);
						break;
					default:
						{
							string errMsg;
							errMsg = ERR_STR_UNEXPECTED_TOKEN + tokens->at(counter).value;
							throw LockableException(errMsg);
						}
				}
			}
			tLineOut->push_back(tokenLine);
			tokenLine.tokens.clear();
		}
		if (tokenLine.tokens.size() > 0) throw out_of_range("");
	} catch (out_of_range) {
		string errMsg;
		errMsg = ERR_STR_NO_LINE_TERMINATOR + tokenLine.toString();
		throw LockableException(errMsg);
	}
}

void lookForTerminator(const vector<Token> *tokens, uint32 &counter, Operator terminator, TokenLine *tLineOut) {
	do {
		tLineOut->tokens.push_back(tokens->at(counter));
	} while (tokens->at(counter++).operation != terminator);
}
