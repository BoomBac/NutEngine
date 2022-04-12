#pragma once
#include "Image.h"
#include "../Math/Guid.h"

namespace Engine
{
	enum class ESceneObjectType : uint8_t
	{
        kSceneObjectTypeMesh = 0u,
        kSceneObjectTypeMaterial = 1u,
        kSceneObjectTypeTexture = 2u,
        kSceneObjectTypeLight = 3u,
        kSceneObjectTypeCamera = 4u,
        kSceneObjectTypeAnimator = 5u,
        kSceneObjectTypeClip = 6u,
        kSceneObjectTypeVertexArray,
        kSceneObjectTypeIndexArray,
        kSceneObjectTypeGeometry
	};
    enum class EVertexDataType : uint8_t
    {
        kVertexDataFloat1 = 0u,
        kVertexDataFloat2,
        kVertexDataFloat3,
        kVertexDataFloat4,
        kVertexDataDouble1,
        kVertexDataDouble2,
        kVertexDataDouble3,
        kVertexDataDouble4,
    };
    enum class EIndexDataType : uint8_t
    {
        kIndexData8i = 1u,
        kIndexData16i,
        kIndexData32i,
        kIndexData64i,
    };
    enum class EPrimitiveType : int32_t {
        kPrimitiveTypeNone,        ///< No particular primitive type.
        kPrimitiveTypePointList,   ///< For N>=0, vertex N renders a point.
        kPrimitiveTypeLineList ,    ///< For N>=0, vertices [N*2+0, N*2+1] render a line.
        kPrimitiveTypeLineStrip,   ///< For N>=0, vertices [N, N+1] render a line.
        kPrimitiveTypeTriList,     ///< For N>=0, vertices [N*3+0, N*3+1, N*3+2] render a triangle.
        kPrimitiveTypeTriFan,      ///< For N>=0, vertices [0, (N+1)%M, (N+2)%M] render a triangle, where M is the vertex count.
        kPrimitiveTypeTriStrip,    ///< For N>=0, vertices [N*2+0, N*2+1, N*2+2] and [N*2+2, N*2+1, N*2+3] render triangles.
        kPrimitiveTypePatch,       ///< Used for tessellation.
        kPrimitiveTypeLineListAdjacency,       ///< For N>=0, vertices [N*4..N*4+3] render a line from [1, 2]. Lines [0, 1] and [2, 3] are adjacent to the rendered line.
        kPrimitiveTypeLineStripAdjacency,      ///< For N>=0, vertices [N+1, N+2] render a line. Lines [N, N+1] and [N+2, N+3] are adjacent to the rendered line.
        kPrimitiveTypeTriListAdjacency,    ///< For N>=0, vertices [N*6..N*6+5] render a triangle from [0, 2, 4]. Triangles [0, 1, 2] [4, 2, 3] and [5, 0, 4] are adjacent to the rendered triangle.
        kPrimitiveTypeTriStripAdjacency,      ///< For N>=0, vertices [N*4..N*4+6] render a triangle from [0, 2, 4] and [4, 2, 6]. Odd vertices Nodd form adjacent triangles with indices min(Nodd+1,Nlast) and max(Nodd-3,Nfirst).
        kPrimitiveTypeRectList,  ///< For N>=0, vertices [N*3+0, N*3+1, N*3+2] render a screen-aligned rectangle. 0 is upper-left, 1 is upper-right, and 2 is the lower-left corner.
        kPrimitiveTypeLineLoop,    ///< Like <c>kPrimitiveTypeLineStrip</c>, but the first and last vertices also render a line.
        kPrimitiveTypeQuadList,    ///< For N>=0, vertices [N*4+0, N*4+1, N*4+2] and [N*4+0, N*4+2, N*4+3] render triangles.
        kPrimitiveTypeQuadStrip,   ///< For N>=0, vertices [N*2+0, N*2+1, N*2+3] and [N*2+0, N*2+3, N*2+2] render triangles.
        kPrimitiveTypePolygon     ///< For N>=0, vertices [0, N+1, N+2] render a triangle.
    };

