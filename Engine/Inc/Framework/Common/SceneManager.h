#pragma once
#include "Framework/Interface/IRuntimeModule.h"
#include "Scene.h"

namespace Engine
{
    class SceneManager : public IRuntimeModule
    {
    public:
        virtual ~SceneManager();
        int Initialize() override;
        void Finalize() override;
        void Tick() override;
        int LoadScene(const char* scene_file_name);
        int LoadScene();
        const Scene* GetSceneForRendering();
        bool IsSceneChanged();
        void NotifySceneIsRenderingQueued();
        void ResetScene();
        bool HasValidScene() const;
        int ParseConfig();
    protected:
        bool LoadFbxScene(const char* fbx_scene_file_name);
    protected:
        std::shared_ptr<Scene>  p_scene_;
        bool b_dirty_flag_ = false;
        bool b_has_scene_ = false;
        std::string default_scene_path_;
    };
    extern SceneManager* g_pSceneManager;
}
