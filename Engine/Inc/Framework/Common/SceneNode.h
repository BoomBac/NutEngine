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
		void AppendChild(std::unique_ptr<BaseSceneNode>&& sub_node)
		{
			children_.push_back(std::move(sub_node));
		}
		void AppendChild(std::unique_ptr<SceneObjectTransform>&& transform)
		{
			transforms_.push_back(std::move(transform));
		}
	protected:
		string name_;
		list<unique_ptr<BaseSceneNode>> children_;
		list<unique_ptr<SceneObjectTransform>> transforms_;
	};
	using SceneRootNode = BaseSceneNode;
	template <typename T>
	class SceneNode : public BaseSceneNode 
	{
	public:
		using BaseSceneNode::BaseSceneNode;
		SceneNode() = default;
		SceneNode(const std::shared_ptr<T>& object) { p_scene_object_ = object; };
		SceneNode(const std::shared_ptr<T>&& object) { p_scene_object_ = std::move(object); };
		void AddSceneObjectRef(const std::shared_ptr<T>& object) { p_scene_object_ = object; };
		void AddSceneObjectRef(const std::shared_ptr<T>&& object) { p_scene_object_ = std::move(object); };
	protected:
		std::shared_ptr<T> p_scene_object_;
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
		void AddSceneObjectRef(const std::shared_ptr<SceneObjectMaterial>& object) { materials_.push_back(object); };
	protected:
		bool        b_visible_;
		bool        b_shadow_;
		bool        b_motion_blur_;
		std::vector<std::shared_ptr<SceneObjectMaterial>> materials_;
	};

	class SceneLightNode : public SceneNode<SceneObjectLight>
	{
	public:
		using SceneNode::SceneNode;
		void SetTarget(Vector3f& target) { target_ = target; };
		const Vector3f& GetTarget() { return target_; };
	protected:
		Vector3f target_;
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