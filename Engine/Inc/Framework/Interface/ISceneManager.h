#pragma once
#include "IRuntimeModule.h"
#include "ISceneParser.h"

namespace Engine
{
    class ISceneManager : public IRuntimeModule
    {
    public:
        virtual ~ISceneManager();
        virtual int Initialize();
        virtual void Finalize();
        virtual void Tick();
        void LoadScene(const char* scene_file_name);
        const Scene& GetSceneForRendering();
    protected:
        void LoadFbxScene(const char* fbx_scene_file_name);
    protected:
        std::unique_ptr<Scene>  p_scene_;
    };
    extern ISceneManager* g_pSceneManager;
}
