#include "pch.h"
#include "Framework/DrawPass/HUDPass.h"
#include "Framework/Common/GraphicsManager.h"

namespace Engine
{
	void HUDPass::Draw(Frame& frame)
	{
		g_pGraphicsManager->DrawOverlay();
	}

}