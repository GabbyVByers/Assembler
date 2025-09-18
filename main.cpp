
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

void assertCharacterIsNumber(char ch)
{
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

void assertAssemblyAndMemoryCoherence(std::vector<SourceLine>& sourceLines)
{
	for (SourceLine& line : sourceLines)
	{
		if (line.type == "ASM")
		{
			if (line.argument == "null")
			{
				assert(line.memorySize == 1);
			}
			if (line.argument != "null")
			{
				assert(line.memorySize == 2);
			}
		}
	}
	for (SourceLine& line : sourceLines)
	{
		if (line.type == "ASM")
		{
			assert(line.inst != "null");
		}
		if (line.type != "ASM")
		{
			assert(line.inst == "null");
			assert(line.dst == "null");
			assert(line.src == "null");
			assert(line.argument == "null");
			assert(line.flag == "null");
		}
	}
}

void assertVariableDeclarationsAreEndOfSourceFile(std::vector<SourceLine>& sourceLines)
{
	int numSourceLines = sourceLines.size();

	bool haveSeenAVariable = false;
	for (int i = 0; i < numSourceLines; i++)
	{
		SourceLine& sourceLine = sourceLines[i];
		if (sourceLine.type == "VAR")
		{
			haveSeenAVariable = true;
		}

		if (haveSeenAVariable)
		{
			if (sourceLine.type != "VAR")
			{
				assert(false);
			}
		}
	}
}

void assertNoDuplicateVariableDeclarations(std::vector<SourceLine>& sourceLines)
{
	int numSourceLines = sourceLines.size();

	for (int i = 0; i < numSourceLines; i++)
	{
		SourceLine& sourceLine = sourceLines[i];
		if (sourceLine.type != "VAR") continue;
		std::string name = sourceLine.variableName;
		assert(name != "null");

		for (int j = 0; j < numSourceLines; j++)
		{
			SourceLine& otherSourceLine = sourceLines[j];
			if (otherSourceLine.type != "VAR") continue;
			std::string otherName = otherSourceLine.variableName;
			assert(otherName != "null");

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

void assertNoDuplicateLabels(std::vector<SourceLine>& sourceLines)
{
	int numSourceLines = sourceLines.size();

	for (int i = 0; i < numSourceLines; i++)
	{
		SourceLine& sourceLine = sourceLines[i];
		if (sourceLine.type != "LBL") continue;
		std::string name = sourceLine.labelName;
		assert(name != "null");

		for (int j = 0; j < numSourceLines; j++)
		{
			SourceLine& otherSourceLine = sourceLines[j];
			if (otherSourceLine.type != "LBL") continue;
			std::string otherName = otherSourceLine.labelName;
			assert(otherName != "null");

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

void assertTokenIsValidRegister(std::string token)
{
	assert((token == "r0") ||
		   (token == "r1") ||
		   (token == "r2") ||
		   (token == "r3"));
}

void assertTokenIsValidArgument(std::string token)
{
	assert(token.size() >= 2);
	assert((token[0] == '#') ||
		   (token[0] == '&') ||
		   (token[0] == '.'));
}

void assertTokenIsValidFlag(std::string token)
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

SourceLine processTokens(std::vector<std::string> tokens)
{
	SourceLine sourceLine;
	assert(tokens.size() != 0);

	if (tokens[0] == "var")
	{
		assert(tokens.size() == 2);
		assert(tokens[1].size() != 0);
		if (tokens[1].find('[') != std::string::npos)
		{
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
		else
		{
			assert(tokens[1].find('[') == std::string::npos);
			assert(tokens[1].find(']') == std::string::npos);
			sourceLine.variableName = tokens[1];
			sourceLine.memorySize = 1;
		}
		sourceLine.type = "VAR";
		return sourceLine;
	}

	if (tokens[0] == "label")
	{
		assert(tokens.size() == 2);
		assert(tokens[1].size() != 0);
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
		(tokens[0] == "CMP"))
	{
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
		(tokens[0] == "POP"))
	{
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
		(tokens[0] == "RTS"))
	{
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
		(tokens[0] == "JSR"))
	{
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
	for (SourceLine& line : sourceLines)
	{
		line.memoryAddress = currentAddress;
		currentAddress += line.memorySize;
	}

	for (SourceLine& line : sourceLines)
	{
		if (line.argument == "null") continue;

		if (line.argument[0] == '#')
		{
			std::string trueArgument = line.argument.substr(1, line.argument.size() - 1);
			assert((line.inst == "LDI") || (line.inst == "ADI"));
			assert(trueArgument.size() >= 1);
			for (char& ch : trueArgument) { assertCharacterIsNumber(ch); }
			line.argument = trueArgument;
			continue;
		}

		if (line.argument[0] == '&')
		{
			std::string targetVariable = line.argument.substr(1, line.argument.size() - 1);
			assert((line.inst == "LDI") || (line.inst == "LDR") || (line.inst == "STR"));
			assert(targetVariable.size() >= 1);

			for (SourceLine& otherLine : sourceLines)
			{
				if (otherLine.type != "VAR") continue;
				assert(otherLine.variableName != "null");
				if (otherLine.variableName == targetVariable)
				{
					line.argument = std::to_string(otherLine.memoryAddress);
				}
			}
		}

		if (line.argument[0] == '.')
		{
			std::string targetLabel = line.argument.substr(1, line.argument.size() - 1);
			assert((line.inst == "JMP") || (line.inst == "JSR"));
			assert(targetLabel.size() >= 1);

			for (SourceLine& otherLine : sourceLines)
			{
				if (otherLine.type != "LBL") continue;
				assert(otherLine.labelName != "null");
				if (otherLine.labelName == targetLabel)
				{
					line.argument = std::to_string(otherLine.memoryAddress);
				}
			}
		}
	}

	return 0;
}

