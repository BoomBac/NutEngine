#include "pch.h"
#include "Framework/Common/TimerManager.h"

namespace Engine
{
	int TimerManager::Initialize()
	{
		int ret = 0;
		init_stamp_ = pre_stamp_;
		delta_time_ = 0.f;
		total_time_ = 0.f;
		pause_time_ = 0.f;
		b_stop_ = false;
		return ret;
	}
	void TimerManager::Finalize()
	{
		b_stop_ = true;
	}
	void Engine::TimerManager::Tick()
	{
		if(b_stop_)
		{
			delta_time_ = 0.0f;
			return;
		}
		cur_stamp_ = std::chrono::high_resolution_clock::now();
		delta_time_ = NutMSecond(cur_stamp_ - pre_stamp_).count();
		pre_stamp_ = cur_stamp_;
	}
	void TimerManager::Pause()
	{
		if(!b_stop_)
		{
			pause_stamp_ = std::chrono::high_resolution_clock::now();
			b_stop_ = true;
		}
	}
	void TimerManager::Mark()
	{
		mark_stamp_ = std::chrono::high_resolution_clock::now();
	}
	void TimerManager::Resume()
	{
		if(b_stop_)
		{
			pause_time_ += NutMSecond(std::chrono::high_resolution_clock::now() - pause_stamp_).count();
			pre_stamp_ = std::chrono::high_resolution_clock::now();
			b_stop_ = false;
		}
	}
	void TimerManager::Reset()
	{
		pre_stamp_ = std::chrono::high_resolution_clock::now();
	}
	float TimerManager::GetDeltaTime() const
	{
		return NutMSecond(std::chrono::high_resolution_clock::now() - mark_stamp_).count();
	}
	float TimerManager::GetTotalTime()
	{
		if(b_stop_)
		{
			return NutMSecond(pause_stamp_ - init_stamp_).count() - pause_time_;
		}
		else
			return NutMSecond(cur_stamp_ - init_stamp_).count() - pause_time_;
	}
}