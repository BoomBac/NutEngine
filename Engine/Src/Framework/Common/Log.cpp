#include "pch.h"
#include "Framework/Common/Log.h"


namespace Engine
{
	extern std::map<std::string, std::string> g_Config_map;
	int Engine::LogManager::Initialize()
	{
		int ret = ParseConfig();
		appenders_.push_back(new OutputAppender());
		return ret;
	}
	void Engine::LogManager::Finalize()
	{
		for(auto& logger : appenders_)
			delete logger;
	}
	void Engine::LogManager::Tick()
	{
	}
	void Engine::LogManager::AddAppender(IAppender* appender)
	{
		appenders_.push_back(appender);
	}
	int LogManager::ParseConfig()
	{
		std::map<std::string, std::string>::iterator it;
		std::stringstream ss;
		int ret = 0;
		if ((it = g_Config_map.find("TraceLevel")) != g_Config_map.end())
		{
			if(it->second == "Warning") output_level_ = ETraceLevel::kWarning;
			else if(it->second == "Error") output_level_ = ETraceLevel::kError;
			else if(it->second == "Normal") output_level_ = ETraceLevel::kNormal;
		}
		else ++ret;
		if ((it = g_Config_map.find("Markers")) != g_Config_map.end())
		{
			if (it->second == "ALL") output_mark_ = ALL;
			else if (it->second == "TRACE_1") output_mark_ = TRACE_1;
			else if (it->second == "TRACE_2") output_mark_ = TRACE_2;
			else if (it->second == "TRACE_3") output_mark_ = TRACE_3;
		}
		else ++ret;
		if ((it = g_Config_map.find("LogPath")) != g_Config_map.end())
		{
			output_path_ = it->second;
		}
		if(ret!=0)
		{
			NE_LOG(TRACE_1, kWarning, "not all config element be found,use the default config!")
				output_level_ = ETraceLevel::kNormal;
			output_mark_ = TRACE_1;
			output_path_ = "H:/Project_VS2019/NutEngine/Engine/log.txt";
		}
		return ret;
	}
}
