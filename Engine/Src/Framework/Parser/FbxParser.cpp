#include "pch.h"
#include "Framework/Parser/FbxParser.h"

using namespace Engine;

using std::string;
using std::unique_ptr;
using std::make_shared;
using fbxsdk::FbxNodeAttribute;
using fbxsdk::FbxCast;
using fbxsdk::FbxNode;


Engine::FbxParser::FbxParser()
{
	fbx_manager_ = FbxManager::Create();
	fbx_ios_ = FbxIOSettings::Create(fbx_manager_, IOSROOT);
	fbx_manager_->SetIOSettings(fbx_ios_);
	fbx_importer_ = FbxImporter::Create(fbx_manager_, "");
}

Engine::FbxParser::~FbxParser()
{
	fbx_importer_->Destroy();
	fbx_ios_->Destroy();
	fbx_manager_->Destroy();
}

unique_ptr<Scene> Engine::FbxParser::Parse(const std::string& buf)
{
	auto abs_path = g_pAssetLoader->GetAbsolutePath((buf.c_str()));

	FbxScene* fbx_scene = FbxScene::Create(fbx_manager_, "RootScene");
	if (fbx_importer_ != nullptr && !fbx_importer_->Initialize(abs_path.c_str(), -1, fbx_manager_->GetIOSettings())) {
		return nullptr;
	}
	fbx_importer_->Import(fbx_scene);
	//printf("Call to FbxImporter::Initialize() failed.\n");
	//printf("Error returned: %s\n\n", importer->GetStatus().GetErrorString());
	unique_ptr<Scene> nut_scene(new Scene("Fbx Scene"));
	std::shared_ptr<BaseSceneNode> root_node = make_shared<BaseSceneNode>("scene_root");
	FbxNode* fbx_rt = fbx_scene->GetRootNode();
	if (fbx_rt != nullptr)
	{
		for (int i = 0; i < fbx_rt->GetChildCount(); i++) {
			ConvertFbxConstructToSceneNode(fbx_rt->GetChild(i), root_node,*nut_scene);
		}
	}
	fbx_scene->Destroy();
	return nut_scene;
}

void Engine::FbxParser::ConvertFbxConstructToSceneNode(fbxsdk::FbxNode* object, std::shared_ptr<BaseSceneNode>& base_node, Scene& scene)
{
	std::shared_ptr<BaseSceneNode> node;
	string object_name = object->GetName();
	fbxsdk::FbxNodeAttribute* attribute = object->GetNodeAttribute();
	switch (attribute->GetAttributeType())
	{
	case FbxNodeAttribute::EType::eLight:
	{
		auto _node = std::make_shared<SceneLightNode>(object_name);
		const fbxsdk::FbxLight* _light = FbxCast<fbxsdk::FbxLight>(attribute);
		_node->SetIfCastShadow(_light->CastShadows.Get());
		_node->AddSceneObjectRef(object_name);
		scene.LightNodes.emplace(object_name, _node);
		node = _node;
		GenerateLight(_light, scene);
	}
	break;
	case FbxNodeAttribute::EType::eCamera:
	{
		auto _node = std::make_shared<SceneCameraNode>(object_name);
		const fbxsdk::FbxCamera* _camera = FbxCast<fbxsdk::FbxCamera>(attribute);
		_node->AddSceneObjectRef(object_name);
		scene.CameraNodes.emplace(object_name, _node);
		node = _node;
	}
	break;
	case FbxNodeAttribute::EType::eMesh:
	{
		auto _node = std::make_shared<SceneGeometryNode>(object_name);
		fbxsdk::FbxMesh* _mesh = FbxCast<fbxsdk::FbxMesh>(attribute);
		_node->SetVisibility(_mesh->PrimaryVisibility.Get());
		_node->SetIfMotionBlur(true);
		_node->SetIfCastShadow(_mesh->CastShadow.Get());
		_node->AddSceneObjectRef(_mesh->GetName());
		for (int32_t i = 0; i < object->GetMaterialCount(); i++)
		{
			auto mat = object->GetMaterial(i);
			object_name = mat->GetName();
			_node->AddMaterialRef(object_name);
			GenerateMaterial(mat, scene);
		}
		object_name = object->GetName();
		scene.GeometryNodes.emplace(object_name, _node);
		std::shared_ptr<SceneObjectGeometry> geometry = make_shared<SceneObjectGeometry>();
		geometry->SetVisibility(_node->Visible());
		geometry->SetIfCastShadow(_node->CastShadow());
		geometry->SetIfMotionBlur(true);
		GenerateMesh(geometry, _mesh, scene);
	}
	break;
	case FbxNodeAttribute::EType::eNurbs: {}break;
	case FbxNodeAttribute::EType::ePatch: {} break;
	case FbxNodeAttribute::EType::eLine: {} break;
	default:
		break;
	}
	base_node->AppendChild(std::move(node));
	for(int32_t i = 0; i < object->GetChildCount(); i++) {
		ConvertFbxConstructToSceneNode(object->GetChild(i),node,scene);
	}
}

