#include "../pch.h"

namespace Engine
{
	struct BlockHeader
	{
		BlockHeader* p_next_;
	};
	struct PageHeader
	{
		PageHeader* p_next_;
		BlockHeader* Blocks() { return reinterpret_cast<BlockHeader*>(this + 1); }
	};
	class Allocator
	{
	public:
		//252 - 254
		static constexpr uint8_t kPATTERN_ALIGN = 0xFC;
		static constexpr uint8_t kPATTERN_ALLOC = 0xFD;
		static constexpr uint8_t kPATTERN_FREE = 0xFE;
		//reset allocator
		Allocator();
		Allocator(size_t size, size_t page_size, size_t align);
		~Allocator();
		void Reset(size_t size, size_t page_size, size_t align);
		//alloc / free blocks
		void* Allocate();
		void Free(void* p);
		void FreeAll();
	private:
#if defined(_DEBUG)
		//fill with debug partern
		void FillFreePage(PageHeader* p_page);
		void FillFreeBlock(BlockHeader* p_block);
		void FillAllocatedBlock(BlockHeader* p_block);
#endif 
		BlockHeader* NextBlock(BlockHeader* p_block);
		PageHeader* p_pagelist_;
		BlockHeader* p_freelist_;
		size_t data_size_;
		size_t page_size_;
		size_t allgnment_size_;
		size_t block_size_;
		uint32_t block_per_page_;
		uint32_t num_page_;
		uint32_t num_block_;
		uint32_t num_freeblock_;
		Allocator(Allocator& other) = delete;
		Allocator& operator=(Allocator& other) = delete;
	};
}