// ConsoleTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <sstream>
#include <unordered_map>
#include <utility>

#include "Framework/Parser/FbxParser.h"



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


namespace Engine
{
	MemoryManager* g_pMemoryManager = new MemoryManager();
	AssetLoader* g_pAssetLoader = new AssetLoader();
}

int main()
{
	//FILE* fp = fopen("H:/Project_VS2019/NutEngine/Engine/Asset/box.fbx", "rb");
	//if (!fp) return false;
	Engine::g_pMemoryManager->Initialize();
	Engine::g_pAssetLoader->Initialize();
	Engine::g_pAssetLoader->AddSearchPath("H:/Project_VS2019/NutEngine/Engine");
	string fbx_name = "box.fbx";
	Engine::ISceneParser* parser = new Engine::FbxParser();
	auto root = parser->Parse(fbx_name);
	//loader.OpenAndReadTextSync("C:\\Users\\22292\\Desktop\\box.fbx");
	//fseek(fp, 0, SEEK_END);
	//long file_size = ftell(fp);
	//fseek(fp, 0, SEEK_SET);
	//auto* content = new ofbx::u8[file_size];
	//fread(content, 1, file_size, fp);

	//delete[] content;
	return 0;
}
