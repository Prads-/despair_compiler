#ifndef CONSTANT_PARSER_H
#define CONSTANT_PARSER_H

#include "declarations.h"
#include "tokenLiner.h"

namespace ConstantParser {
	int constantToInteger(const TokenLine *constantTokens);
}

#endif