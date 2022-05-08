#ifndef __HDR_H__
#define __HDR_H__
#include "../Interface/ImageParser.h"

namespace Engine
{
	class HDRParser : public IIMageParser
	{
	public:
		Image Parse(const Buffer& buf) final;
	};
}
#endif // __HDR_H__

