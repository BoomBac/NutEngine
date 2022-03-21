#pragma once

#include "pch.h"

namespace Engine
{
	void InitPipeline(HWND hWnd);
	void LoadAsset();
	void OnRender();
	void PopulateCommandList();
	void WaitForPreviousFrame();
	void OnDestroy();
}

