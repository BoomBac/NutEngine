#pragma once
#include <unordered_map>

#include "../pch.h"
#include "../External/OpenFbx/ofbx.h"
#include "../Inc/Framework/Common/AssetLoader.h"
#include "../Inc/Framework/Interface/ISceneParser.h"

using std::string;


namespace Engine
{
	extern AssetLoader* g_pAssetLoader;
	class FbxParser : public ISceneParser
	{
	public:
		FbxParser() = default;
		virtual ~FbxParser() = default;
		std::unique_ptr<BaseSceneNode> Parse(const std::string& buf) override
		{
			auto ptr = g_pAssetLoader->OpenAndReadBinarySync(buf.c_str());
			ofbx::IScene* scene = ofbx::load((ofbx::u8*)ptr.p_data_, ptr.size_, (ofbx::u64)ofbx::LoadFlags::TRIANGULATE);
			std::unique_ptr<BaseSceneNode> root_node(new BaseSceneNode("scene_root"));
			for (int i = 0; i < scene->getAllObjectCount(); i++)
			{
				auto p = scene->getAllObjects()[i];
				ConvertFbxConstructToSceneNode(p,root_node);
			}
			return root_node;
		}
	private:
		std::unordered_map<string, std::shared_ptr<BaseSceneObject>> scene_objects_;
	private:
		void ConvertFbxConstructToSceneNode(const ofbx::Object* object)
		{
			
			int i = 0;
			while (ofbx::Object* child = object->resolveObjectLink(i))
			{	
				if (!object->isNode()) break;
				std::cout << "child name:" << child->name << std::endl;
				ConvertFbxConstructToSceneNode(child);
				++i;
			}
			std::cout << object->name <<" has " <<i<<" children¡£"<< std::endl;
		}
		void ConvertFbxConstructToSceneNode(const ofbx::Object* object,std::unique_ptr<BaseSceneNode>& base_node)
		{
			std::unique_ptr<BaseSceneNode> node;
			string object_name = object->name;
			switch (object->getType())
			{
				case ofbx::Object::Type::ROOT:
					node = std::make_unique<SceneRootNode>(object_name);
				break;
				case ofbx::Object::Type::GEOMETRY:
				{
					auto geo = dynamic_cast<ofbx::Geometry*>(const_cast<ofbx::Object*>(object));
					std::shared_ptr<SceneObjectGeometry> _object;
					if (!scene_objects_[object_name])
						scene_objects_[object_name] = std::make_shared<SceneObjectGeometry>();
					_object = std::dynamic_pointer_cast<SceneObjectGeometry>(scene_objects_[object_name]);
					_object->SetIfCastShadow(true);
					_object->SetIfMotionBlur(true);
					_object->SetVisibility(true);
					std::shared_ptr<SceneObjectMesh> mesh(new SceneObjectMesh());
					mesh->SetPrimitiveType(EPrimitiveType::kPrimitiveTypeTriList);
					SceneObjectVertexArray& _v_array = *new SceneObjectVertexArray("vertex_arr", 0u, EVertexDataType::kVertexDataFloat3, 
					geo->getVertices(), geo->getVertexCount());
					SceneObjectIndexArray& _i_array = *new SceneObjectIndexArray(0u,0u,EIndexDataType::kIndexData32i,geo->getFaceIndices(),geo->getIndexCount());
					mesh->AddVertexArray(std::move(_v_array));
					mesh->AddIndexArray(std::move(_i_array));
					_object->AddMesh(mesh);
				}
				break;
				case ofbx::Object::Type::MESH:
				{
					node = std::make_unique<SceneGeometryNode>(object_name);
					SceneGeometryNode& _node = dynamic_cast<SceneGeometryNode&>(*node);
					_node.SetIfCastShadow(true);
					_node.SetIfMotionBlur(true);
					_node.SetVisibility(true);
					if (!scene_objects_[object_name])
						scene_objects_[object_name] = std::make_shared<SceneObjectGeometry>();
					else break;
					_node.AddSceneObjectRef(std::dynamic_pointer_cast<SceneObjectGeometry>(scene_objects_[object_name]));
					auto mesh = dynamic_cast<ofbx::Mesh*>(const_cast<ofbx::Object*>(object));
					for (int32_t i = 0; i < mesh->getMaterialCount(); i++)
					{
						object_name = mesh->getMaterial(i)->name;
						if(!scene_objects_[object_name])
							scene_objects_[object_name] = std::make_shared<SceneObjectMaterial>();
						_node.AddSceneObjectRef(std::dynamic_pointer_cast<SceneObjectMaterial>(scene_objects_[object_name]));
					}
				}
				break;
				case ofbx::Object::Type::MATERIAL:
				{
					auto _mat = dynamic_cast<ofbx::Material*>(const_cast<ofbx::Object*>(object));
					if (!scene_objects_[object_name])
						scene_objects_[object_name] = std::make_shared<SceneObjectMaterial>();
					else break;
					std::shared_ptr<SceneObjectMaterial> material = std::dynamic_pointer_cast<SceneObjectMaterial>(scene_objects_[object_name]);
					material->SetName(object_name);
					Vector4f bc;
					bc.r = _mat->getDiffuseColor().r;
					bc.g = _mat->getDiffuseColor().g;
					bc.b = _mat->getDiffuseColor().b;
					bc.a = 1.f;
					material->SetColor("diffuse", bc);
					//char file_name[256];
					//if (const ofbx::Texture* texture = _mat->getTexture(ofbx::Texture::TextureType::DIFFUSE))
					//	texture->getFileName().toString(file_name);				
				}
				break;
			}
			int i = 0;
			if (node != nullptr)
			{
				while (ofbx::Object* child = object->resolveObjectLink(i))
				{
					if (!child->isNode())
					{
						++i; continue;
					}continue;
					ConvertFbxConstructToSceneNode(child, node);
					++i;
				}
				base_node->AppendChild(std::move(node));
			}

		}

	};
}