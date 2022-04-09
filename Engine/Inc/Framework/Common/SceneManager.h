#pragma once
#include "Framework/Interface/IRuntimeModule.h"
#include "Scene.h"

namespace Engine
{
    class SceneManager : public IRuntimeModule
    {
    public:
        virtual ~SceneManager();
        virtual int Initialize();
        virtual void Finalize();
        virtual void Tick();
        int LoadScene(const char* scene_file_name);
        const Scene* GetSceneForRendering();
        bool IsSceneChanged();
        void NotifySceneIsRenderingQueued();
        void ResetScene();
        bool HasValidScene() const;
    protected:
        bool LoadFbxScene(const char* fbx_scene_file_name);
    protected:
        std::shared_ptr<Scene>  p_scene_;
        bool b_dirty_flag_ = false;
        bool b_has_scene_ = false;
    };
    extern SceneManager* g_pSceneManager;
}
