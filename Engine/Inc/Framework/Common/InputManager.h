#pragma once
#include "Framework/Interface/IRuntimeModule.h"

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
		void SetPreMousePos(int32_t x, int32_t y);
	private:
		int16_t pre_mouse_pos[2];
	};
	extern InputManager* g_InputManager;
}