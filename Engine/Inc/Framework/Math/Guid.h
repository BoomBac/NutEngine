#pragma once
#include "../pch.h"


namespace Engine
{

	class Guid
	{
	public:
		Guid() { uid_ = "uidd"; }
	private:
		std::string uid_;
	};
	Guid NewGuid();
}