// ConsoleTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <sstream>
#include "Framework/Math/Math.h"

using namespace std;

//template<typename T>
//void format_helper(std::ostringstream& oss,
//	std::string_view& str, const T& value)
//{
//	std::size_t openBracket = str.find('{');
//	if (openBracket == std::string::npos) { return; }
//	std::size_t closeBracket = str.find('}', openBracket + 1);
//	if (closeBracket == std::string::npos) { return; }
//	oss << str.substr(0, openBracket) << value;
//	str = str.substr(closeBracket + 1);
//}
//
//template<typename... Targs>
//std::string format(std::string_view str, Targs...args)
//{
//	std::ostringstream oss;
//	(format_helper(oss, str, args), ...);
//	oss << str;
//	return oss.str();
//}
//
//template<typename... Targs>
//void Print(std::string_view str, Targs...args)
//{
//	cout << format(str, args...);
//}
//
//template<typename... Targs>
//auto Add(Targs&&...args)
//{
//	//return (... + args);
//	return (args + ...);
//}
//
//#ifndef ALIGN
//#define ALIGN(x,a)	(((x) + ((a)-1)) & ~((a) -1))
//#endif // !ALIGN
//
//template<typename T, size_t size_of_arr>
//constexpr size_t CountOf(T(&)[size_of_arr])
//{
//	return size_of_arr;
//}
//
//template<typename T,typename ... args>
//int func(T var, args... arg)
//{
//	T arr[sizeof...(arg)];
//	for (int i = 0; i < sizeof...(arg); i++)
//		arr[i] = i;
//	return (arr[arg] + ...);
//}

int main()
{
	Engine::Vector2f v1(1.f, 2.f);
	Engine::Vector2f v2(1.f, 2.f);
	Engine::Vector2f v3(1.f, 2.f);
	Engine::Matrix4x4f mat = 
	{{{
		{1,2,3,4},
		{1,2,3,4},
		{1,2,3,4},
		{1,2,3,4}
	}}};
	auto str = Engine::MatrixToString(mat);
	cout <<  str << endl;
	cout <<  str << endl;
	return 0;
}
