#ifndef __FILTER_CUBE_MAP_PASS__
#define __FILTER_CUBE_MAP_PASS__
#include "../Interface/IDrawPass.h"
#include "RHI/D3D12GrahpicsManager.h"
namespace Engine
{
	class FilterCubeMapPass : public IDrawPass
	{
	public:
		~FilterCubeMapPass() = default;

		void Draw(Frame& frame) final
		{
			auto d3d = dynamic_cast<D3d12GraphicsManager*>(g_pGraphicsManager);
			for(int i = 0; i < 6; ++i)
			{
				d3d->BeginSkyBox(i,1);
				d3d->DrawSkyBox(1);
				d3d->EndSkyBox(i,1);
			}
		}
	};
}
#endif // !__FILTER_CUBE_MAP_PASS__

