



#include <string>
#include <vector>
#include <sstream>
#include <cassert>




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
	std::string labelAlias = "null";

	// Variable
	std::string variableName = "null";
};

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
	std::string firstToken = tokens[0];

	if (firstToken == "var")
	{
		assert(tokens.size() == 2);
		sourceLine.type = "VAR";

		std::string secondToken = tokens[1];

		size_t openBraketIndex = secondToken.find('[');
		if (openBraketIndex != std::string::npos)
		{
			assert(secondToken.find(']') != std::string::npos);
			assert(secondToken.find('[') < secondToken.find(']'));
			assert(secondToken.find(']') == secondToken.size() - 1);
			int sizeCharacterLength = secondToken.size() - openBraketIndex - 2;
			std::string sizeSubString = secondToken.substr(openBraketIndex + 1, sizeCharacterLength);
			sourceLine.memorySize = std::stoul(sizeSubString);
			sourceLine.variableName = secondToken.substr(0, openBraketIndex);
		}
		else
		{
			sourceLine.memorySize = 1;
			sourceLine.variableName = secondToken;
		}

		return;
	}

	if (firstToken == "label")
	{
		assert(tokens.size() == 2);
		sourceLine.labelAlias = tokens[1];
		return;
	}

	if ((firstToken == "LDI") || (firstToken == "LDR") || (firstToken == "STR") || (firstToken == "ADI"))
	{
		assert((tokens.size() == 3) || (tokens.size() == 4));
		assertValidRegister(tokens[1]);
		assertValidArgument(tokens[2]);
		if (tokens.size() == 4)
		{
			assertValidFlag(tokens[3]);
		}

		sourceLine.inst = tokens[0];
		sourceLine.dst = tokens[1];
		sourceLine.argument = tokens[2];

		if (tokens.size() == 4)
		{
			sourceLine.flag = tokens[3];
		}

		sourceLine.memorySize = 2;
		return;
	}

	assert(false);
}

int main()
{

}