    class BaseSceneObject
    {
    protected:
        BaseSceneObject(ESceneObjectType type) : type_(type) { guid_ = NewGuid(); };
        BaseSceneObject(Guid& guid, ESceneObjectType type) : guid_(guid), type_(type) {};
        BaseSceneObject(Guid&& guid, ESceneObjectType type) : guid_(std::move(guid)), type_(type) {};
        BaseSceneObject(BaseSceneObject&& obj) : guid_(std::move(obj.guid_)), type_(obj.type_) {};
        BaseSceneObject& operator=(BaseSceneObject&& obj) { this->guid_ = std::move(obj.guid_); this->type_ = obj.type_; return *this; };
        virtual ~BaseSceneObject() {};
    private:
        BaseSceneObject() = delete;
        BaseSceneObject(BaseSceneObject& obj) = delete;
        BaseSceneObject& operator=(BaseSceneObject& obj) = delete;
    protected:
        Guid guid_;
        ESceneObjectType type_;
    public:
        const Guid& GetGuid() const 
        {
            return guid_;
        }
    };
    class SceneObjectVertexArray
    {
    public:
        SceneObjectVertexArray(const char* attr = "", const uint32_t morph_index = 0, const EVertexDataType data_type = EVertexDataType::kVertexDataFloat3, 
            const void* data = nullptr, const size_t data_size = 0) :
            attribute_(attr), morph_target_index_(morph_index), data_type_(data_type),p_data_(data), size_(data_size) {};
        SceneObjectVertexArray(SceneObjectVertexArray& arr) = default;
        SceneObjectVertexArray(SceneObjectVertexArray && arr) = default;
        size_t GetDataSize() const
        {
            size_t size = size_;
            switch (data_type_) {
            case EVertexDataType::kVertexDataFloat1:
            case EVertexDataType::kVertexDataFloat2:
            case EVertexDataType::kVertexDataFloat3:
                size *= sizeof(float) * 3;
                break;
            case EVertexDataType::kVertexDataFloat4:
                size *= sizeof(float) * 4;
                break;
            case EVertexDataType::kVertexDataDouble1:
            case EVertexDataType::kVertexDataDouble2:
            case EVertexDataType::kVertexDataDouble3:
                size *= sizeof(float) * 3;
                break;
            case EVertexDataType::kVertexDataDouble4:
                size *= sizeof(double) * 4;
                break;
            default:
                size = 0;
                assert(0);
                break;
            }
            return size;
        };
        const void* GetData() const { return p_data_; }
        size_t GetVertexCount() const
        {
            return size_;
        }
    protected:
        const std::string attribute_;
        const uint32_t    morph_target_index_;
        const EVertexDataType data_type_;
        const void* p_data_;
        //represents the number of vertices, a vertex may consist of 3-4 float numbers
        size_t      size_;
    };
    class SceneObjectIndexArray
    {
    public:
        SceneObjectIndexArray(const uint32_t material_index = 0, const size_t restart_index = 0, const EIndexDataType data_type = EIndexDataType::kIndexData16i,
            const void* data = nullptr, const size_t data_size = 0)
            : material_index_(material_index), restart_index_(restart_index), data_type_(data_type), p_data_(data), size_(data_size) {};
        SceneObjectIndexArray(SceneObjectIndexArray& arr) = default;
        SceneObjectIndexArray(SceneObjectIndexArray && arr) = default;
        ~SceneObjectIndexArray()
        {
            
        }
        const uint32_t GetMaterialIndex() const { return material_index_; };
        const EIndexDataType GetIndexType() const { return data_type_; };
        const void* GetData() const { return p_data_; };
        size_t GetDataSize() const
        {
            size_t size = size_;
            switch (data_type_) {
            case EIndexDataType::kIndexData8i:
                size *= sizeof(int8_t);
                break;
            case EIndexDataType::kIndexData16i:
                size *= sizeof(int16_t);
                break;
            case EIndexDataType::kIndexData32i:
                size *= sizeof(int32_t);
                break;
            case EIndexDataType::kIndexData64i:
                size *= sizeof(int64_t);
                break;
            default:
                size = 0;
                assert(0);
                break;
            }

            return size;
        };

