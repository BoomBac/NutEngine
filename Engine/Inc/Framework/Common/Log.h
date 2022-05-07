#ifndef __LOG_MANAGER_H__
#define __LOG_MANAGER_H__
#include "../pch.h"
#include "Framework/Interface/IRuntimeModule.h"

namespace Engine
{
	template<typename T>
	static void format_helper(std::ostringstream& oss, std::string_view& str, const T& value)
	{
		std::size_t openBracket = str.find('{');
		if (openBracket == std::string::npos) { return; }
		std::size_t closeBracket = str.find('}', openBracket + 1);
		if (closeBracket == std::string::npos) { return; }
		oss << str.substr(0, openBracket) << value;
		str = str.substr(closeBracket + 1);
	}
	template<typename... Targs>
	static std::string format(std::string_view str, Targs...args)
	{
		std::ostringstream oss;
		(format_helper(oss, str, args), ...);
		oss << str;
		return oss.str();
	}
	static const char* log_str[]{ "Normal","Warning","Error" };
	constexpr uint8_t ALL = 0;
	constexpr uint8_t TRACE_1 = 1 << 0;
	constexpr uint8_t TRACE_2 = 1 << 1;
	constexpr uint8_t TRACE_3 = 1 << 2;
	enum ETraceLevel
	{
		kNormal,
		kWarning,
		kError,
	};
	class IAppender
	{
	public:
		virtual void Print(std::string str) = 0;
	};
	class ConsoleAppender : public IAppender
	{
	public:
		void Print(std::string str) override
		{
			std::cout << str.c_str() << std::endl;
		}
	};
	class FileAppender : public IAppender
	{
		std::string out_path_ = "H:/Project_VS2019/NutEngine/Engine/log.txt";
	public:
		void Print(std::string str) override
		{
			std::ofstream out(out_path_, std::ios_base::app);
			if (out.is_open())
			{
				out << str << std::endl;
				out.close();
			}
		}
	};
	class OutputAppender : public IAppender
	{
	public:
		OutputAppender() = default;
		void Print(std::string str) override
		{
			str.append("\r\n");
			OutputDebugStringA(str.c_str());
		}
	};
	class LogManager : public IRuntimeModule
	{
		std::vector<IAppender*> appenders_;
		ETraceLevel output_level_;
		uint16_t output_mark_;
		std::string output_path_;
	public:
		int Initialize() override;
		void Finalize() override;
		void Tick() override;
		void AddAppender(IAppender* appender);
		const std::vector<IAppender*>& GetAppenders() const {return appenders_;}
		const ETraceLevel& GetOutputLevel() const {return output_level_;}
		const uint16_t& GetOutputMark() const {return output_mark_;}
		const std::string& GetOutputPath() const{return output_path_;}
	private:
		int ParserConfig();
	};
	extern LogManager* g_pLogManager;
	template<typename... Targs>
	static std::string _Log(ETraceLevel level, std::string_view str, Targs... args)
	{
		std::string s = log_str[level];
		s.append(": ");
		s.append(format(str, args...));
		return s;
	}
	template<typename... Targs>
	static void Log(uint8_t maker, ETraceLevel level, std::string_view str, Targs... args)
	{
		if (level >= g_pLogManager->GetOutputLevel() && (maker == 0 || (maker & g_pLogManager->GetOutputMark()) != 0))
		{
			for (auto& app : g_pLogManager->GetAppenders())
			{
				app->Print(_Log(level, str, args...));
			}			
		}
	}
#define NE_LOG(maker,Level,msg,...) Log(maker,Level,msg,##__VA_ARGS__);
}

using Engine::ALL;
using Engine::ETraceLevel;

#endif // !__LOG_MANAGER_H__

