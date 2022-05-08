#include "pch.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../External/stb-image/stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "../External/stb-image/stb_image_resize.h"

#include "Framework/Scene/SceneObjectTexture.h"
#include "Framework/Common/AssetLoader.h"

#include "Framework/Parser/JPEG.h"
#include "Framework/Parser/HDR.h"

#include <future>

namespace Engine
{
	SceneObjectTexture::SceneObjectTexture(std::string path) : BaseSceneObject(ESceneObjectType::kSceneObjectTypeTexture), tex_coord_id_(0), path_(path)
	{
		int pos = 0;
		for (auto it = path_.rbegin(); it != path_.rend(); it++)
		{
			auto c = *it;
			if (*it == '/' || *it == '\\')
				break;
			++pos;
		}
		name_ = path_.substr(path_.size() - pos);
	}

	void SceneObjectTexture::LoadTexture()
	{
		if (images_.empty())
		{
			Buffer buf = g_pAssetLoader->OpenAndReadBinarySync(path_.c_str());
			std::string ext = path_.substr(path_.find_last_of(".") + 1);
			//std::shared_ptr<Image> image;
			std::unique_ptr<IIMageParser> parser;
			if(ext == "jpg")
				parser = std::make_unique<JpegParser>();
			else if(ext == "hdr")
				parser = std::make_unique<HDRParser>();
			auto image = std::make_shared<Image>(parser->Parse(buf));
			channel_ = image->channel;
			width_ = image->width;
			height_ = image->height;
			pitch_ = image->pitch;
			int i = 2;
			while (width_ / i > 0)
			{
				++max_lod_level_;
				i *= 2;
			}
			images_.resize(max_lod_level_);
			//jpeg handle all format temp
			images_[0] = std::move(image);
			auto& img0 = *images_[0].get();
			//expand to 32 bit depth
			if (img0.channel == 3)
			{
				uint32_t new_pitch = pitch_ / 3 * 4;
				size_t data_size = new_pitch * height_;
				if(img0.format == EImageFormat::kNutFormatR8G8B8)
				{
					NE_LOG(ALL, kWarning, "{} will be expand to 32bit form 24bit", name_)
					void* data = new uint8_t[data_size];
					uint8_t* buf = reinterpret_cast<uint8_t*>(data);
					uint8_t* src = reinterpret_cast<uint8_t*>(img0.data);
					for (int row = 0; row < img0.height; row++)
					{
						buf = reinterpret_cast<uint8_t*>(data) + row * new_pitch;
						src = reinterpret_cast<uint8_t*>(img0.data) + row * img0.pitch;
						for (int col = 0; col < img0.width; col++)
						{
							*(uint32_t*)buf = *(uint32_t*)src;
							buf[3] = 255;
							buf += 4;
							src += 3;
						}
					}
					delete img0.data;
					img0.data = data;
					img0.format = EImageFormat::kNutFormatR8G8B8A8;
				}
				else if(img0.format == EImageFormat::kNutFormatR32G32B32)
				{
					NE_LOG(ALL, kWarning, "{} will be expand to 96bit bit form 128bit", name_)
					void* data = new float[data_size];
					float* buf = reinterpret_cast<float*>(data);
					float* src = reinterpret_cast<float*>(img0.data);
					int i = 0;
					for (int row = 0; row < img0.height; row++)
					{
						for (int col = 0; col < img0.width; col++)
						{
							buf[0] = src[0];
							buf[1] = src[1];
							buf[2] = src[2];
							buf[3] = 1.f;
							buf += 4;
							src += 3;
						}
					}
					delete img0.data;
					img0.data = data;
					img0.format = EImageFormat::kNutFormatR32G32B32A32;
				}
				img0.data_size = data_size;
				img0.pitch = new_pitch;				
				img0.channel = 4;
				pitch_ = new_pitch;
				channel_ = 4;
			}
		}
	}

	void SceneObjectTexture::SaveTexture()
	{
		char buf[32];
		int w = width_, h = height_;
		for (int i = 1; i < mipmap_level_; ++i)
		{
			w /= 2;
			h /= 2;
			sprintf(buf, "%i", w);
			std::string output_path{ path_.substr(0,path_.find(".")) };
			output_path.append(buf);
			output_path.append(name_.substr(name_.find(".")));
			int ret = stbi_write_png(output_path.c_str(), w, h, channel_, images_[i]->data, 0);
			if (ret == 1)
				NE_LOG(ALL, kNormal, "image save to {}", output_path)
			else
				NE_LOG(ALL, kError, "image {} save failed!", output_path)
				memset(&buf, 0x00, 16);
		}
	}
	void SceneObjectTexture::GenerateMipMap(UINT8 level)
	{
		if(images_.empty() || level > max_lod_level_) 
		{
			return;
		}
		if(level > mipmap_level_)
		{
			int w = width_,h = height_;
			for(int i = mipmap_level_; i < level; ++i)
			{
				w = width_ / pow(2,i);
				h = height_ / pow(2,i);
				int data_size = w * h * channel_;
				UINT8* buffer = new UINT8[data_size];
				stbir_resize(images_[0]->data, width_, height_, 0, buffer, w, h, 0, STBIR_TYPE_UINT8, channel_, STBIR_ALPHA_CHANNEL_NONE, 0,
					STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
					STBIR_FILTER_BOX, STBIR_FILTER_BOX,
					STBIR_COLORSPACE_SRGB, nullptr);
				std::shared_ptr<Image> img = std::make_shared<Image>();
				img->width = w;
				img->height = h;
				img->channel = channel_;
				img->data_size = data_size;
				img->format = images_[0]->format;
				img->pitch = pitch_ / pow(2, i);
				img->data = buffer;
				images_[i] = std::move(img);
			}
			mipmap_level_ = level;
		}
	}
}
