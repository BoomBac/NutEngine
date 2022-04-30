#ifndef __SHADER_MANAGER_H__
#define __SHADER_MANAGER_H__

#include "../Interface/IShaderManager.h"
#include <unordered_map>

namespace Engine
{
	class ShaderManager : public IShaderManager
	{
	public:
		ShaderManager() = default;
		~ShaderManager() = default;

		virtual long long GetDefaultShaderProgram(EDefaultShaderType index) final
		{
			return default_shaders_[index];
		}
	private:
		std::unordered_map<const EDefaultShaderType, long long> default_shaders_;
	};
}

#endif //__SHADER_MANAGER_H__
