#ifndef __HUD_PASS_H__
#define __HUD_PASS_H__
#include "../Interface/IDrawPass.h"
namespace Engine
{
	class HUDPass : public IDrawPass
	{
	public:
		~HUDPass() = default;
		void Draw(Frame& frame) final;
	};
}
#endif // !__HUD_PASS_H__

