#pragma once
#include "../pch.h"

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
		inline static const char* kAssetTexturePath = "H:/Project_VS2019/NutEngine/Engine/Asset/Img/";
		std::unordered_map<string, std::shared_ptr<BaseSceneObject>> scene_objects_;
		FbxManager* fbx_manager_;
		FbxIOSettings* fbx_ios_;
		FbxImporter* fbx_importer_;
	private:
		void GenerateCamera(const fbxsdk::FbxCamera* camera, Scene& scene);
		void GenerateMaterial(const fbxsdk::FbxSurfaceMaterial* mat, Scene& scene);
		void GenerateLight(const fbxsdk::FbxLight* light, Scene& scene);
		bool GenerateMesh(std::shared_ptr<SceneObjectGeometry> geo, fbxsdk::FbxMesh* mesh, Scene& scene);
		bool ReadNormal(const fbxsdk::FbxMesh& mesh, std::shared_ptr<SceneObjectMesh> nut_mesh);
		bool ReadVertex(const fbxsdk::FbxMesh& mesh, std::shared_ptr<SceneObjectMesh> nut_mesh);
		bool ReadUVs(const fbxsdk::FbxMesh& mesh, std::shared_ptr<SceneObjectMesh> nut_mesh);
		bool ReadTangent(const fbxsdk::FbxMesh& mesh, std::shared_ptr<SceneObjectMesh> nut_mesh);
		bool CalculateTangant(std::shared_ptr<SceneObjectMesh> nut_mesh);
		void CopyTexture(std::string src, std::string dst, std::string name);
		bool ConvertFbxConstructToSceneNode(fbxsdk::FbxNode* object,std::shared_ptr<BaseSceneNode>& base_node,Scene& scene, fbxsdk::FbxScene* fbx_scene);
		Vector4f FbxToNutVector4f(fbxsdk::FbxDouble3 in)
		{
			Vector4f ret{};
			ret[0] = in[0];
			ret[1] = in[1];
			ret[2] = in[2];
			ret[3] = in[1];
			return ret;
		}
		std::shared_ptr<SceneObjectTransform> GenerateTransform(fbxsdk::FbxNode* p_node);
		unique_ptr<ThreadPool> p_thread_pool_;
		bool bYup_ = true;
	};
}