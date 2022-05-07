#include "pch.h"
#include "Framework/DrawPass/SkyBoxPass.h"
#include "Framework/Common/GraphicsManager.h"

namespace Engine
{
	void SkyBoxPass::Draw(Frame& frame)
	{
		g_pGraphicsManager->DrawSkyBox();
	}

}