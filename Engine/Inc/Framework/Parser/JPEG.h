#ifndef __IMAGE_PARSE_H__
#define __IMAGE_PARSE_H__
#include "../Interface/ImageParser.h"



namespace Engine
{
	class JpegParser : public IIMageParser
	{
	public:
		Image Parse(const Buffer& buf) final;
	};
}
#endif // !__IMAGE_PARSE_H__

