
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <cctype>
#include <iomanip>

struct SourceLine {
	unsigned int memoryAddress = 0;
	unsigned int memorySize = 0;

	std::string type         = "null";
	std::string inst         = "null";
	std::string dst          = "null";
	std::string src          = "null";
	std::string argument     = "null";
	std::string flag         = "null";
	std::string variableName = "null";
	std::string labelName    = "null";
};

void assertCharacterIsNumber(char ch) {
	assert((ch == '0') ||
		   (ch == '1') ||
		   (ch == '2') ||
		   (ch == '3') ||
		   (ch == '4') ||
		   (ch == '5') ||
		   (ch == '6') ||
		   (ch == '7') ||
		   (ch == '8') ||
		   (ch == '9'));
}

void assertAssemblyAndMemoryCoherence(std::vector<SourceLine>& sourceLines) {
	for (SourceLine& line : sourceLines) {
		if (line.type == "ASM") {
			if (line.argument == "null") {
				assert(line.memorySize == 1);
			}
			if (line.argument != "null") {
				assert(line.memorySize == 2);
			}
		}
	}
	for (SourceLine& line : sourceLines) {
		if (line.type == "ASM") {
			assert(line.inst != "null");
		}
		if (line.type != "ASM") {
			assert(line.inst == "null");
			assert(line.dst == "null");
			assert(line.src == "null");
			assert(line.argument == "null");
			assert(line.flag == "null");
		}
	}
}

void assertVariableDeclarationsAreEndOfSourceFile(std::vector<SourceLine>& sourceLines) {
	int numSourceLines = sourceLines.size();

	bool haveSeenAVariable = false;
	for (int i = 0; i < numSourceLines; i++) {
		SourceLine& sourceLine = sourceLines[i];
		if (sourceLine.type == "VAR") {
			haveSeenAVariable = true;
		}

		if (haveSeenAVariable) {
			if (sourceLine.type != "VAR") {
				assert(false);
			}
		}
	}
}

void assertNoDuplicateVariableDeclarations(std::vector<SourceLine>& sourceLines) {
	int numSourceLines = sourceLines.size();

	for (int i = 0; i < numSourceLines; i++) {
		SourceLine& sourceLine = sourceLines[i];
		if (sourceLine.type != "VAR") continue;
		std::string name = sourceLine.variableName;
		assert(name != "null");

		for (int j = 0; j < numSourceLines; j++) {
			SourceLine& otherSourceLine = sourceLines[j];
			if (otherSourceLine.type != "VAR") continue;
			std::string otherName = otherSourceLine.variableName;
			assert(otherName != "null");

			if (name == otherName) {
				if (i != j) {
					assert(false);
				}
			}
		}
	}
}

void assertNoDuplicateLabels(std::vector<SourceLine>& sourceLines) {
	int numSourceLines = sourceLines.size();

	for (int i = 0; i < numSourceLines; i++) {
		SourceLine& sourceLine = sourceLines[i];
		if (sourceLine.type != "LBL") continue;
		std::string name = sourceLine.labelName;
		assert(name != "null");

		for (int j = 0; j < numSourceLines; j++) {
			SourceLine& otherSourceLine = sourceLines[j];
			if (otherSourceLine.type != "LBL") continue;
			std::string otherName = otherSourceLine.labelName;
			assert(otherName != "null");

			if (name == otherName) {
				if (i != j)
					assert(false);
			}
		}
	}
}

void assertTokenIsValidRegister(std::string token) {
	assert((token == "r0") ||
		   (token == "r1") ||
		   (token == "r2") ||
		   (token == "r3"));
}

void assertTokenIsValidArgument(std::string token) {
	assert(token.size() >= 2);
	if ((token[0] == '#') ||
		(token[0] == '&') ||
		(token[0] == '.')) {
		return;
	}
	std::cout << "\n CATASTROPHIC ERROR: Invalid Argument: " << token << "\n";
	assert(false);
}

