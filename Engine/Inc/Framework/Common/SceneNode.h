#pragma once
#include "../pch.h"
#include <list>

#include "SceneObject.h"

using std::list;
using std::string;
using std::unique_ptr;

namespace Engine
{
	class BaseSceneNode
	{
	public:
		BaseSceneNode() {};
		BaseSceneNode(const char* name) { name_ = name; };
		BaseSceneNode(const std::string& name) { name_ = name; };
		BaseSceneNode(const std::string&& name) { name_ = std::move(name); };
		virtual ~BaseSceneNode() {};
		void AppendChild(std::shared_ptr<BaseSceneNode>&& sub_node)
		{
			children_.push_back(std::move(sub_node));
		}
		void AppendChild(std::shared_ptr<SceneObjectTransform>&& transform)
		{
			transforms_.push_back(std::move(transform));
		}
		const std::shared_ptr<Matrix4x4f> GetCalculatedTransform() const
		{
			std::shared_ptr<Matrix4x4f> result(new Matrix4x4f());
			BuildIdentityMatrix(*result);

			// TODO: cascading calculation
			for (auto trans : transforms_)
			{
				*result = *result * static_cast<Matrix4x4f>(*trans);
			}

			return result;
		}
	protected:
		string name_;
		list<std::shared_ptr<BaseSceneNode>> children_;
		list<std::shared_ptr<SceneObjectTransform>> transforms_;
	};
	using SceneRootNode = BaseSceneNode;
	template <typename T>
	class SceneNode : public BaseSceneNode 
	{
	public:
		using BaseSceneNode::BaseSceneNode;
		SceneNode() = default;
		void AddSceneObjectRef(const std::string& key) { scene_object_key_ = key; };
		const std::string& GetSceneObjectRef() { return scene_object_key_; };
	protected:
		std::string scene_object_key_;
	};

	class SceneGeometryNode : public SceneNode<SceneObjectGeometry>
	{
	public:
		using SceneNode::SceneNode;
		void SetVisibility(bool visible) { b_visible_ = visible; };
		const bool Visible() { return b_visible_; };
		void SetIfCastShadow(bool shadow) { b_shadow_ = shadow; };
		const bool CastShadow() { return b_shadow_; };
		void SetIfMotionBlur(bool motion_blur) { b_motion_blur_ = motion_blur; };
		const bool MotionBlur() { return b_motion_blur_; };
		using SceneNode::AddSceneObjectRef;
		void AddMaterialRef(const std::string& key) { materials_.push_back(key); };
		void AddMaterialRef(const std::string&& key) { materials_.push_back(key); };
	protected:
		bool        b_visible_;
		bool        b_shadow_;
		bool        b_motion_blur_;
		std::vector<std::string> materials_;
	};

	class SceneLightNode : public SceneNode<SceneObjectLight>
	{
	public:
		using SceneNode::SceneNode;
		void SetIfCastShadow(bool shadow) { b_shadow_ = shadow; };
		const bool CastShadow() { return b_shadow_; };
	protected:
		bool b_shadow_;
	};
	class SceneCameraNode : public SceneNode<SceneObjectCamera>
	{
	public:
		using SceneNode::SceneNode;
		void SetTarget(Vector3f& target) { target_ = target; };
		const Vector3f& GetTarget() { return target_; };
	protected:
		Vector3f target_;
	};
}