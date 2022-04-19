#pragma once
#include "../pch.h"
#include <unordered_map>
#include "fbxsdk.h"
#include "Framework/Common/AssetLoader.h"
#include "Framework/Interface/ISceneParser.h"
#include "Framework/Common/ThreadPool.h"

using namespace std;


namespace Engine
{
	class FbxParser : public ISceneParser
	{
	public:
		FbxParser();
		virtual ~FbxParser();
		unique_ptr<Scene> Parse(const std::string & file_path) override;
	private:
		std::unordered_map<string, std::shared_ptr<BaseSceneObject>> scene_objects_;
		FbxManager* fbx_manager_;
		FbxIOSettings* fbx_ios_;
		FbxImporter* fbx_importer_;
	private:
		void GenerateCamera(const fbxsdk::FbxCamera* camera, Scene& scene);
		void GenerateMaterial(const fbxsdk::FbxSurfaceMaterial* mat, Scene& scene);
		void GenerateLight(const fbxsdk::FbxLight* light, Scene& scene);
		void GenerateMesh(std::shared_ptr<SceneObjectGeometry> geo, fbxsdk::FbxMesh* mesh, Scene& scene);
		void ReadNormal(const fbxsdk::FbxMesh& mesh, std::shared_ptr<SceneObjectMesh> nut_mesh);
		void ReadVertex(const fbxsdk::FbxMesh& mesh, std::shared_ptr<SceneObjectMesh> nut_mesh);
		bool ConvertFbxConstructToSceneNode(fbxsdk::FbxNode* object,std::shared_ptr<BaseSceneNode>& base_node,Scene& scene, fbxsdk::FbxScene* fbx_scene);
		std::shared_ptr<SceneObjectTransform> GenerateTransform(fbxsdk::FbxNode* p_node);
		unique_ptr<ThreadPool> p_thread_pool_;
		bool bYup_ = true;
	};
}