void assertTokenIsValidFlag(std::string token) {
	assert((token == "FLAG_ZERO")          ||
		   (token == "FLAG_EQUAL")         ||
		   (token == "FLAG_GREATER_THAN")  ||
		   (token == "FLAG_LESS_THAN")     ||
		   (token == "!FLAG_ZERO")         ||
		   (token == "!FLAG_EQUAL")        ||
		   (token == "!FLAG_GREATER_THAN") ||
		   (token == "!FLAG_LESS_THAN"));
}

std::vector<std::string> tokenize(const std::string& input) {
	std::vector<std::string> result;
	std::istringstream iss(input);
	std::string word;
	while (iss >> word) {
		result.push_back(word);
	}
	return result;
}

SourceLine processTokens(std::vector<std::string> tokens) {
	SourceLine sourceLine;
	assert(tokens.size() != 0);

	if (tokens[0] == "var") {
		assert(tokens.size() == 2);
		assert(tokens[1].size() != 0);
		if (tokens[1].find('[') != std::string::npos) {
			size_t indexOpenBraket = tokens[1].find('[');
			size_t indexCloseBraket = tokens[1].find(']');
			assert(indexOpenBraket < indexCloseBraket - 1);
			assert(indexOpenBraket != std::string::npos);
			assert(indexCloseBraket != std::string::npos);
			assert(indexCloseBraket == tokens[1].size() - 1);
			assert(indexOpenBraket != 0);

			std::string memorySizeString = tokens[1].substr(indexOpenBraket + 1, tokens[1].size() - indexOpenBraket - 2);
			std::string trueVariableName = tokens[1].substr(0, indexOpenBraket);
			assert(memorySizeString.size() != 0);
			assert(trueVariableName.size() != 0);
			for (char ch : memorySizeString) { assertCharacterIsNumber(ch); }

			sourceLine.variableName = trueVariableName;
			sourceLine.memorySize = std::stoul(memorySizeString);
		}
		else {
			assert(tokens[1].find('[') == std::string::npos);
			assert(tokens[1].find(']') == std::string::npos);
			sourceLine.variableName = tokens[1];
			sourceLine.memorySize = 1;
		}
		sourceLine.type = "VAR";
		return sourceLine;
	}

	if (tokens[0] == "label") {
		assert(tokens.size() == 2);
		assert(tokens[1].size() != 0);
		sourceLine.type = "LBL";
		sourceLine.labelName = tokens[1];
		return sourceLine;
	}

	if ((tokens[0] == "LDI") ||
		(tokens[0] == "LDR") ||
		(tokens[0] == "STR") ||
		(tokens[0] == "ADI")) {
		assert((tokens.size() == 3) || (tokens.size() == 4));
		assertTokenIsValidRegister(tokens[1]);
		assertTokenIsValidArgument(tokens[2]);
		if (tokens.size() == 4) {
			assertTokenIsValidFlag(tokens[3]);
		}

		sourceLine.inst = tokens[0];
		sourceLine.dst = tokens[1];
		sourceLine.argument = tokens[2];
		if (tokens.size() == 4) {
			sourceLine.flag = tokens[3];
		}

		sourceLine.type = "ASM";
		sourceLine.memorySize = 2;
		return sourceLine;
	}

	if ((tokens[0] == "LDX") ||
		(tokens[0] == "STX") ||
		(tokens[0] == "MOV") ||
		(tokens[0] == "ADD") ||
		(tokens[0] == "SUB") ||
		(tokens[0] == "AND") ||
		(tokens[0] == "ORR") ||
		(tokens[0] == "XOR") ||
		(tokens[0] == "CMP")) {
		assert((tokens.size() == 3) || (tokens.size() == 4));
		assertTokenIsValidRegister(tokens[1]);
		assertTokenIsValidRegister(tokens[2]);
		if (tokens.size() == 4) {
			assertTokenIsValidFlag(tokens[3]);
		}

		sourceLine.inst = tokens[0];
		sourceLine.dst = tokens[1];
		sourceLine.src = tokens[2];
		if (tokens.size() == 4) {
			sourceLine.flag = tokens[3];
		}

		sourceLine.type = "ASM";
		sourceLine.memorySize = 1;
		return sourceLine;
	}

	if ((tokens[0] == "SHR") ||
		(tokens[0] == "SHL") ||
		(tokens[0] == "KIN") ||
		(tokens[0] == "GSP") ||
		(tokens[0] == "PSH") ||
		(tokens[0] == "POP")) {
		assert((tokens.size() == 2) || (tokens.size() == 3));
		assertTokenIsValidRegister(tokens[1]);
		if (tokens.size() == 3) {
			assertTokenIsValidFlag(tokens[2]);
		}

		sourceLine.inst = tokens[0];
		sourceLine.dst = tokens[1];
		if (tokens.size() == 3) {
			sourceLine.flag = tokens[2];
		}

		sourceLine.type = "ASM";
		sourceLine.memorySize = 1;
		return sourceLine;
	}

	if ((tokens[0] == "DRW") ||
		(tokens[0] == "RTS")) {
		assert((tokens.size() == 1) || (tokens.size() == 2));
		if (tokens.size() == 2) {
			assertTokenIsValidFlag(tokens[1]);
		}

		sourceLine.inst = tokens[0];
		if (tokens.size() == 2) {
			sourceLine.flag = tokens[1];
		}

		sourceLine.type = "ASM";
		sourceLine.memorySize = 1;
		return sourceLine;
	}

	if ((tokens[0] == "JMP") ||
		(tokens[0] == "JSR")) {
		assert((tokens.size() == 2) || (tokens.size() == 3));
		assertTokenIsValidArgument(tokens[1]);
		if ((tokens.size() == 3)) {
			assertTokenIsValidFlag(tokens[2]);
		}

		sourceLine.inst = tokens[0];
		sourceLine.argument = tokens[1];
		if ((tokens.size() == 3)) {
			sourceLine.flag = tokens[2];
		}

		sourceLine.type = "ASM";
		sourceLine.memorySize = 2;
		return sourceLine;
	}

	assert(false);
}