void Engine::FbxParser::GenerateMaterial(const fbxsdk::FbxSurfaceMaterial* mat, Scene& scene)
{
	string material_name = mat->GetName();
	if (scene.GetMaterial(material_name) != nullptr) return;
	auto material = make_shared<SceneObjectMaterial>();
	material->SetName(material_name);
	const fbxsdk::FbxSurfacePhong* phong = FbxCast<fbxsdk::FbxSurfacePhong>(mat);
	if (phong != nullptr) {
		//phong->Diffuse.Get().Buffer
	}
	scene.Materials[material_name] = material;
}

void Engine::FbxParser::GenerateLight(const fbxsdk::FbxLight* light, Scene& scene)
{
	std::shared_ptr<SceneObjectLight> _light;
	switch (light->LightType)
	{
	case fbxsdk::FbxLight::EType::ePoint:
	{
		_light = make_shared<SceneObjectPointLight>();
	}
	break;
	case fbxsdk::FbxLight::EType::eSpot:
	{
		_light = make_shared<SceneObjectSpotLight>();
	}
	break;
	//TODO
	case fbxsdk::FbxLight::EType::eArea: break;
	case fbxsdk::FbxLight::EType::eDirectional: break;
	case fbxsdk::FbxLight::EType::eVolume: break;
	default:
		break;
	}
	_light->SetIfCastShadow(light->CastShadows.Get());
	string lcolor = "color";
	Vector4f temp{};
	temp.r = light->Color.Get().mData[0];
	temp.g = light->Color.Get().mData[1];
	temp.b = light->Color.Get().mData[2];
	temp.a = 1.f;
	_light->SetColor(lcolor, temp);
	lcolor = "intensity";
	_light->SetParam(lcolor, static_cast<float>(light->Intensity.Get()));
	scene.Lights[light->GetName()] = _light;
}

void FbxParser::GenerateMesh(std::shared_ptr<SceneObjectGeometry> geo, fbxsdk::FbxMesh* mesh, Scene& scene)
{
	std::shared_ptr<SceneObjectMesh> nut_mesh(new SceneObjectMesh());
	if(mesh->IsTriangleMesh()) {
		fbxsdk::FbxGeometryConverter convert(fbx_manager_);
		mesh = FbxCast<fbxsdk::FbxMesh>(convert.Triangulate(mesh, true));
	}
	size_t element_count = mesh->GetControlPointsCount();
	size_t buffer_size = sizeof(double) * element_count * 4;
	void* data = new double[element_count * 4];
	bool b_reuse = false;
	memcpy(data, mesh->GetControlPoints()->mData,buffer_size);
	EVertexDataType vertex_type = EVertexDataType::kVertexDataDouble4;
	SceneObjectVertexArray& _v_array = *new SceneObjectVertexArray("", 0u, vertex_type, data, element_count);
	nut_mesh->AddVertexArray(std::move(_v_array));
	element_count = mesh->GetPolygonVertexCount();
	void* _data = nullptr;
	if(buffer_size >= sizeof(int32_t) * element_count) {
		b_reuse = true;
		_data = new(data) int32_t;
	}
	else {
		_data = new int32_t[element_count];	
		delete[] data;
	}
	buffer_size = sizeof(int32_t) * element_count;
	EIndexDataType index_type = EIndexDataType::kIndexData32i;
	SceneObjectIndexArray& _i_array = *new SceneObjectIndexArray(0,0,index_type,_data,element_count);
	memcpy(_data,mesh->GetPolygonVertices(),buffer_size);
	nut_mesh->AddIndexArray(std::move(_i_array));
	if(b_reuse) delete[] data;
	geo->AddMesh(nut_mesh);
	scene.Geometries[mesh->GetName()] = geo;
}