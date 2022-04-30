#ifndef __FORWARD_PASS_H__
#define __FORWARD_PASS_H__
#include "../Interface/IDrawPass.h"
namespace Engine
{
	class ForwardRenderPass : public IDrawPass
	{
	public:
		~ForwardRenderPass() = default;
		void Draw(Frame& frame) final;
	};
}
#endif // !__FORWARD_PASS_H__

