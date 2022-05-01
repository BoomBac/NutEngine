#include "pch.h"
#include "Framework/DrawPass/ShadowMapPass.h"
#include "Framework/Common/GraphicsManager.h"

namespace Engine
{
	void ShadowMapPass::Draw(Frame& frame)
	{
		int index = 0;
		for(auto& light : frame.frame_context.lights_)
		{
			if(light.light_instensity > 0.f)
			{
				g_pGraphicsManager->BeginShadowMap(index);
				for (auto batch : frame.batch_contexts)
				{
					g_pGraphicsManager->DrawBatch(batch);
				}
				g_pGraphicsManager->EndShadowMap(index, false);
				light.shadow_map_index = index++;
			}

		}
		g_pGraphicsManager->EndShadowMap(index, true);
	}
}