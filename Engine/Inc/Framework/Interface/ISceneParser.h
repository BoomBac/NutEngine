#pragma once
#include "../pch.h"
#include "../Common/SceneNode.h"


namespace Engine
{
    class ISceneParser
    {
    public:
        virtual std::unique_ptr<BaseSceneNode> Parse(const std::string & buf) = 0;
    };
}