        size_t GetIndexCount() const
        {
            return size_;
        }
    protected:
        const uint32_t    material_index_;
        const size_t      restart_index_;
        const EIndexDataType data_type_;
        const void* p_data_;
        const size_t size_;
    };
    class SceneObjectMesh : public BaseSceneObject
    {
    public:
        SceneObjectMesh(bool visible = true, bool shadow = true, bool motion_blur = true) : BaseSceneObject(ESceneObjectType::kSceneObjectTypeMesh), 
            b_visible_(visible), b_shadow_(shadow), b_motion_blur_(motion_blur) {};
        SceneObjectMesh(SceneObjectMesh&& mesh)
            : BaseSceneObject(ESceneObjectType::kSceneObjectTypeMesh),
            index_arr_(std::move(mesh.index_arr_)),
            vertex_arr_(std::move(mesh.vertex_arr_)),
            prim_type_(mesh.prim_type_),
            b_visible_(mesh.b_visible_),
            b_shadow_(mesh.b_shadow_),
            b_motion_blur_(mesh.b_motion_blur_) {};
        void AddIndexArray(SceneObjectIndexArray&& array) { index_arr_.push_back(std::move(array)); };
        void AddVertexArray(SceneObjectVertexArray&& array) { vertex_arr_.push_back(std::move(array)); };
        void SetPrimitiveType(EPrimitiveType type) { prim_type_ = type; };
        size_t GetIndexGroupCount() const { return index_arr_.size(); };
        size_t GetIndexCount(const size_t index) const { return (index_arr_.empty() ? 0 : index_arr_[index].GetIndexCount()); };
        size_t GetVertexCount() const { return (vertex_arr_.empty() ? 0 : vertex_arr_[0].GetVertexCount()); };
        size_t GetVertexPropertiesCount() const { return vertex_arr_.size(); };
        const SceneObjectVertexArray& GetVertexPropertyArray(const size_t index) const { return vertex_arr_[index]; };
        const SceneObjectIndexArray& GetIndexArray(const size_t index) const { return index_arr_[index]; };
        const EPrimitiveType& GetPrimitiveType() { return prim_type_; };
    protected:
        std::vector<SceneObjectIndexArray>  index_arr_;
        std::vector<SceneObjectVertexArray> vertex_arr_;
        EPrimitiveType prim_type_;
        bool        b_visible_ = true;
        bool        b_shadow_ = false;
        bool        b_motion_blur_ = false;

    };
    class SceneObjectTexture : public BaseSceneObject
    {
    public:
        SceneObjectTexture() : BaseSceneObject(ESceneObjectType::kSceneObjectTypeTexture), tex_coord_id_(0) {};
        SceneObjectTexture(std::string& name) : BaseSceneObject(ESceneObjectType::kSceneObjectTypeTexture), tex_coord_id_(0), name_(name) {};
        SceneObjectTexture(uint32_t coord_index, std::shared_ptr<Image>& image) : BaseSceneObject(ESceneObjectType::kSceneObjectTypeTexture), tex_coord_id_(coord_index), p_image_(image) {};
        SceneObjectTexture(uint32_t coord_index, std::shared_ptr<Image>&& image) : BaseSceneObject(ESceneObjectType::kSceneObjectTypeTexture), tex_coord_id_(coord_index), p_image_(std::move(image)) {};
        SceneObjectTexture(SceneObjectTexture&) = default;
        SceneObjectTexture(SceneObjectTexture&&) = default;
        void SetName(std::string& name) { name_ = name; };
        void AddTransform(Matrix4x4f& matrix) { transforms_.push_back(matrix); };
    protected:
        uint32_t tex_coord_id_;
        std::string name_;
        std::shared_ptr<Image> p_image_;
        std::vector<Matrix4x4f> transforms_;
    };
    template <typename T>
    struct ParameterValueMap
    {
        T value_;
        std::shared_ptr<SceneObjectTexture> value_map_;
        ParameterValueMap() = default;
        ParameterValueMap(const T value) : value_(value) {};
        ParameterValueMap(const std::shared_ptr<SceneObjectTexture>&value) : value_map_(value) {};
        ParameterValueMap(const ParameterValueMap & rhs) = default;
        ParameterValueMap(ParameterValueMap && rhs) = default;
        ParameterValueMap& operator=(const ParameterValueMap & rhs) = default;
        ParameterValueMap& operator=(ParameterValueMap && rhs) = default;
        ParameterValueMap& operator=(const std::shared_ptr<SceneObjectTexture>&rhs)
        {
            value_map_ = rhs;
            return *this;
        };
        ~ParameterValueMap() = default;
    };

