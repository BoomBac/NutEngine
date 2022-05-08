#include "pch.h"
#include "Framework/Parser/JPEG.h"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "../External/stb-image/stb_image.h"

namespace Engine
{
	Image JpegParser::Parse(const Buffer& buf)
	{
		Image image{};
		int channel = 0;
		UINT8* data = stbi_load_from_memory(buf.GetData(),buf.GetDataSize(), reinterpret_cast<int*>(&image.width), reinterpret_cast<int*>(&image.height),&channel,0);
		image.channel = channel;
		image.bit_count = channel * 8;
		image.pitch = ((image.width * image.bit_count >> 3) + 3) & ~3;
		image.data_size = image.pitch * image.height;
		if (channel == 3)
		{
			image.format = EImageFormat::kNutFormatR8G8B8;
			image.data = std::move(reinterpret_cast<R8G8B8Unorm*>(data));
		}
		else if (channel == 4)
		{
			image.format = EImageFormat::kNutFormatR8G8B8A8;
			image.data = std::move(reinterpret_cast<R8G8B8A8Unorm*>(data));
		}
		return image;
	}
	//Image Engine::JpegParser::Parse(const std::string path)
	//{
	//	Image image{};
	//	int channel = 0;
	//	UINT8* data = stbi_load(path.c_str(), reinterpret_cast<int*>(&image.width), reinterpret_cast<int*>(&image.height), &channel, 0);
	//	image.bit_count = channel * 8;
	//	image.pitch = ((image.width * image.bit_count >> 3) + 3) & ~3;
	//	image.data_size = image.pitch * image.height;
	//	if (channel == 3)
	//	{
	//		image.format = EImageFormat::kNutFormatR8G8B8;
	//		image.data = std::move(reinterpret_cast<R8G8B8Unorm*>(data));
	//	}
	//	else if (channel == 4)
	//	{
	//		image.format = EImageFormat::kNutFormatR8G8B8A8;
	//		image.data = std::move(reinterpret_cast<R8G8B8A8Unorm*>(data));
	//	}
	//	return image;
	//}

}