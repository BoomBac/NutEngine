#pragma once
#include "../pch.h"
#include <unordered_map>
#include "fbxsdk.h"
#include "Framework/Common/AssetLoader.h"
#include "Framework/Interface/ISceneParser.h"



namespace Engine
{
	class FbxParser : public ISceneParser
	{
	public:
		FbxParser();
		virtual ~FbxParser();
		unique_ptr<Scene> Parse(const std::string& buf) override;
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
		void ReadNormal(fbxsdk::FbxMesh* mesh, std::shared_ptr<SceneObjectMesh> nut_mesh);
		void ReadVertex(fbxsdk::FbxMesh* mesh, std::shared_ptr<SceneObjectMesh> nut_mesh);
		void ConvertFbxConstructToSceneNode(fbxsdk::FbxNode* object,std::shared_ptr<BaseSceneNode>& base_node,Scene& scene, fbxsdk::FbxScene* fbx_scene);
		std::shared_ptr<SceneObjectTransform> GenerateTransform(fbxsdk::FbxNode* p_node);
		bool bYup_ = true;
	};
}