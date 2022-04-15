#include "pch.h"
#include "Framework/Common/InputManager.h"
#include "Framework/Common/GraphicsManager.h"

int Engine::InputManager::Initialize()
{
	int ret = 0;
	return ret;
}

void Engine::InputManager::Finalize()
{
}

void Engine::InputManager::Tick()
{
}

void Engine::InputManager::ButtonDown(EKeyButton btn)
{
	if(btn == EKeyButton::kD)
	{
		g_pGraphicsManager->MoveCameraRight(10.f);
	}
	else if (btn == EKeyButton::kA)
	{
		g_pGraphicsManager->MoveCameraRight(-10.f);
	}
	else if(btn == EKeyButton::kW)
	{
		g_pGraphicsManager->MoveCameraForward(10.f);
	}
	else if (btn == EKeyButton::kS)
	{
		g_pGraphicsManager->MoveCameraForward(-10.f);
	}
}

void Engine::InputManager::ButtonUp(EKeyButton btn)
{
}

void Engine::InputManager::MouseMove(const MouseMoveEvent& ev)
{
	float dx = static_cast<float>(ev.x_ - pre_mouse_pos[0]) * 0.25f;
	float dy = static_cast<float>(ev.y_ - pre_mouse_pos[1]) * 0.25f;
	g_pGraphicsManager->CameraRotateYaw(dx);
	pre_mouse_pos[0] = ev.x_;
	pre_mouse_pos[1] = ev.y_;
}

void Engine::InputManager::SetPreMousePos(int32_t x, int32_t y)
{
	pre_mouse_pos[0] = static_cast<int32_t>(x);
	pre_mouse_pos[1] = static_cast<int32_t>(y);
}


