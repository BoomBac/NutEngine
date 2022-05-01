#ifndef __SHADOWMAP_PASS_H__
#define __SHADOWMAP_PASS_H__
#include "../Interface/IDrawPass.h"
namespace Engine
{
	class ShadowMapPass : public IDrawPass
	{
	public:
		~ShadowMapPass() = default;
		void Draw(Frame& frame) final;
	};
}
#endif // !__SHADOWMAP_PASS_H__

