
#include <string>
#include <vector>
#include <sstream>
#include <cassert>

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
	}

	assert(false);
}

int main()
{

}

