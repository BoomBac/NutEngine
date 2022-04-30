#ifndef __IDRAW_PASS_H__
#define __IDRAW_PASS_H__
#include "../Common/GfxStructures.h"
namespace Engine
{
	class IDrawPass
	{
	public:
		IDrawPass() = default;
		virtual ~IDrawPass() {};
		virtual void Draw(Frame& frame) = 0;
	};
}

#endif // !__IDRAW_PASS_H__

