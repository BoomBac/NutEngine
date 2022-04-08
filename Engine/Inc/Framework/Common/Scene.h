#pragma once
#include "Framework/Common/SceneNode.h"

namespace Engine
{
    class Scene
    {
    public:
        Scene()
        {
            p_default_material_ = std::make_shared<SceneObjectMaterial>("default");
        }
        Scene(const char* scene_name) : SceneGraph(new BaseSceneNode(scene_name)) {}
        ~Scene() = default;
        void LoadResource();
    public:
        std::unique_ptr<BaseSceneNode> SceneGraph;
        std::unordered_multimap<std::string, std::shared_ptr<SceneCameraNode>>       CameraNodes;
        std::unordered_multimap<std::string, std::shared_ptr<SceneLightNode>>        LightNodes;
        std::unordered_multimap<std::string, std::shared_ptr<SceneGeometryNode>>     GeometryNodes;
        std::unordered_map<std::string, std::shared_ptr<SceneObjectCamera>>      Cameras;
        std::unordered_map<std::string, std::shared_ptr<SceneObjectLight>>       Lights;
        std::unordered_map<std::string, std::shared_ptr<SceneObjectMaterial>>    Materials;
        std::unordered_map<std::string, std::shared_ptr<SceneObjectGeometry>>    Geometries;

        const std::shared_ptr<SceneObjectCamera> GetCamera(std::string key) const;
        const std::shared_ptr<SceneCameraNode> GetFirstCameraNode() const;
        const std::shared_ptr<SceneCameraNode> GetNextCameraNode() const;

        const std::shared_ptr<SceneObjectLight> GetLight(std::string key) const;
        const std::shared_ptr<SceneLightNode> GetFirstLightNode() const;
        const std::shared_ptr<SceneLightNode> GetNextLightNode() const;

        const std::shared_ptr<SceneObjectGeometry> GetGeometry(std::string key) const;
        const std::shared_ptr<SceneGeometryNode> GetFirstGeometryNode() const;
        const std::shared_ptr<SceneGeometryNode> GetNextGeometryNode() const;

        const std::shared_ptr<SceneObjectMaterial> GetMaterial(std::string key) const;
        const std::shared_ptr<SceneObjectMaterial> GetFirstMaterial() const;
        const std::shared_ptr<SceneObjectMaterial> GetNextMaterial() const;
    private:
        std::shared_ptr<SceneObjectMaterial> p_default_material_;
    };
}