#ifndef __TIME_MANAGER_H__
#define __TIME_MANAGER_H__

#include "Framework/Interface/IRuntimeModule.h"
#include <chrono>

namespace Engine
{
	class TimerManager : public IRuntimeModule
	{
	public:
		using NutSecond = std::chrono::seconds;
		using NutMSecond = std::chrono::duration<float, std::ratio<1, 1000>>;
		using NutTimeStamp = std::chrono::high_resolution_clock::time_point;
		TimerManager() = default;
		int Initialize() override;
		void Finalize() override;
		void Tick() override;	
		void Pause();
		void Mark();
		void Resume();
		void Reset();
		float GetTickDeltaTime() const{return delta_time_;}
		float GetDeltaTime() const;
		float GetTotalTime();
	private:
		NutTimeStamp pre_stamp_;
		NutTimeStamp cur_stamp_;
		NutTimeStamp init_stamp_;
		NutTimeStamp pause_stamp_;
		NutTimeStamp mark_stamp_;
		float delta_time_;
		float total_time_;
		float pause_time_;
		bool b_stop_;
	};
	extern TimerManager* g_pTimerManager;
}
#endif // !__TIME_MANAGER_H__

