#pragma once
#include <sstream>
#include <QTextBrowser>
#include <QDateTime>
#include <QVector>
#include <QMap>


enum class ELogLevel
{
	kLog,
	kWarning,
	kError,
	kNone
};
enum class ELogGroup
{
	kLogTemp,
	kNone
};

namespace NutEnum
{
	ELogLevel ToELogLevel(const QString& s)
	{
		if (s.compare("Log")==0) return ELogLevel::kLog;
		else if (s.compare("Warning")==0) return ELogLevel::kWarning;
		else if (s.compare("Error")==0) return ELogLevel::kError;
		else return ELogLevel::kNone;
	}
	ELogGroup ToELogGroup(const QString& s)
	{
		if (s.compare("LogTemp")==0) return ELogGroup::kLogTemp;
		else return ELogGroup::kNone;
	}
}

namespace
{
	template<typename T>
	void format_helper(std::ostringstream& oss,std::string_view& str, const T& value)
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
	struct LogInfo
	{
		ELogLevel level_;
		ELogGroup group_;
		QString msg_;
		LogInfo(ELogLevel l, ELogGroup g, QString&& s)
		{
			level_ = l;
			group_ = g;
			msg_ = s;
		}
	};
	QVector<LogInfo> g_logs;
	QMap<ELogGroup, QString> g_group_map;
	QMap<ELogLevel, QString> g_level_map;
	QTextBrowser* p_texts_ = nullptr;
};


namespace Logger
{
#define ENUM_TO_GROUP(e) g_group_map.insert(ELogGroup::e,#e); 
#define ENUM_TO_LEVEL(e) g_level_map.insert(ELogLevel::e,#e); 
	void InitializeLogger(QTextBrowser* p,QComboBox* cbl, QComboBox* cbg)
	{
		p_texts_ = p;
		ENUM_TO_GROUP(kLogTemp)
		ENUM_TO_LEVEL(kLog)
		ENUM_TO_LEVEL(kWarning)
		ENUM_TO_LEVEL(kError)
		cbg->addItem("All");
		cbl->addItem("All");
		for (auto it = g_group_map.begin(); it != g_group_map.end(); it++)
			cbg->addItem(it.value().sliced(1));
		for (auto it = g_level_map.begin(); it != g_level_map.end(); it++)
			cbl->addItem(it.value().sliced(1));
	}
#undef ENUM_TO_STRING
#undef ENUM_TO_LEVEL
	template<typename... Targs>
	void Log(ELogLevel level, ELogGroup group,std::string_view str,Targs... args)
	{
		auto cur_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
		std::string s = format(str, args...);
		cur_time.append(" : ").append(QString::fromStdString(s));
		QString  text;
		if (level == ELogLevel::kError)
		{
			text = "<font color=\"#FF0000\">" + cur_time + "</font>";		
		}
		else if (level == ELogLevel::kWarning)
		{
			text = "<font color=\"#FFD700\">" + cur_time + "</font>";
		}
		else 
		{
			text = cur_time;
		}
		g_logs.append(LogInfo{ level,group,std::move(text) });
		p_texts_->append(g_logs.back().msg_);
	}
	template<typename... Targs>
	void Log(std::string_view str, Targs... args)
	{
		Log(ELogLevel::kLog, ELogGroup::kLogTemp, str, args...);
	}
	void Filter(ELogGroup group, ELogLevel level)
	{
		p_texts_->clear();
		if (level == ELogLevel::kNone && group == ELogGroup::kNone)
		{
			for (auto& log : g_logs)
				p_texts_->append(log.msg_);
			return;
		}
		else if (group == ELogGroup::kNone)
		{
			for (auto& log : g_logs)
			{
				if (log.level_ == level) p_texts_->append(log.msg_);
			}
			return;
		}
		else if (level == ELogLevel::kNone)
		{
			for (auto& log : g_logs)
			{
				if (log.group_ == group) p_texts_->append(log.msg_);
			}
			return;
		}

		for (auto& log : g_logs)
		{
			if (log.group_ == group && log.level_ == level) p_texts_->append(log.msg_);
		}
	}
}


