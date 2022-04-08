#pragma once
#include "../pch.h"
#include "Framework/Common/Scene.h"


namespace Engine
{
    class ISceneParser
    {
    public:
        virtual std::unique_ptr<Scene> Parse(const std::string & buf) = 0;
    };
}