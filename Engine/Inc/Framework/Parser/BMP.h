#pragma once
#include "../Interface/ImageParser.h"

namespace Engine 
{
	constexpr uint8_t kBitMapFileHeaderSize = 14;
#pragma pack(push,1)
	struct BitMapFileHeader
	{
		uint16_t signature;
		uint32_t size;
		uint32_t reserved;
		uint32_t bit_offset;
	};
	struct BitMapHeader
	{
		uint32_t header_size;
		int32_t width;
		int32_t height;
		uint16_t planes;
		uint16_t bit_count;
		uint32_t compression;
		uint32_t size_image;
		int32_t pels_perMeterX;
		int32_t pels_perMeterY;
		uint32_t clr_used;
		uint32_t clr_important;
	};
#pragma pack(pop)

	class BmpParser : public IIMageParser
	{
        virtual Image Parse(const Buffer& buf)
        {
            Image img;
            BitMapFileHeader* pFileHeader = reinterpret_cast<BitMapFileHeader*>(buf.p_data_);
            BitMapHeader* pBmpHeader = reinterpret_cast<BitMapHeader*>(buf.p_data_ + kBitMapFileHeaderSize);
            if (pFileHeader->signature == 0x4D42 /* 'B''M' */) 
            {
                std::cout << "Asset is Windows BMP file" << std::endl;
                std::cout << "BMP Header" << std::endl;
                std::cout << "----------------------------" << std::endl;
                std::cout << "File Size: " << pFileHeader->size << std::endl;
                std::cout << "Data Offset: " << pFileHeader->bit_offset << std::endl;
                std::cout << "Image Width: " << pBmpHeader->width << std::endl;
                std::cout << "Image Height: " << pBmpHeader->height << std::endl;
                std::cout << "Image Planes: " << pBmpHeader->planes << std::endl;
                std::cout << "Image BitCount: " << pBmpHeader->bit_count << std::endl;
                std::cout << "Image Compression: " << pBmpHeader->compression << std::endl;
                std::cout << "Image Size: " << pBmpHeader->size_image << std::endl;

                img.width = pBmpHeader->width;
                img.height = pBmpHeader->height;
                img.bit_count = 32;
                img.pitch = ((img.width * img.bit_count >> 3) + 3) & ~3;
                img.data_size = img.pitch * img.height;
                img.data = reinterpret_cast<R8G8B8A8Unorm*>(g_pMemoryManager->Allocate(img.data_size));

                if (img.bit_count < 24) 
                {
                    std::cout << "Sorry, only true color BMP is supported at now." << std::endl;
                }
                else 
                {
                    uint8_t* pSourceData = buf.p_data_ + pFileHeader->bit_offset;
                    for (int32_t y = img.height - 1; y >= 0; y--) 
                    {
                        for (uint32_t x = 0; x < img.width; x++)
                        {
                            (img.data + img.width * (img.height - y - 1) + x)->bgra = *reinterpret_cast<R8G8B8A8Unorm*>(pSourceData + img.pitch * y + x * (img.bit_count >> 3));
                        }
                    }
                }
            }

            return img;
        }
	};
}
