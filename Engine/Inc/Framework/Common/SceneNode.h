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
		Vector3f GetWorldPosition()
		{
			auto transf = *GetCalculatedTransform().get();
			position_.x = transf[3][0];
			position_.y = transf[3][1];
			position_.z = transf[3][2];
			return position_;
		}
	protected:
		string name_;
		list<std::shared_ptr<BaseSceneNode>> children_;
		list<std::shared_ptr<SceneObjectTransform>> transforms_;
		Vector3f position_;
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
		std::string GetMaterialRef(const size_t index)
		{
			if(index < materials_.size()) return materials_[index];
			else return std::string{};
		}
		void LinkRigidBody(void* rigidBody)
		{
			p_rigid_body_ = rigidBody;
		}

		void* UnlinkRigidBody()
		{
			void* rigidBody = p_rigid_body_;
			p_rigid_body_ = nullptr;
			return rigidBody;
		}
		void* RigidBody() { return p_rigid_body_; }
	protected:
		bool        b_visible_;
		bool        b_shadow_;
		bool        b_motion_blur_;
		std::vector<std::string> materials_;
		void*		p_rigid_body_ = nullptr;
	};

	class SceneLightNode : public SceneNode<SceneObjectLight>
	{
	public:
		using SceneNode::SceneNode;
		void SetIfCastShadow(bool shadow) { b_shadow_ = shadow; };
		const bool CastShadow() { return b_shadow_; };
		Vector3f GetForwardDir() const
		{
			Vector3f down{ 0.f,1.f,0.f };
			TransformNormal(down, *GetCalculatedTransform().get());
			return down;
		}
	protected:
		bool b_shadow_;
	};
	class SceneCameraNode : public SceneNode<SceneObjectCamera>
	{
	public:
		using SceneNode::SceneNode;
		void SetTarget(Vector3f target) { target_ = target; };
		void SetPosition(Vector3f position) { position_ = position; };
		void SetUp(Vector3f up) { up_ = up; };

		const Vector3f& GetTarget() { return target_; };
		const Vector3f& GetPosition() { return position_; };
		const Vector3f& GetUp() { return up_; };
		const Vector3f& GetRight() { return right_; };
		const Vector3f& GetForward() { return forawrd_; };

	protected:
		Vector3f target_;
		Vector3f position_;
		Vector3f up_;
		Vector3f right_;
		Vector3f forawrd_;

	};
}