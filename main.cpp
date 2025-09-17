
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <cctype>

struct SourceLine
{
	std::string type = "null";      // "ASM" or "LBL" or "VAR" or "null"
	unsigned int memoryAddress = 0; // Location in program memory
	unsigned int memorySize = 0;    // 0 or 1 or 2

	// Assembly
	std::string inst = "null";      // "LDI" or "LDR" or "STR" or ... or "POP" or "null"
	std::string dst = "null";       // "r0" or "r1" or "r2" or "r3" or "null"
	std::string src = "null";       // "r0" or "r1" or "r2" or "r3" or "null"
	std::string argument = "null";  // "VAR: " or "LIT: " or "LBL: " or "null"
	std::string flag = "null";      // "FLAG_ZERO" or "FLAG_EQUAL" or "FLAG_GREATER_THAN" or "FLAG_LESS_THAN" or "null"

	// Label
	std::string labelName = "null";

	// Variable
	std::string variableName = "null";

	void print()
	{
		assert((type == "ASM") || (type == "VAR") || (type == "LBL"));
		std::cout << "Type: " << type;

		if (type == "ASM")
		{
			if (argument == "null") {
				assert(memorySize == 1);
			}
			else {
				assert(memorySize == 2);
			}
			std::cout << " Inst: " << inst;
			std::cout << " rD: " << dst;
			std::cout << " rS: " << src;
			std::cout << " Arg: " << argument;
			std::cout << " Flag: " << flag;
			std::cout << " Mem Size: " << memorySize;
			std::cout << " Address: " << memoryAddress;
			assert(labelName == "null");
			assert(variableName == "null");
			std::cout << "\n";
		}

		if (type == "VAR")
		{
			std::cout << " Variable Name: " << variableName;
			std::cout << " Mem Size: " << memorySize;
			std::cout << " Address: " << memoryAddress;
			assert(inst == "null");
			assert(dst == "null");
			assert(src == "null");
			assert(argument == "null");
			assert(flag == "null");
			assert(labelName == "null");
			std::cout << "\n";
		}

		if (type == "LBL")
		{
			std::cout << " Label Name: " << labelName;
			std::cout << " Mem Size: " << memorySize;
			std::cout << " Address: " << memoryAddress;
			assert(inst == "null");
			assert(dst == "null");
			assert(src == "null");
			assert(argument == "null");
			assert(flag == "null");
			assert(variableName == "null");
			std::cout << "\n";
		}
	}
};

void assertNoDuplicateVariableDeclarations(std::vector<SourceLine>& sourceLines)
{
	int numSourceLines = sourceLines.size();
	for (int i = 0; i < numSourceLines; i++)
	{
		SourceLine& line = sourceLines[i];
		if (line.type != "VAR") continue;
		std::string& name = line.variableName;

		for (int j = 0; j < numSourceLines; j++)
		{
			SourceLine& otherLine = sourceLines[j];
			if (otherLine.type != "VAR") continue;
			std::string& otherName = otherLine.variableName;

			if (name == otherName)
			{
				if (i != j)
				{
					assert(false);
				}
			}
		}
	}
}

void assertNoDuplicateLabelDeclarations(std::vector<SourceLine>& sourceLines)
{
	int numSourceLines = sourceLines.size();
	for (int i = 0; i < numSourceLines; i++)
	{
		SourceLine& line = sourceLines[i];
		if (line.type != "LBL") continue;
		std::string& name = line.labelName;

		for (int j = 0; j < numSourceLines; j++)
		{
			SourceLine& otherLine = sourceLines[j];
			if (otherLine.type != "LBL") continue;
			std::string& otherName = otherLine.labelName;

			if (name == otherName)
			{
				if (i != j)
				{
					assert(false);
				}
			}
		}
	}
}

void assertValidFlag(std::string token)
{
	assert((token == "FLAG_ZERO")          ||
		   (token == "FLAG_EQUAL")         ||
		   (token == "FLAG_GREATER_THAN")  ||
		   (token == "FLAG_LESS_THAN")     ||
		   (token == "!FLAG_ZERO")         ||
		   (token == "!FLAG_EQUAL")        ||
		   (token == "!FLAG_GREATER_THAN") ||
		   (token == "!FLAG_LESS_THAN"));
}

void assertValidRegister(std::string token)
{
	assert((token == "r0") || (token == "r1") || (token == "r2") || (token == "r3"));
}

void assertValidArgument(std::string token)
{
	std::string subToken = token.substr(0, 4);
	assert((subToken == "VAR:") || (subToken == "LBL:") || (subToken == "LIT:"));
}

std::vector<std::string> tokenize(const std::string& input)
{
	std::vector<std::string> result;
	std::istringstream iss(input);
	std::string word;
	while (iss >> word)
	{
		result.push_back(word);
	}
	return result;
}