    using Color = ParameterValueMap<Vector4f>;
    using Normal = ParameterValueMap<Vector3f>;
    using Parameter = ParameterValueMap<float>;
    class SceneObjectMaterial : public BaseSceneObject
    {
    public:
        SceneObjectMaterial(const std::string& name) : BaseSceneObject(ESceneObjectType::kSceneObjectTypeMaterial), name_(name) {};
        SceneObjectMaterial(std::string&& name) : BaseSceneObject(ESceneObjectType::kSceneObjectTypeMaterial), name_(std::move(name)) {};
        SceneObjectMaterial(const std::string& name = "", Color&& base_color = Vector4f(1.0f), Parameter&& metallic = 0.0f, Parameter&& roughness = 0.0f, 
            Normal&& normal = Vector3f(0.0f, 0.0f, 1.0f), Parameter&& specular = 0.0f, Parameter&& ao = 0.0f) : 
            BaseSceneObject(ESceneObjectType::kSceneObjectTypeMaterial), name_(name), base_color_(std::move(base_color)), metallic_(std::move(metallic)),
            roughness_(std::move(roughness)), normal_(std::move(normal)), specularE(std::move(specular)), abient_occlusion_(std::move(ao)) {};
        void SetName(const std::string& name) { name_ = name; };
        void SetName(std::string&& name) { name_ = std::move(name); };
        void SetColor(std::string attrib, Vector4f& color)
        {
            if (attrib == "diffuse") {
                base_color_ = Color(color);
            }
        };
        void SetParam(std::string& attrib, float param)
        {
        };
        void SetTexture(std::string& attrib, std::string& textureName)
        {
            if (attrib == "diffuse") {
                base_color_ = std::make_shared<SceneObjectTexture>(textureName);
            }
        };
        void SetTexture(std::string& attrib, std::shared_ptr<SceneObjectTexture>& texture)
        {
            if (attrib == "diffuse") {
                base_color_ = texture;
            }
        };
    protected:
        std::string name_;
        Color       base_color_;
        Parameter   metallic_;
        Parameter   roughness_;
        Normal      normal_;
        Parameter   specularE;
        Parameter   abient_occlusion_;
    };

    class SceneObjectGeometry : public BaseSceneObject
    {
    public:
        SceneObjectGeometry() : BaseSceneObject(ESceneObjectType::kSceneObjectTypeGeometry) {};
        void SetVisibility(bool visible) { b_visible_ = visible; };
        const bool Visible() { return b_visible_; };
        void SetIfCastShadow(bool shadow) { b_shadow_ = shadow; };
        const bool CastShadow() { return b_shadow_; };
        void SetIfMotionBlur(bool motion_blur) { b_motion_blur_ = motion_blur; };
        const bool MotionBlur() { return b_motion_blur_; };
        void AddMesh(std::shared_ptr<SceneObjectMesh>& mesh) 
        {
            mesh_.push_back(mesh);
        };
        const std::weak_ptr<SceneObjectMesh> GetMesh() { return (mesh_.empty() ? nullptr : mesh_[0]); };
        const std::weak_ptr<SceneObjectMesh> GetMeshLOD(size_t lod) { return (lod < mesh_.size() ? mesh_[lod] : nullptr); };
    protected:
        std::vector<std::shared_ptr<SceneObjectMesh>> mesh_;
        bool        b_visible_;
        bool        b_shadow_;
        bool        b_motion_blur_;
    };

    using AttenFunc = float (*)(float /* Intensity */, float /* Distance */);

    class SceneObjectLight : public BaseSceneObject
    {
    protected:
        // can only be used as base class of delivered lighting objects
        SceneObjectLight() : BaseSceneObject(ESceneObjectType::kSceneObjectTypeLight) {};
    protected:
        Color       light_color_;
        float       intensity_;
        AttenFunc   light_attenuation_;
        float       near_clip_distance_;
        float       far_clip_distance_;
        bool        b_cast_shadows_;
        std::string texture_;
    public:
        void SetIfCastShadow(bool shadow) { b_cast_shadows_ = shadow; };
        void SetColor(std::string& attrib, Vector4f& color)
        {
            if (attrib == "color") {
                light_color_ = Color(color);
            }
        };
        void SetParam(std::string& attrib, float param)
        {
            if (attrib == "intensity") {
                intensity_ = param;
            }
        };
        void SetTexture(std::string& attrib, std::string& textureName)
        {
            if (attrib == "projection") {
                texture_ = textureName;
            }
        };
        void SetAttenuation(AttenFunc func)
        {
            light_attenuation_ = func;
        }
        const Color& GetColor() { return light_color_; };
        float GetIntensity() { return intensity_; };
    };

