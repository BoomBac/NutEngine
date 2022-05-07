#ifndef __SKYBOX_PASS_H__
#define __SKYBOX_PASS_H__
#include "../Interface/IDrawPass.h"
namespace Engine
{
	class SkyBoxPass : public IDrawPass
	{
	public:
		~SkyBoxPass() = default;
		void Draw(Frame& frame) final;
	};
}
#endif // !__HUD_PASS_H__

