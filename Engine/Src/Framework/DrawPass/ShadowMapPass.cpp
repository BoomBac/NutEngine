#include "pch.h"
#include "Framework/DrawPass/ShadowMapPass.h"
#include "Framework/Common/GraphicsManager.h"

namespace Engine
{
	void ShadowMapPass::Draw(Frame& frame)
	{
		int index = 0;
		int point_light_index = 0;
		int non_point_light_index = 0;
		int light_type = 0;
		//if needn't update shaodw map,just set the index
		if(!g_pGraphicsManager->REGenerateShadowMap()) 
		{
			g_pGraphicsManager->SetShadowMap();
			for (auto& light : frame.frame_context.lights_)
			{
				if (light.light_instensity > 0.f)
				{
					light_type = light.type & 0x03;
					if (light_type == 1)
						light.shadow_map_index = point_light_index++;
					else
						light.shadow_map_index = non_point_light_index++;
				}			
			}
			return;
		}
		for(auto& light : frame.frame_context.lights_)
		{
			if(light.light_instensity > 0.f)
			{
				light_type = light.type & 0x03;
				if(light_type == 1)
				{
					for(int i = 0; i < 6; ++i)
					{
						g_pGraphicsManager->BeginShadowMap(light.type, index,true,point_light_index,i);
						for (auto batch : frame.batch_contexts)
						{
							g_pGraphicsManager->DrawBatch(batch);
						}
						g_pGraphicsManager->EndShadowMap(light.type);
					}
					light.shadow_map_index = point_light_index++;
				}
				else
				{
					g_pGraphicsManager->BeginShadowMap(light.type, index,true);
					for (auto batch : frame.batch_contexts)
					{
						g_pGraphicsManager->DrawBatch(batch);
					}
					g_pGraphicsManager->EndShadowMap(light.type);
					light.shadow_map_index = non_point_light_index++;
				}
			}
			++index;
		}
		g_pGraphicsManager->EndShadowMap();
	}
}