    class SceneObjectPointLight : public SceneObjectLight
    {
    public:
        using SceneObjectLight::SceneObjectLight;
    };

    class SceneObjectSpotLight : public SceneObjectLight
    {
    public:
        using SceneObjectLight::SceneObjectLight;
    protected:
        float   cone_angle_;
        float   penumbra_angle_;
    };

    class SceneObjectCamera : public BaseSceneObject
    {
    public:
        SceneObjectCamera() : BaseSceneObject(ESceneObjectType::kSceneObjectTypeCamera) {};
        SceneObjectCamera(float aspect = 16.0f / 9.0f, float near_clip = 1.0f, float far_clip = 100.0f) : BaseSceneObject(ESceneObjectType::kSceneObjectTypeCamera), 
            fov_(aspect), near_clip_distance_(near_clip), far_clip_distance_(far_clip) {};
    protected:
        float fov_;
        float near_clip_distance_;
        float far_clip_distance_;
    };
    class SceneObjectOrthogonalCamera : public SceneObjectCamera
    {
    public:
        using SceneObjectCamera::SceneObjectCamera;
    };
    class SceneObjectPerspectiveCamera : public SceneObjectCamera
    {
    protected:
        float m_fFov;

    public:
        SceneObjectPerspectiveCamera(float aspect = 16.0f / 9.0f, float near_clip = 1.0f, float far_clip = 100.0f, 
            float fov = kPi / 2.0) : SceneObjectCamera(aspect, near_clip, far_clip), m_fFov(fov) {};
    };

    class SceneObjectTransform
    {
    public:
        SceneObjectTransform() { BuildIdentityMatrix(matrix_); b_scene_object_only_ = false; };
        SceneObjectTransform(const Matrix4x4f& matrix, const bool object_only = false) { matrix_ = matrix; b_scene_object_only_ = object_only; };
        operator Matrix4x4f() { return matrix_; };
        operator const Matrix4x4f() const { return matrix_; };
    protected:
        Matrix4x4f matrix_;
        bool b_scene_object_only_;
    };
    class SceneObjectTranslation : public SceneObjectTransform
    {
    public:
        SceneObjectTranslation(const char axis, const float amount)
        {
            switch (axis) {
            case 'x':
                MatrixTranslation(matrix_, amount, 0.0f, 0.0f);
                break;
            case 'y':
                MatrixTranslation(matrix_, 0.0f, amount, 0.0f);
                break;
            case 'z':
                MatrixTranslation(matrix_, 0.0f, 0.0f, amount);
                break;
            default:
                assert(0);
            }
        }
        SceneObjectTranslation(const float x, const float y, const float z)
        {
            MatrixTranslation(matrix_, x, y, z);
        }
    };
    class SceneObjectRotation : public SceneObjectTransform
    {
    public:
        SceneObjectRotation(const char axis, const float theta)
        {
            switch (axis) {
            case 'x':
                MatrixRotationX(matrix_, theta);
                break;
            case 'y':
                MatrixRotationY(matrix_, theta);
                break;
            case 'z':
                MatrixRotationZ(matrix_, theta);
                break;
            default:
                assert(0);
            }
        }
        SceneObjectRotation(Vector3f& axis, const float theta)
        {
            Normalize(axis);
            MatrixRotationAxis(matrix_, axis, theta);
        }
        SceneObjectRotation(const Quaternion quaternion)
        {
            MatrixRotationQuaternion(matrix_, quaternion);
        }
    };

    class SceneObjectScale : public SceneObjectTransform
    {
    public:
        SceneObjectScale(const char axis, const float amount)
        {
            switch (axis) {
            case 'x':
                MatrixScale(matrix_, amount, 0.0f, 0.0f);
                break;
            case 'y':
                MatrixScale(matrix_, 0.0f, amount, 0.0f);
                break;
            case 'z':
                MatrixScale(matrix_, 0.0f, 0.0f, amount);
                break;
            default:
                assert(0);
            }
        }
        SceneObjectScale(const float x, const float y, const float z)
        {
            MatrixScale(matrix_, x, y, z);
        }
    };

}