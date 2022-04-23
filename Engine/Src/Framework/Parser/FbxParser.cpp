#include "pch.h"
#include <filesystem>
#include "Framework/Parser/FbxParser.h"
#include "Framework/Common/TimerManager.h"
#include "Framework/Common/Log.h"
#include "Physics/NutPhysicsManager.h"

using namespace Engine;

using std::string;
using std::unique_ptr;
using std::make_shared;
using fbxsdk::FbxNodeAttribute;
using fbxsdk::FbxCast;
using fbxsdk::FbxNode;

namespace fs = std::filesystem;


Engine::FbxParser::FbxParser()
{
	fbx_manager_ = FbxManager::Create();
	fbx_ios_ = FbxIOSettings::Create(fbx_manager_, IOSROOT);
	fbx_manager_->SetIOSettings(fbx_ios_);
	fbx_importer_ = FbxImporter::Create(fbx_manager_, "");
	p_thread_pool_ = make_unique<ThreadPool>();
}

Engine::FbxParser::~FbxParser()
{
	fbx_importer_->Destroy();
	fbx_ios_->Destroy();
	fbx_manager_->Destroy();
}

unique_ptr<Scene> Engine::FbxParser::Parse(const std::string & file_path)
{
	g_pTimerManager->Mark();
	FbxScene* fbx_scene = FbxScene::Create(fbx_manager_, "RootScene");
	if (fbx_importer_ != nullptr && !fbx_importer_->Initialize(file_path.c_str(), -1, fbx_manager_->GetIOSettings())) {
		return nullptr;
	}
	fbx_importer_->Import(fbx_scene);
	unique_ptr<Scene> nut_scene(new Scene("Fbx Scene"));
	std::shared_ptr<BaseSceneNode> root_node = make_shared<BaseSceneNode>("scene_root");
	FbxNode* fbx_rt = fbx_scene->GetRootNode();
	//Convert Scene
	fbxsdk::FbxAxisSystem::DirectX.DeepConvertScene(fbx_scene);
	if(fbx_scene->GetGlobalSettings().GetSystemUnit() != fbxsdk::FbxSystemUnit::cm)
	{
		const FbxSystemUnit::ConversionOptions lConversionOptions = {
		false, /* mConvertRrsNodes */
		true, /* mConvertAllLimits */
		true, /* mConvertClusters */
		true, /* mConvertLightIntensity */
		true, /* mConvertPhotometricLProperties */
		true  /* mConvertCameraClipPlanes */
		};
		fbxsdk::FbxSystemUnit::cm.ConvertScene(fbx_scene, lConversionOptions);
	}
	if (fbx_rt != nullptr)
	{
		for (int i = 0; i < fbx_rt->GetChildCount(); i++) {
			ConvertFbxConstructToSceneNode(fbx_rt->GetChild(i), root_node,*nut_scene,fbx_scene);
		}
	}
	NE_LOG(ALL,kWarning,"Parse scene completed,cost {} ms",g_pTimerManager->GetDeltaTime())
	fbx_scene->Destroy();
	return nut_scene;
}

bool Engine::FbxParser::ConvertFbxConstructToSceneNode(fbxsdk::FbxNode* object, std::shared_ptr<BaseSceneNode>& base_node, Scene& scene, fbxsdk::FbxScene* fbx_scene)
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
		scene.CameraNodes.emplace(object_name, _node);
		node = _node;
		const fbxsdk::FbxCamera* camera = FbxCast<fbxsdk::FbxCamera>(attribute);
		_node->AddSceneObjectRef(camera->GetName());
		GenerateCamera(camera,scene);	
		_node->AppendChild(GenerateTransform(object));
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
		std::shared_ptr<SceneObjectGeometry> geometry = make_shared<SceneObjectGeometry>();
		geometry->SetVisibility(_node->Visible());
		geometry->SetIfCastShadow(_node->CastShadow());
		geometry->SetIfMotionBlur(true);
		geometry->SetCollisionType(ESceneObjectCollisionType::kSceneObjectCollisionTypeSphere);
		//temp
		float radius = 1.0f;
		geometry->SetCollisionParam(&radius,1);
		g_pPhysicsManager->CreateRigidBody(*_node.get(),*geometry.get());
		if(GenerateMesh(geometry, _mesh, scene))
		{
			scene.GeometryNodes.emplace(object->GetName(), _node);
			auto object_transf = std::async(&FbxParser::GenerateTransform, this, object);
			_node->AppendChild(object_transf.get());
		}
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
		ConvertFbxConstructToSceneNode(object->GetChild(i),node,scene, fbx_scene);
	}
	return true;
}

