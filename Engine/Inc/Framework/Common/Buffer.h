#include "MemoryManager.hpp"

namespace Engine
{
	extern MemoryManager* g_pMemoryManager;
	class Buffer
	{
    public:
        Buffer() : p_data_(nullptr), size_(0), alignment_(alignof(uint32_t)) {}
        Buffer(size_t size, size_t alignment = 4) : size_(size), alignment_(alignment)
        { 
            p_data_ = reinterpret_cast<uint8_t*>(g_pMemoryManager->Allocate(size, alignment)); 
        }
        ~Buffer() 
        { 
            if (p_data_) g_pMemoryManager->Free(p_data_, size_); 
            p_data_ = nullptr; 
        }
        Buffer(const Buffer& rhs) noexcept
        {
            p_data_ = reinterpret_cast<uint8_t*>(g_pMemoryManager->Allocate(rhs.size_, rhs.alignment_));
            memcpy(p_data_,rhs.p_data_, rhs.size_);
            size_ = rhs.size_;
            alignment_ = rhs.alignment_;
        }
        Buffer(Buffer&& rhs) noexcept
        {
            p_data_ = rhs.p_data_;
            size_ = rhs.size_;
            alignment_ = rhs.alignment_;
            rhs.p_data_ = nullptr;
            rhs.size_ = 0;
            rhs.alignment_ = 4;
        }
        Buffer& operator = (const Buffer& rhs) 
        {
            if (size_ >= rhs.size_ && alignment_ == rhs.alignment_) 
            {
                memcpy(p_data_, rhs.p_data_, rhs.size_);
            }
            else 
            {
                if (p_data_) g_pMemoryManager->Free(p_data_, size_);
                p_data_ = reinterpret_cast<uint8_t*>(g_pMemoryManager->Allocate(rhs.size_, rhs.alignment_));
                memcpy(p_data_, rhs.p_data_, rhs.size_);
                size_ = rhs.size_;
                alignment_ = rhs.alignment_;
            }
            return *this;
        }
        Buffer& operator = (Buffer&& rhs)  noexcept
        {
            if (p_data_) g_pMemoryManager->Free(p_data_, size_);
            p_data_ = rhs.p_data_;
            size_ = rhs.size_;
            alignment_ = rhs.alignment_;
            rhs.p_data_ = nullptr;
            rhs.size_ = 0;
            rhs.alignment_ = 4;
            return *this;
        }
    public:
        uint8_t* p_data_;
        size_t size_;
        size_t alignment_;
	};
}