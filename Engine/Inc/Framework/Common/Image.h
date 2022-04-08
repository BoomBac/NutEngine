#pragma once
#include "../pch.h"
#include "Framework/Math/NutMath.hpp"

namespace Engine
{
	struct Image
	{
		uint32_t width;
		uint32_t height;
		//one pixel size on bit
		uint32_t bit_count;
		R8G8B8A8Unorm* data;
		//one row byte
		uint32_t pitch;
		// pitch * height  inclue memory align
		size_t data_size;
	};
}