std::string stringToHex(unsigned short value) {
	std::ostringstream oss;
	oss << std::hex << std::setw(4) << std::setfill('0') << std::nouppercase << value;
	return oss.str();
}

int main() {
	std::vector<std::string> lines;
	std::ifstream file("assembly.txt");
	std::string line;
	while (std::getline(file, line)) {
		lines.push_back(line);
	}
	file.close();

	std::vector<SourceLine> sourceLines;
	for (std::string& line : lines) {
		std::vector<std::string> tokens = tokenize(line);
		if (tokens.size() == 0) continue;
		if (tokens[0].substr(0, 2) == "//") continue;
		sourceLines.push_back(processTokens(tokens));
	}

	assertAssemblyAndMemoryCoherence(sourceLines);
	assertVariableDeclarationsAreEndOfSourceFile(sourceLines);
	assertNoDuplicateVariableDeclarations(sourceLines);
	assertNoDuplicateLabels(sourceLines);

	unsigned int currentAddress = 0;
	for (SourceLine& line : sourceLines) {
		line.memoryAddress = currentAddress;
		currentAddress += line.memorySize;
	}

	for (SourceLine& line : sourceLines) {
		if (line.argument == "null") continue;
		assert(line.type == "ASM");

		if (line.argument[0] == '#') {
			std::string trueArgument = line.argument.substr(1, line.argument.size() - 1);
			assert((line.inst == "LDI") || (line.inst == "ADI"));
			assert(trueArgument.size() >= 1);
			for (char& ch : trueArgument) { assertCharacterIsNumber(ch); }
			line.argument = trueArgument;
			continue;
		}

		if (line.argument[0] == '&') {
			std::string targetVariable = line.argument.substr(1, line.argument.size() - 1);
			assert((line.inst == "LDI") || (line.inst == "LDR") || (line.inst == "STR"));
			assert(targetVariable.size() >= 1);

			for (SourceLine& otherLine : sourceLines) {
				if (otherLine.type != "VAR") continue;
				assert(otherLine.variableName != "null");
				if (otherLine.variableName == targetVariable) {
					line.argument = std::to_string(otherLine.memoryAddress);
					goto doubleContinue;
				}
			}

			std::cout << "\n CATASTROPHIC ERROR: Could not Find Variable: " << targetVariable << "\n";
			assert(false);
		}

		if (line.argument[0] == '.') {
			std::string targetLabel = line.argument.substr(1, line.argument.size() - 1);
			assert((line.inst == "JMP") || (line.inst == "JSR"));
			assert(targetLabel.size() >= 1);

			for (SourceLine& otherLine : sourceLines) {
				if (otherLine.type != "LBL") continue;
				assert(otherLine.labelName != "null");
				if (otherLine.labelName == targetLabel) {
					line.argument = std::to_string(otherLine.memoryAddress);
					goto doubleContinue;
				}
			}

			std::cout << "\n CATASTROPHIC ERROR: Could not Find Label: " << targetLabel << "\n";
			assert(false);
		}

	doubleContinue:
		continue;
	}

	std::ofstream out("compiled_binary.bin", std::ios::binary);
	std::ofstream textOut("memory_init.hex");
	for (SourceLine& line : sourceLines) {
		assert(line.type != "null");
		assert(sizeof(unsigned short) == 2);

		if (line.type == "ASM") {
			unsigned short machineCode = 0b0000000000000000;

			if (line.inst == "LDI") { machineCode += 0b0000000000000000; textOut << "00"; }
			if (line.inst == "LDR") { machineCode += 0b0000000100000000; textOut << "01"; }
			if (line.inst == "STR") { machineCode += 0b0000001000000000; textOut << "02"; }
			if (line.inst == "LDX") { machineCode += 0b0000001100000000; textOut << "03"; }
			if (line.inst == "STX") { machineCode += 0b0000010000000000; textOut << "04"; }
			if (line.inst == "MOV") { machineCode += 0b0000010100000000; textOut << "05"; }
			if (line.inst == "SHR") { machineCode += 0b0000011000000000; textOut << "06"; }
			if (line.inst == "SHL") { machineCode += 0b0000011100000000; textOut << "07"; }
			if (line.inst == "ADI") { machineCode += 0b0000100000000000; textOut << "08"; }
			if (line.inst == "ADD") { machineCode += 0b0000100100000000; textOut << "09"; }
			if (line.inst == "SUB") { machineCode += 0b0000101000000000; textOut << "0A"; }
			if (line.inst == "AND") { machineCode += 0b0000101100000000; textOut << "0B"; }
			if (line.inst == "ORR") { machineCode += 0b0000110000000000; textOut << "0C"; }
			if (line.inst == "XOR") { machineCode += 0b0000110100000000; textOut << "0E"; }
			if (line.inst == "CMP") { machineCode += 0b0000111000000000; textOut << "0E"; }
			if (line.inst == "KIN") { machineCode += 0b0000111100000000; textOut << "0F"; }
			if (line.inst == "DRW") { machineCode += 0b0001000000000000; textOut << "10"; }
			if (line.inst == "JMP") { machineCode += 0b0001000100000000; textOut << "11"; }
			if (line.inst == "JSR") { machineCode += 0b0001001000000000; textOut << "12"; }
			if (line.inst == "RTS") { machineCode += 0b0001001100000000; textOut << "13"; }
			if (line.inst == "GSP") { machineCode += 0b0001010000000000; textOut << "14"; }
			if (line.inst == "POP") { machineCode += 0b0001010100000000; textOut << "15"; }
			if (line.inst == "PSH") { machineCode += 0b0001011000000000; textOut << "16"; }

			if (line.dst == "r0")   { machineCode += 0b0000000000000000; }
			if (line.dst == "r1")   { machineCode += 0b0000000001000000; }
			if (line.dst == "r2")   { machineCode += 0b0000000010000000; }
			if (line.dst == "r3")   { machineCode += 0b0000000011000000; }
								    
			if (line.src == "r0")   { machineCode += 0b0000000000000000; }
			if (line.src == "r1")   { machineCode += 0b0000000000010000; }
			if (line.src == "r2")   { machineCode += 0b0000000000100000; }
			if (line.src == "r3")   { machineCode += 0b0000000000110000; }

			std::string DST = line.dst;
			std::string SRC = line.src;
			if ((DST == "r0") || (DST == "null")) {
				if ((SRC == "r0") || (SRC == "null")) { textOut << "0"; }
				if  (SRC == "r1")                     { textOut << "1"; }
				if  (SRC == "r2")                     { textOut << "2"; }
				if  (SRC == "r3")                     { textOut << "3"; } 
			}
			if  (DST == "r1") {
				if ((SRC == "r0") || (SRC == "null")) { textOut << "4"; }
				if  (SRC == "r1")                     { textOut << "5"; }
				if  (SRC == "r2")                     { textOut << "6"; }
				if  (SRC == "r3")                     { textOut << "7"; }
			}
			if  (DST == "r2") {
				if ((SRC == "r0") || (SRC == "null")) { textOut << "8"; }
				if  (SRC == "r1")                     { textOut << "9"; }
				if  (SRC == "r2")                     { textOut << "A"; }
				if  (SRC == "r3")                     { textOut << "B"; }
			}
			if  (DST == "r3") {
				if ((SRC == "r0") || (SRC == "null")) { textOut << "C"; }
				if  (SRC == "r1")                     { textOut << "D"; }
				if  (SRC == "r2")                     { textOut << "E"; }
				if  (SRC == "r3")                     { textOut << "F"; }
			}

			if (line.flag == "null")               { machineCode += 0b0000000000000000; textOut << "0\n"; }
			if (line.flag == "FLAG_ZERO")          { machineCode += 0b0000000000001000; textOut << "8\n"; }
			if (line.flag == "FLAG_EQUAL")         { machineCode += 0b0000000000001001; textOut << "9\n"; }
			if (line.flag == "FLAG_GREATER_THAN")  { machineCode += 0b0000000000001010; textOut << "A\n"; }
			if (line.flag == "FLAG_LESS_THAN")     { machineCode += 0b0000000000001011; textOut << "B\n"; }
			if (line.flag == "!FLAG_ZERO")         { machineCode += 0b0000000000001100; textOut << "C\n"; }
			if (line.flag == "!FLAG_EQUAL")        { machineCode += 0b0000000000001101; textOut << "D\n"; }
			if (line.flag == "!FLAG_GREATER_THAN") { machineCode += 0b0000000000001110; textOut << "E\n"; }
			if (line.flag == "!FLAG_LESS_THAN")    { machineCode += 0b0000000000001111; textOut << "F\n"; }

			out.write((const char*)(&machineCode), sizeof(unsigned short));

			if (line.argument != "null") {
				for (char ch : line.argument) { assertCharacterIsNumber(ch); }
				assert(line.memorySize == 2);

				unsigned long argumentLong = std::stoul(line.argument);
				assert(argumentLong < 65536);
				unsigned short argumentShort = (unsigned short)argumentLong;
				out.write((const char*)(&argumentShort), sizeof(unsigned short));
				textOut << stringToHex(argumentShort);
				textOut << "\n";
			}
		}

		if (line.type == "VAR") {
			unsigned short zero = 0;
			for (int i = 0; i < line.memorySize; i++) {
				out.write((const char*)(&zero), sizeof(unsigned short));
				textOut << "0000\n";
			}
		}

		if (line.type == "LBL")
			continue;
	}

	return 0;
}

