#include "pch.h"
#include "Framework/DrawPass/ForwardRenderPass.h"
#include "Framework/Common/GraphicsManager.h"

namespace Engine
{
	void ForwardRenderPass::Draw(Frame& frame)
	{
		g_pGraphicsManager->DrawBatch(frame.batch_contexts);
	}

}