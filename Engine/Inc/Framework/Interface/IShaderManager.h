#ifndef __ISHADER_MANAGER_H__
#define __ISHADER_MANAGER_H__
#include "IRuntimeModule.h"

namespace Engine
{
	enum EDefaultShaderType
	{
		kShadowMap,kForward,kDiffered,kDebug
	};
	class IShaderManager : public IRuntimeModule
	{
	public:
		virtual ~IShaderManager() = default;
		virtual bool InitializeShaders() = 0;
		virtual void ClearShaders() = 0;
		virtual long long GetDefaultShaderProgram(EDefaultShaderType index) = 0;
	};
}

#endif // !__ISHADER_MANAGER_H__

