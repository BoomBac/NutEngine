#ifndef __SCENE_OBJECT_TEXTURE_H__
#define __SCENE_OBJECT_TEXTURE_H__

#include "SceneObject.h"

namespace Engine
{
    class SceneObjectTexture : public BaseSceneObject
    {
    public:
        SceneObjectTexture() : BaseSceneObject(ESceneObjectType::kSceneObjectTypeTexture), tex_coord_id_(0) {};
        SceneObjectTexture(std::string path);
        SceneObjectTexture(std::string path, std::string name) : BaseSceneObject(ESceneObjectType::kSceneObjectTypeTexture, name), tex_coord_id_(0), path_(path) {};
        SceneObjectTexture(std::string path, std::string name, UINT32 mipmap_level) : SceneObjectTexture(path, name)
        {
            mipmap_level_ = mipmap_level;
        }
        //SceneObjectTexture(uint32_t coord_index, std::shared_ptr<Image>& image) : BaseSceneObject(ESceneObjectType::kSceneObjectTypeTexture), tex_coord_id_(coord_index), p_image_(image) {};
        //SceneObjectTexture(uint32_t coord_index, std::shared_ptr<Image>&& image) : BaseSceneObject(ESceneObjectType::kSceneObjectTypeTexture), tex_coord_id_(coord_index), 
        //    p_image_(std::move(image)) {};
        SceneObjectTexture(SceneObjectTexture&) = default;
        SceneObjectTexture(SceneObjectTexture&&) = default;
        void AddTransform(Matrix4x4f& matrix) { transforms_.push_back(matrix); };
        Image& GetImage()
        {
            if (images_.empty()) LoadTexture();
            return *images_[0];
        }
        Image& GetImage(int mip_level)
        {
            if (images_.empty()) LoadTexture();
            if (mip_level > mipmap_level_)
            {
                NE_LOG(ALL, kWarning, "texture: {} has't mip level {},will return most detailed level!", name_, mip_level)
                    return GetImage();
            }
            return *images_[mip_level];
        }
        void SaveTexture();
        void GenerateMipMap(UINT8 level = 1u);

        UINT8 mipmap_level_ = 1;
        INT32 width_;
        INT32 height_;
        INT64 pitch_;
        INT32 channel_;
    private:
        UINT8 max_lod_level_ = 1;
        void LoadTexture();
        uint32_t tex_coord_id_;
        std::vector<std::shared_ptr<Image>> images_;
        std::vector<Matrix4x4f> transforms_;
        std::string path_;
    };
}

#endif // !__SCENE_OBJECT_TEXTURE_H__








