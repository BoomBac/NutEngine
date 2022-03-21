// ConsoleTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <sstream>

using namespace std;

template<typename T>
void format_helper(std::ostringstream& oss,
	std::string_view& str, const T& value)
{
	std::size_t openBracket = str.find('{');
	if (openBracket == std::string::npos) { return; }
	std::size_t closeBracket = str.find('}', openBracket + 1);
	if (closeBracket == std::string::npos) { return; }
	oss << str.substr(0, openBracket) << value;
	str = str.substr(closeBracket + 1);
}

template<typename... Targs>
std::string format(std::string_view str, Targs...args)
{
	std::ostringstream oss;
	(format_helper(oss, str, args), ...);
	oss << str;
	return oss.str();
}

template<typename... Targs>
void Print(std::string_view str, Targs...args)
{
	cout << format(str, args...);
}

template<typename... Targs>
auto Add(Targs&&...args)
{
	//return (... + args);
	return (args + ...);
}

int main()
{
	//Print("这是二十{},b={}", 20, 30.25f);
	std::string a{ "Hello " };
	std::string b{ "World" };
	 cout << Add(a, b);
	 return 0;
}
