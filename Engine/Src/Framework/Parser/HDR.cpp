#include "pch.h"
#include "Framework/Parser/HDR.h"
#include "Framework/Common/Log.h"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "../External/stb-image/stb_image.h"



namespace Engine
{
	Image Engine::HDRParser::Parse(const Buffer& buf)
	{
		Image image{};
		//float* data = stbi_loadf_from_memory(buf.GetData(),buf.GetDataSize(), reinterpret_cast<int*>(&image.width), reinterpret_cast<int*>(&image.height), 
		//	reinterpret_cast<int*>(&image.channel),0);
		float* data = stbi_loadf("H:/Project_VS2019/NutEngine/Engine/Asset/env_img/Eden_REF.hdr", reinterpret_cast<int*>(&image.width), reinterpret_cast<int*>(&image.height),
			reinterpret_cast<int*>(&image.channel), 0);
		image.bit_count = image.channel * 32;
		image.pitch = image.width * (image.bit_count >> 3);
		image.data_size = image.pitch * image.height;
		if (image.channel == 3)
		{
			image.format = EImageFormat::kNutFormatR32G32B32;
			image.data = std::move(reinterpret_cast<R32G32B32Float*>(data));
		}
		else if (image.channel == 4)
		{
			image.format = EImageFormat::kNutFormatR32G32B32A32;
			image.data = std::move(reinterpret_cast<R32G32B32A32Float*>(data));
		}
		//float* color= reinterpret_cast<float*>(image.data);
		//int count = 0;
		//for(int i = 0; i < image.height; ++i)
		//{
		//	for (int j = 0; j < image.width; ++j)
		//	{
		//		NE_LOG(ALL, kWarning, "R:{} G:{} B:{} count{}", color[0], color[1], color[2],count++)
		//		color += 3;
		//	}

		//}
		return image;
	}

}