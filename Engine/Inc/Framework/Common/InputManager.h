#pragma once
#include "Framework/Interface/IRuntimeModule.h"
#include <thread>


namespace Engine
{
	enum class EKeyButton
	{
		kW, kA, kS, kD
	};
	enum class EMoveType
	{
		kPositiveX,
		kNegativeX,
		kPositiveY,
		kNegativeY,
	};
	struct MouseMoveEvent
	{
		int16_t x_;
		int16_t y_;
		MouseMoveEvent(){};
		MouseMoveEvent(int16_t x, int16_t y) : x_(x),y_(y){}
	};
	class InputManager : IRuntimeModule
	{
	public:
		int Initialize() override;
		void Finalize() override;
		void Tick() override;
		void ButtonDown(EKeyButton btn);
		void ButtonUp(EKeyButton btn);
		void MouseMove(const MouseMoveEvent& ev);
		void SetPreMousePos(int x, int y);
		void ProcessInput(MouseMoveEvent* ev);
	private:
		int16_t pre_mouse_pos[2];
		MouseMoveEvent cur_mouse_ev_;
		std::thread thread_input_handler_;
	};
	extern InputManager* g_InputManager;
}