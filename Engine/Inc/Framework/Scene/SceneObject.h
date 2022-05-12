#ifndef __SCENE_OBJECT_H__
#define __SCENE_OBJECT_H__

#include "Framework/Common/Image.h"
#include "Framework/Math/Guid.h"
#include "Framework/Common/Log.h"

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
        kPrimitiveTypeLineList,    ///< For N>=0, vertices [N*2+0, N*2+1] render a line.
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
    enum class EVertexArrayType
    {
        kVertex,
        kNormal,
        kUVs,
        kColor,
        kTangant
    };

    enum class ESceneObjectCollisionType
    {
        kSceneObjectCollisionTypeNone,
        kSceneObjectCollisionTypeSphere,
        kSceneObjectCollisionTypeBox,
        kSceneObjectCollisionTypeCylinder,
        kSceneObjectCollisionTypeCapsule,
        kSceneObjectCollisionTypeCone,
        kSceneObjectCollisionTypeMultiSphere,
        kSceneObjectCollisionTypeConvexHull,
        kSceneObjectCollisionTypeConvexMesh,
        kSceneObjectCollisionTypeBvhMesh,
        kSceneObjectCollisionTypeHeightfield,
        kSceneObjectCollisionTypePlane
    };

    enum class ELightType
    {
        kDirectional,
        kPoint,
        kSpot
    };

    class BaseSceneObject
    {
    protected:
        BaseSceneObject(ESceneObjectType type) : type_(type), name_("null") { guid_ = NewGuid(); };
        BaseSceneObject(ESceneObjectType type, std::string name) : type_(type), name_(name) { guid_ = NewGuid(); };
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
        std::string name_;
    public:
        const Guid& GetGuid() const
        {
            return guid_;
        }
        void SetName(std::string& name) { name_ = name; };
        void SetName(std::string&& name) { name_ = std::move(name); };
        const std::string& GetName() const { return name_; };
    };

    class SceneObjectVertexArray
    {
    public:
        SceneObjectVertexArray() = default;
        SceneObjectVertexArray(EVertexArrayType type, const uint32_t morph_index = 0, const EVertexDataType data_type = EVertexDataType::kVertexDataFloat3,
            void* data = nullptr, const size_t data_size = 0) : type_(type),
            morph_target_index_(morph_index), data_type_(data_type), p_data_(data), size_(data_size) {};
        SceneObjectVertexArray(SceneObjectVertexArray& arr) = default;
        SceneObjectVertexArray(SceneObjectVertexArray&& arr) = default;

        SceneObjectVertexArray& operator=(SceneObjectVertexArray&& other)
        {
            this->type_ = other.type_;
            this->morph_target_index_ = other.morph_target_index_;
            this->data_type_ = other.data_type_;
            this->size_ = other.size_;
            this->p_data_ = other.p_data_;
            other.p_data_ = nullptr;
            return *this;
        }

        size_t GetDataSize() const
        {
            size_t size = size_;
            switch (data_type_) {
            case EVertexDataType::kVertexDataFloat1:
            case EVertexDataType::kVertexDataFloat2:
                size *= sizeof(float) * 2;
                break;
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
        EVertexArrayType GetType() const { return type_; }
    protected:
        EVertexArrayType type_;
        uint32_t    morph_target_index_;
        EVertexDataType data_type_;
        void* p_data_;
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
        SceneObjectIndexArray(SceneObjectIndexArray&& arr) = default;
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

    class SceneObjectTexture;
    template <typename T>
    struct ParameterValueMap
    {
        T value_;
        std::shared_ptr<SceneObjectTexture> value_map_;
        ParameterValueMap() = default;
        ParameterValueMap(const T value) : value_(value) {};
        ParameterValueMap(const std::shared_ptr<SceneObjectTexture>& value) : value_map_(value) {};
        ParameterValueMap(const ParameterValueMap& rhs) = default;
        ParameterValueMap(ParameterValueMap&& rhs) = default;
        ParameterValueMap& operator=(const ParameterValueMap& rhs) = default;
        ParameterValueMap& operator=(ParameterValueMap&& rhs) = default;
        ParameterValueMap& operator=(const std::shared_ptr<SceneObjectTexture>& rhs)
        {
            value_map_ = rhs;
            return *this;
        };
        ~ParameterValueMap() = default;
    };

    using Color = ParameterValueMap<Vector4f>;
    using Normal = ParameterValueMap<Vector3f>;
    using Parameter = ParameterValueMap<float>;

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

#endif // __SCENE_OBJECT_H__