std::shared_ptr<SceneObjectTransform> Engine::FbxParser::GenerateTransform(fbxsdk::FbxNode* p_node)
{
	fbxsdk::FbxAMatrix matrixGeo;
	matrixGeo.SetIdentity();
	const auto t = p_node->GetGeometricTranslation(fbxsdk::FbxNode::eSourcePivot);
	const auto r = p_node->GetGeometricRotation(fbxsdk::FbxNode::eSourcePivot);
	const auto s = p_node->GetGeometricScaling(fbxsdk::FbxNode::eSourcePivot);
	matrixGeo.SetT(t);
	matrixGeo.SetR(r);
	matrixGeo.SetS(s);
	FbxAMatrix localMatrix = p_node->EvaluateLocalTransform();
	//Recursively traverse the parent node to get the current world matrix
	//FbxNode* pParentNode = p_node->GetParent();
	//FbxAMatrix parentMatrix = pParentNode->EvaluateLocalTransform();
	//while ((pParentNode = pParentNode->GetParent()) != nullptr) {
	//	parentMatrix = pParentNode->EvaluateLocalTransform() * parentMatrix;
	//}
	//FbxAMatrix matrix = parentMatrix * localMatrix * matrixGeo;
	FbxAMatrix matrix = localMatrix * matrixGeo;
	Matrix4x4f res{};
	for(int32_t i = 0; i < 4; ++i)
	{
		for (int32_t j = 0; j < 4; ++j)
			res[i][j] = localMatrix.Get(i,j);
	}
	//RH y-up ->LH z-up
	//std::swap(res[1][0],res[2][0]);
	//std::swap(res[1][1],res[2][1]);
	//std::swap(res[1][2],res[2][2]);
	//std::swap(res[1][3],res[2][3]);
	//std::swap(res[0][1], res[0][2]);
	//std::swap(res[1][1], res[1][2]);
	//std::swap(res[2][1], res[2][2]);
	//std::swap(res[3][1], res[3][2]);
	return make_shared<SceneObjectTransform>(res);
}


void Engine::FbxParser::GenerateCamera(const fbxsdk::FbxCamera* camera, Scene& scene)
{
	string camera_name = camera->GetName();
	float fov = camera->FieldOfView.Get() * kPi / 360.F;
	float aspect = camera->AspectWidth.Get() / camera->AspectHeight.Get();
	std::shared_ptr<SceneObjectCamera> _camera;
	if(camera->ProjectionType.Get() == fbxsdk::FbxCamera::EProjectionType::eOrthogonal) {
		_camera = make_shared<SceneObjectOrthogonalCamera>();
	}
	else {
		_camera = make_shared<SceneObjectPerspectiveCamera>();
		_camera->SetParam("fov", fov);
	}
	_camera->SetParam("near",static_cast<float>(camera->GetNearPlane()));
	_camera->SetParam("far", static_cast<float>(camera->GetFarPlane()));
	_camera->SetParam("aspect", aspect);
	scene.Cameras[camera_name] = _camera;
}

