#ifndef __GLOBAL_H__
#define  __GLOBAL_H__
#include "../pch.h"

using std::map;
using std::string;



namespace Engine
{
	map<string,string> g_Config_map;
	static void LoadConfigFile(const char* path)
	{
		std::ifstream in(path);
		assert(in.is_open());
		std::string str;
		while (!in.eof())
		{
			std::string  key, value;
			std::getline(in, str);
			if (str[0] == ';' || str[0] == '[' || str.empty()) continue;
			size_t pos = str.find("=");
			size_t pre = pos-1,next = pos+1;
			while (str[pre] == ' ') --pre;
			key = str.substr(0, pre + 1);
			while (str[next] == ' ') ++next;
			value = str.substr(next);
			g_Config_map.emplace(std::move((std::make_pair(key, value))));
		}
	}
}

#endif // !__GLOBAL_H__

