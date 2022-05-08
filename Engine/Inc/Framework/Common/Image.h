#pragma once
#include "../pch.h"
#include "Framework/Math/NutMath.hpp"

namespace Engine
{
	enum class EImageFormat{
		kNutFormatR8G8B8,
		kNutFormatR8G8B8A8,
		kNutFormatR16G16B16A16,
		kNutFormatR16G16B16,
		kNutFormatR32G32B32A32,
		kNutFormatR32G32B32,
	};
	struct Image
	{
		uint32_t width;
		uint32_t height;
		//one pixel size on bit
		uint32_t channel;
		uint32_t bit_count;
		EImageFormat format;
		void* data;
		//one row byte
		uint32_t pitch;
		// pitch * height  inclue memory align
		size_t data_size;
		Image() : width(0), height(0),data(nullptr), bit_count(0),pitch(0),data_size(0),channel(0)
		{};
	};
}