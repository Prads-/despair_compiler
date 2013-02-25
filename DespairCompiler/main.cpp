/*
	Written By Pradipna Nepal
	www.pradsprojects.com

	Copyright (C) 2013 Pradipna Nepal
	Please read COPYING.txt included along with this source code for more detail.
	If not included, see http://www.gnu.org/licenses/
*/

#include <iostream>
#include <fstream>
#include <vector>
#include "errorStrings.h"
#include "tokenizer.h"
#include "tokenLiner.h"
#include "tokenModule.h"
#include "symbolTable.h"
#include "parseModule.h"
#include "codeGenerator.h"
#include "preprocessor.h"
#include "stringExtension.h"
#include "constTable.h"
#include "lockableException.h"
using namespace std;
using namespace Tokenizer;
using namespace TokenLiner;
using namespace TokenMod;
using namespace SymbolTable;
using namespace ParseModule;
using namespace CodeGenerator;
using namespace HeaderGenerator;
using namespace Preprocessor;
using namespace StringExtension;
using namespace ConstTable;

bool saveBinFile(DPVMHeader *header, CodeContainer *machineCode, char *savePath);
bool loadDefaultFont(int &fontSize, char **font);
void logMsg(ofstream *logFile, string msg);

int main(int argc, char **argv) {
	cout << "*** despairCompiler version 0.01 ***" << endl;
	cout << "Written by Prads" << endl;
	cout << "Copyright (C) 2013 Pradipna Nepal" << endl;
	cout << "Please read COPYING.txt included along with this source code for more detail." << endl;
	cout << "If not included, see http://www.gnu.org/licenses/\r\n" << endl;

	CodeContainer machineCode;
	DPVMHeader header;
	int retVal = 0;
	ofstream logFile("buildLog.txt");

	if (argc < 3) {
		logMsg(&logFile, ERR_STR_NOT_ENOUGH_ARG); 
		retVal = 1;
		goto lblEnd;
	}
    {
        vector<vector<Token> > *tokens = 0;
        vector<vector<TokenLine> > *tokenLines = 0;
        TokenModule *tokenModule = 0;
        GroupTable *groupTable = 0;
        FunctionTable *functionTable = 0;
		char *font = 0;
		
        try {
            logMsg(&logFile, "Converting source code into tokens...");
			ConstCharTable constCharTable;
			ConstantTable constTable;
			tokens = new vector<vector<Token> >;

			//Get the project folder where all the source exists
			string projectFolder = argv[1];
			int projectFolderSize;
			for (projectFolderSize = projectFolder.size() - 1; projectFolderSize >= 0; --projectFolderSize) {
				if (projectFolder[projectFolderSize] == '/') {
					break;
				}
			}
			projectFolder = projectFolder.substr(0, projectFolderSize + 1);

			//Get contents in project file
			ifstream dproFile(argv[1]);
			if (dproFile.is_open()) {
				//Get frame length
				string frameWidth, frameHeight;
				getLineFromFile(&dproFile, &frameWidth);
				getLineFromFile(&dproFile, &frameHeight);
				header.part1.frameBufferWidth = atoi(frameWidth.c_str());
				header.part1.frameBufferHeight = atoi(frameHeight.c_str());

				//Get Data Space and Stack Space Size
				string dataSize, stackSize;
				getLineFromFile(&dproFile, &dataSize);
				getLineFromFile(&dproFile, &stackSize);
				header.part1.dataSize = atoi(dataSize.c_str());
				header.part1.stackSize = atoi(stackSize.c_str());

				//Get source paths
				string sourcePath;
				while (getLineFromFile(&dproFile, &sourcePath)) {
					if (sourcePath == "[source]") {
						getLineFromFile(&dproFile, &sourcePath);
						vector<Token> moduleTokens;
						getTokens(projectFolder + sourcePath, &moduleTokens, &constCharTable);
						tokens->push_back(moduleTokens);
					}
				}
			} else {
				throw LockableException("Project File not found");
			}
			logMsg(&logFile, "OK...");

            logMsg(&logFile, "Converting tokens into token lines...");
            tokenLines = new vector<vector<TokenLine> >;
			for (uint32 i = 0; i < tokens->size(); ++i) {
				vector<TokenLine> moduleTokenLine;
				getTokenLines(&(*tokens)[i], &moduleTokenLine);
				tokenLines->push_back(moduleTokenLine);
			}
			delete tokens;
            tokens = 0;
            logMsg(&logFile, "OK...");

			logMsg(&logFile, "Creating Constant Table...");
			createConstantTable(&constTable, tokenLines);
			logMsg(&logFile, "OK...");


            logMsg(&logFile, "Creating Token Modules and Group Table...");
            tokenModule = new TokenModule;
            groupTable = new GroupTable;
            createTokenModule(tokenLines, tokenModule, groupTable);
            delete tokenLines;
            tokenLines = 0;
			logMsg(&logFile, "OK...");

			logMsg(&logFile, "Loading default fonts...");
			int fontSize;
			if (!loadDefaultFont(fontSize, &font)) {
				throw LockableException("Loading default font failed");
			}
			logMsg(&logFile, "OK...");

            logMsg(&logFile, "Creating Symbol Table...");
            functionTable = new FunctionTable;
            createSymbolTable(tokenModule, groupTable, functionTable, &constCharTable, fontSize);
            logMsg(&logFile, "OK...");

			logMsg(&logFile, "Preprocessing...");
			preprocess(tokenModule, groupTable, &constTable);
			logMsg(&logFile, "OK...");

            logMsg(&logFile, "Parsing Modules...");
            uint32 mainFunctionID = parseModule(tokenModule, groupTable, functionTable, &constCharTable);
            delete tokenModule;
            tokenModule = 0;
            delete groupTable;
            groupTable = 0;
            logMsg(&logFile, "OK...");

            logMsg(&logFile, "Generating code...");
            generateCode(functionTable, mainFunctionID, &machineCode, &header, &constCharTable, font, fontSize);
            delete functionTable;
            logMsg(&logFile, "OK...");
        } catch (exception &e) {
            logMsg(&logFile, "FAILED...");
            logMsg(&logFile, e.what());
            delete tokens;
            delete tokenLines;
            delete tokenModule;
            delete groupTable;
            delete functionTable;
			delete font;
			retVal = 1;
            goto lblEnd;
        }

        logMsg(&logFile, "Saving Binary File...");
        if (saveBinFile(&header, &machineCode, argv[2])) {
            logMsg(&logFile, "OK...");
        } else {
            logMsg(&logFile, "FAILED...");
			retVal = 1;
        }
    }

lblEnd:
	logFile.close();
	return retVal;
}

bool saveBinFile(DPVMHeader *header, CodeContainer *machineCode, char *savePath) {
	ofstream outputFile;
	outputFile.open(savePath, ofstream::binary);
	if (outputFile.is_open()) {
		outputFile.write((char*)&header->part1, sizeof(HeaderPart1));
		outputFile.write((char*)header->part2.globalPreData, header->part2.globalPreDataSize);
		outputFile.write((char*)machineCode->getCode(), machineCode->getSize());
	} else {
		return false;
	}
	outputFile.close();
	return true;
}

bool loadDefaultFont(int &fontSize, char **font) {
	ifstream fontFile;
	fontFile.open("bin/font.bin", ifstream::binary | ifstream::ate);
	if (fontFile.is_open()) {
		fontSize = fontFile.tellg();
		fontFile.seekg(0, ios::beg);
		*font = new char[fontSize];
		fontFile.read(*font, fontSize);
		fontFile.close();
	} else {
		return false;
	}

	return true;
}

void logMsg(ofstream *logFile, string msg) {
	*logFile << msg << endl;
	cout << msg << endl;
}