void Engine::FbxParser::GenerateMaterial(const fbxsdk::FbxSurfaceMaterial* mat, Scene& scene)
{
	string material_name = mat->GetName();
	if (scene.GetMaterial(material_name) != nullptr) return;
	auto material = make_shared<SceneObjectMaterial>();
	material->SetName(material_name);
	const fbxsdk::FbxSurfacePhong* phong = FbxCast<fbxsdk::FbxSurfacePhong>(mat);
	if (phong != nullptr) 
	{
		material->SetColor(SceneObjectMaterial::kSpecular, FbxToNutVector4f(phong->Specular));
		material->SetColor(SceneObjectMaterial::kDiffuse, FbxToNutVector4f(phong->Diffuse));
		material->SetParam(SceneObjectMaterial::kSpecularFactor,static_cast<float>(phong->SpecularFactor));
		for(int32_t i = 0; i < fbxsdk::FbxLayerElement::sTypeTextureCount; ++i)
		{
			auto prop = phong->FindProperty(fbxsdk::FbxLayerElement::sTextureChannelNames[i]);
			if(prop.IsValid())
			{
				int tex_count = prop.GetSrcObjectCount<fbxsdk::FbxTexture>();
				for(int j = 0; j < tex_count; ++j)
				{
					fbxsdk::FbxTexture* texture = prop.GetSrcObject<fbxsdk::FbxTexture>(j);
					if(texture)
					{
						std::string texture_type = prop.GetNameAsCStr();
						fbxsdk::FbxFileTexture* file_texture = fbxsdk::FbxCast<FbxFileTexture>(texture);
						std::string file_name{file_texture->GetMediaName()};
						if(texture_type=="DiffuseColor")
						{
							p_thread_pool_->Enqueue(&FbxParser::CopyTexture,this, std::string(file_texture->GetFileName()), 
								std::string(kAssetTexturePath), file_name);
							material->SetTexture(SceneObjectMaterial::kDiffuse, std::string(kAssetTexturePath).append(file_name));
						}						
						else if(texture_type == "SpecularColor")
						{
							p_thread_pool_->Enqueue(&FbxParser::CopyTexture, this, std::string(file_texture->GetFileName()),
								std::string(kAssetTexturePath), file_name);
							material->SetTexture(SceneObjectMaterial::kSpecular, std::string(kAssetTexturePath).append(file_name));
						}
						else if(texture_type == "NormalMap")
						{
							p_thread_pool_->Enqueue(&FbxParser::CopyTexture, this, std::string(file_texture->GetFileName()),
								std::string(kAssetTexturePath), file_name);
							material->SetTexture(SceneObjectMaterial::kNormalMap, std::string(kAssetTexturePath).append(file_name));
						}
					}
				}
			}
		}
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

bool FbxParser::GenerateMesh(std::shared_ptr<SceneObjectGeometry> geo, fbxsdk::FbxMesh* mesh, Scene& scene)
{
	std::shared_ptr<SceneObjectMesh> nut_mesh(new SceneObjectMesh());
	if(!mesh->IsTriangleMesh()) {
		fbxsdk::FbxGeometryConverter convert(fbx_manager_);
		mesh = FbxCast<fbxsdk::FbxMesh>(convert.Triangulate(mesh, true));
	}
	//The vertex must be added to the MeshObject's vector before the normal, 
	//because the input layout is passed in the vertex-normal order
	//not thread safe now 
	//auto ret1 = p_thread_pool_->Enqueue(&FbxParser::ReadVertex,this,std::ref(*mesh),nut_mesh);
	//auto ret2 = p_thread_pool_->Enqueue(&FbxParser::ReadNormal, this, std::ref(*mesh), nut_mesh);
	//auto ret3 = p_thread_pool_->Enqueue(&FbxParser::ReadUVs, this, std::ref(*mesh), nut_mesh);

	auto ret1 = ReadVertex(*mesh,nut_mesh);
	auto ret2 = ReadNormal(*mesh,nut_mesh);
	auto ret3 = ReadUVs(*mesh,nut_mesh);
	if(ret1&& ret2&& ret3)
	{
		geo->AddMesh(nut_mesh);
		scene.Geometries[mesh->GetName()] = geo;
		return true;
	}
	return false;
}

bool Engine::FbxParser::ReadNormal(const fbxsdk::FbxMesh& mesh, std::shared_ptr<SceneObjectMesh> nut_mesh)
{
	if (mesh.GetElementNormalCount() < 1) return false;
	auto* normals = mesh.GetElementNormal(0);
	int vertex_count = mesh.GetControlPointsCount(), data_size = 0;
	void* data = nullptr;
	std::vector<Vector3f> temp_v;
	if (normals->GetMappingMode() == fbxsdk::FbxLayerElement::EMappingMode::eByControlPoint)
	{
		data = new float[vertex_count * 3];
		for (int i = 0; i < vertex_count; ++i)
		{
			int normal_index = 0;
			if (normals->GetReferenceMode() == fbxsdk::FbxLayerElement::EReferenceMode::eDirect)
				normal_index = i;
			else if (normals->GetReferenceMode() == fbxsdk::FbxLayerElement::EReferenceMode::eIndexToDirect)
				normal_index = normals->GetIndexArray().GetAt(i);
			auto normal = normals->GetDirectArray().GetAt(normal_index);
			Vector3f n{ (float)normal[0],(float)normal[1],(float)normal[2] };
			reinterpret_cast<float*>(data)[i * 3] = normals->GetDirectArray().GetAt(normal_index)[0];
			reinterpret_cast<float*>(data)[i * 3 + 1] = normals->GetDirectArray().GetAt(normal_index)[1];
			reinterpret_cast<float*>(data)[i * 3 + 2] = normals->GetDirectArray().GetAt(normal_index)[2];
			temp_v.push_back(n);
		}
	}
	else if (normals->GetMappingMode() == fbxsdk::FbxLayerElement::EMappingMode::eByPolygonVertex)
	{
		int trangle_count = mesh.GetPolygonCount();
		vertex_count = trangle_count * 3;
		data = new float[vertex_count * 3];
		int cur_vertex_id = 0;
		for (int i = 0; i < trangle_count; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				int normal_index = 0;
				if (normals->GetReferenceMode() == fbxsdk::FbxLayerElement::EReferenceMode::eDirect)
					normal_index = cur_vertex_id;
				else if (normals->GetReferenceMode() == fbxsdk::FbxLayerElement::EReferenceMode::eIndexToDirect)
					normal_index = normals->GetIndexArray().GetAt(cur_vertex_id);
				auto normal = normals->GetDirectArray().GetAt(normal_index);
				Vector3f n{ (float)normal[0],(float)normal[1],(float)normal[2] };
				reinterpret_cast<float*>(data)[cur_vertex_id * 3] = normals->GetDirectArray().GetAt(normal_index)[0];
				reinterpret_cast<float*>(data)[cur_vertex_id * 3 + 1] = normals->GetDirectArray().GetAt(normal_index)[1];
				reinterpret_cast<float*>(data)[cur_vertex_id * 3 + 2] = normals->GetDirectArray().GetAt(normal_index)[2];
				temp_v.push_back(n);
				++cur_vertex_id;
			}
		}
	}
	SceneObjectVertexArray& _v_array = *new SceneObjectVertexArray(EVertexArrayType::kNormal, 0u, EVertexDataType::kVertexDataFloat3,
		data, vertex_count);
	if(vertex_count == 0)
	{
		NE_LOG(ALL, kWarning, "{}'s normal with 0 vertex", mesh.GetName())
		return false;
	}
	nut_mesh->AddVertexArray(std::move(_v_array));
	return true;
}

bool Engine::FbxParser::ReadVertex(const fbxsdk::FbxMesh& mesh, std::shared_ptr<SceneObjectMesh> nut_mesh)
{
	int32_t trangle_count = mesh.GetPolygonCount();
	int32_t vertex_count = trangle_count * 3;
	//TODO:Here the vertex and index buffers are not freed and can cause memory leaks
	void* vertex_buf = new float[vertex_count * 3];
	fbxsdk::FbxVector4* points = mesh.GetControlPoints();
	int32_t point_index = 0, cur_index_count = 0;
	for (int32_t i = 0; i < trangle_count; ++i)
	{
		for (int32_t j = 0; j < 3; ++j)
		{
			point_index = mesh.GetPolygonVertex(i, j);
			reinterpret_cast<float*>(vertex_buf)[cur_index_count * 3] = points[point_index].mData[0];
			reinterpret_cast<float*>(vertex_buf)[cur_index_count * 3 + 1] = points[point_index].mData[1];
			reinterpret_cast<float*>(vertex_buf)[cur_index_count * 3 + 2] = points[point_index].mData[2];
			++cur_index_count;
		}
	}
	EVertexDataType vertex_type = EVertexDataType::kVertexDataFloat3;
	SceneObjectVertexArray& _v_array = *new SceneObjectVertexArray(EVertexArrayType::kVertex, 0u, vertex_type, vertex_buf, vertex_count);
	if (vertex_count == 0)
	{
		NE_LOG(ALL, kWarning, "{}'s vertex with 0 vertex", mesh.GetName())
		return false;
	}
	nut_mesh->AddVertexArray(std::move(_v_array));
	return true;
}

bool Engine::FbxParser::ReadUVs(const fbxsdk::FbxMesh& mesh, std::shared_ptr<SceneObjectMesh> nut_mesh)
{
	//get all UV set names
	fbxsdk::FbxStringList name_list;
	mesh.GetUVSetNames(name_list);
	for(int i = 0; i < name_list.GetCount(); ++i)
	{
		//get lUVSetIndex-th uv set
		const char* uv_name = name_list.GetStringAt(i);
		const FbxGeometryElementUV* uv = mesh.GetElementUV(uv_name);
		if(!uv) continue;
		//index array, where holds the index referenced to the uv data
		const bool lUseIndex = uv->GetReferenceMode() != FbxGeometryElement::eDirect;
		const int lIndexCount = (lUseIndex) ? uv->GetIndexArray().GetCount() : 0;
		//iterating through the data by polygon
		const int trangle_count = mesh.GetPolygonCount();
		float* data = new float[trangle_count * 6];
		if (uv->GetMappingMode() == FbxGeometryElement::eByControlPoint)
		{
			int cur_vertex_id = 0;
			for (int lPolyIndex = 0; lPolyIndex < trangle_count; ++lPolyIndex)
			{
				// build the max index array that we need to pass into MakePoly
				for (int lVertIndex = 0; lVertIndex < 3; ++lVertIndex)
				{
					//get the index of the current vertex in control points array
					int lPolyVertIndex = mesh.GetPolygonVertex(lPolyIndex, lVertIndex);
					//the UV index depends on the reference mode
					int lUVIndex = lUseIndex ? uv->GetIndexArray().GetAt(lPolyVertIndex) : lPolyVertIndex;
					FbxVector2 uvs = uv->GetDirectArray().GetAt(lUVIndex);
					reinterpret_cast<float*>(data)[cur_vertex_id * 2] = uv->GetDirectArray().GetAt(lUVIndex)[0];
					reinterpret_cast<float*>(data)[cur_vertex_id * 2 + 1] = 1.f - uv->GetDirectArray().GetAt(lUVIndex)[1];
					++cur_vertex_id;
				}
			}
		}
		else if (uv->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
		{
			int lPolyIndexCounter = 0;
			for (int lPolyIndex = 0; lPolyIndex < trangle_count; ++lPolyIndex)
			{
				for (int lVertIndex = 0; lVertIndex < 3; ++lVertIndex)
				{
					//the UV index depends on the reference mode
					int lUVIndex = lUseIndex ? uv->GetIndexArray().GetAt(lPolyIndexCounter) : lPolyIndexCounter;
					FbxVector2 uvs = uv->GetDirectArray().GetAt(lUVIndex);
					reinterpret_cast<float*>(data)[lPolyIndexCounter * 2] = uv->GetDirectArray().GetAt(lUVIndex)[0];
					reinterpret_cast<float*>(data)[lPolyIndexCounter * 2 + 1] = 1.f - uv->GetDirectArray().GetAt(lUVIndex)[1];
					//NE_LOG(ALL,kWarning,"U:{}V:{}",uvs[0],uvs[1])
					lPolyIndexCounter++;
				}
			}
		}
		SceneObjectVertexArray& _u_array = *new SceneObjectVertexArray(EVertexArrayType::kUVs,0u,EVertexDataType::kVertexDataFloat2,data,trangle_count*3);
		if (trangle_count == 0)
		{
			NE_LOG(ALL, kWarning, "{}'s uv with 0 vertex", mesh.GetName())
				return false;
		}
		nut_mesh->AddVertexArray(std::move(_u_array));
	}
	return true;
}

void Engine::FbxParser::CopyTexture(std::string src, std::string dst, std::string name)
{
	fs::path src_path(src);
	if (!fs::exists(src_path))
	{
		NE_LOG(ALL, kError, "source texture is missing,the path is {}", src)
			return;
	}
	fs::path dst_path(dst);
	if(!fs::exists(dst_path))
	{
		fs::create_directory(dst_path);
	}
	dst_path.append(name);
	if(fs::exists(dst_path))
		NE_LOG(ALL, kWarning, "{} already exist,origin content will be overwrite", dst_path.string())
	if(!fs::copy_file(src_path, dst_path,fs::copy_options::overwrite_existing))
		NE_LOG(ALL, kError, "texture form {} copy failed", src)
	else
		NE_LOG(ALL, kNormal, "texture {} copy completed", name)
}