SourceLine processRawSourceLine(std::string rawSourceLine)
{
	SourceLine sourceLine;
	std::vector<std::string> tokens = tokenize(rawSourceLine);

	if (tokens[0] == "var")
	{
		assert(tokens.size() == 2);
		sourceLine.type = "VAR";

		std::string secondToken = tokens[1];

		size_t openBraketIndex = secondToken.find('[');
		if (openBraketIndex != std::string::npos)
		{
			assert(secondToken.find(']') != std::string::npos);
			assert(secondToken.find('[') < secondToken.find(']') - 1);
			assert(secondToken.find(']') == secondToken.size() - 1);
			int sizeCharacterLength = secondToken.size() - openBraketIndex - 2;
			std::string sizeSubString = secondToken.substr(openBraketIndex + 1, sizeCharacterLength);
			sourceLine.memorySize = std::stoul(sizeSubString);
			assert(sourceLine.memorySize != 0);
			sourceLine.variableName = secondToken.substr(0, openBraketIndex);
			assert(sourceLine.variableName != "");
		}
		else
		{
			sourceLine.memorySize = 1;
			sourceLine.variableName = secondToken;
		}

		return sourceLine;
	}

	if (tokens[0] == "label")
	{
		assert(tokens.size() == 2);
		sourceLine.type = "LBL";
		sourceLine.labelName = tokens[1];
		return sourceLine;
	}

	if ((tokens[0] == "LDI") ||
		(tokens[0] == "LDR") ||
		(tokens[0] == "STR") ||
		(tokens[0] == "ADI"))
	{
		assert((tokens.size() == 3) || (tokens.size() == 4));
		assertValidRegister(tokens[1]);
		assertValidArgument(tokens[2]);
		if (tokens.size() == 4) {
			assertValidFlag(tokens[3]);
		}

		sourceLine.inst = tokens[0];
		sourceLine.dst = tokens[1];
		sourceLine.argument = tokens[2];
		if (tokens.size() == 4) {
			sourceLine.flag = tokens[3];
		}

		sourceLine.memorySize = 2;
		sourceLine.type = "ASM";
		return sourceLine;
	}

	if ((tokens[0] == "LDX") ||
		(tokens[0] == "STX") ||
		(tokens[0] == "ADD") ||
		(tokens[0] == "SUB") ||
		(tokens[0] == "AND") ||
		(tokens[0] == "ORR") ||
		(tokens[0] == "XOR") ||
		(tokens[0] == "CMP") ||
		(tokens[0] == "MOV"))
	{
		assert((tokens.size() == 3) || (tokens.size() == 4));
		assertValidRegister(tokens[1]);
		assertValidRegister(tokens[2]);
		if (tokens.size() == 4) {
			assertValidFlag(tokens[3]);
		}

		sourceLine.inst = tokens[0];
		sourceLine.dst = tokens[1];
		sourceLine.src = tokens[2];
		if (tokens.size() == 4) {
			sourceLine.flag = tokens[3];
		}

		sourceLine.memorySize = 1;
		sourceLine.type = "ASM";
		return sourceLine;
	}

	if ((tokens[0] == "SHR") ||
		(tokens[0] == "SHL") ||
		(tokens[0] == "KIN") ||
		(tokens[0] == "GSP") ||
		(tokens[0] == "PSH") ||
		(tokens[0] == "POP"))
	{
		assert((tokens.size() == 2) || (tokens.size() == 3));
		assertValidRegister(tokens[1]);
		if (tokens.size() == 3) {
			assertValidFlag(tokens[2]);
		}

		sourceLine.inst = tokens[0];
		sourceLine.dst = tokens[1];
		if (tokens.size() == 3) {
			sourceLine.flag = tokens[2];
		}

		sourceLine.memorySize = 1;
		sourceLine.type = "ASM";
		return sourceLine;
	}

	if ((tokens[0] == "DRW") ||
		(tokens[0] == "RTS"))
	{
		assert((tokens.size() == 1) || (tokens.size() == 2));
		if (tokens.size() == 2) {
			assertValidFlag(tokens[1]);
		}

		sourceLine.inst = tokens[0];
		if (tokens.size() == 2) {
			sourceLine.flag = tokens[1];
		}

		sourceLine.memorySize = 1;
		sourceLine.type = "ASM";
		return sourceLine;
	}

	if ((tokens[0] == "JMP") ||
		(tokens[0] == "JSR"))
	{
		assert((tokens.size() == 2) || (tokens.size() == 3));
		assertValidArgument(tokens[1]);
		if (tokens.size() == 3) {
			assertValidFlag(tokens[2]);
		}

		sourceLine.inst = tokens[0];
		sourceLine.argument = tokens[1];
		if (tokens.size() == 3) {
			sourceLine.flag = tokens[2];
		}

		sourceLine.memorySize = 2;
		sourceLine.type = "ASM";
		return sourceLine;
	}

	assert(false);
}

bool isOnlyWhitespace(std::string& line)
{
	return std::all_of(line.begin(), line.end(), [](unsigned char ch) { return std::isspace(ch); });
}

int main()
{
	std::vector<std::string> lines;
	std::ifstream file("test.txt");
	std::string line;
	while (std::getline(file, line))
	{
		lines.push_back(line);
	}
	file.close();

	std::vector<SourceLine> sourceLines;
	for (std::string& line : lines)
	{
		if (isOnlyWhitespace(line)) continue;
		if (line.substr(0, 2) == "//") continue;
		sourceLines.push_back(processRawSourceLine(line));
	}

	assertNoDuplicateVariableDeclarations(sourceLines);
	assertNoDuplicateLabelDeclarations(sourceLines);
	for (SourceLine& line : sourceLines)
	{
		line.print();
	}


	
	return 0;
}

