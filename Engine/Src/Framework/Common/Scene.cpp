#include "pch.h"
#include "Framework/Common/Scene.h"

using namespace Engine;

void Engine::Scene::LoadResource()
{

}

const std::shared_ptr<SceneObjectCamera> Engine::Scene::GetCamera(std::string key) const
{
	auto i = Cameras.find(key);
	if (i == Cameras.end())
		return nullptr;
	else
		return i->second;
}

const std::shared_ptr<SceneCameraNode> Engine::Scene::GetFirstCameraNode() const
{
	return CameraNodes.empty() ? nullptr : CameraNodes.cbegin()->second;
}

const std::shared_ptr<SceneCameraNode> Engine::Scene::GetNextCameraNode() const
{
	static thread_local auto _it = CameraNodes.cbegin();
	if (_it == CameraNodes.cend()) return nullptr;
	return ((++_it == CameraNodes.cend()) ? nullptr : _it->second);
}

const std::shared_ptr<SceneObjectLight> Engine::Scene::GetLight(std::string key) const
{
	auto i = Lights.find(key);
	if (i == Lights.end())
		return nullptr;
	else
		return i->second;
}

const std::shared_ptr<SceneLightNode> Engine::Scene::GetFirstLightNode() const
{
	return LightNodes.empty() ? nullptr : LightNodes.cbegin()->second;
}

const std::shared_ptr<SceneLightNode> Engine::Scene::GetNextLightNode() const
{
	static thread_local auto _it = LightNodes.cbegin();
	if (_it == LightNodes.cend()) return nullptr;
	return ((++_it == LightNodes.cend()) ? nullptr : _it->second);
}


const std::shared_ptr<SceneObjectMaterial> Engine::Scene::GetMaterial(std::string key) const
{
	auto i = Materials.find(key);
	if (i == Materials.end())
		return nullptr;
	else
		return i->second;
}

const std::shared_ptr<SceneObjectMaterial> Engine::Scene::GetFirstMaterial() const
{
	return (Materials.empty() ? nullptr : Materials.cbegin()->second);
}

const std::shared_ptr<SceneObjectMaterial> Engine::Scene::GetNextMaterial() const
{
	static thread_local auto _it = Materials.cbegin();
	if (_it == Materials.cend()) return nullptr;
	return ((++_it == Materials.cend()) ? nullptr : _it->second);
}

const std::shared_ptr<SceneObjectGeometry> Engine::Scene::GetGeometry(std::string key) const
{
	auto i = Geometries.find(key);
	if (i == Geometries.end())
		return nullptr;
	else
		return i->second;
}

const std::shared_ptr<SceneGeometryNode> Engine::Scene::GetFirstGeometryNode() const
{
	return GeometryNodes.empty() ? nullptr : GeometryNodes.cbegin()->second;
}

const std::shared_ptr<SceneGeometryNode> Engine::Scene::GetNextGeometryNode() const
{
	static thread_local auto _it = GeometryNodes.cbegin();
	if (_it == GeometryNodes.cend()) return nullptr;
	return ((++_it == GeometryNodes.cend()) ? nullptr : _it->second);
}
