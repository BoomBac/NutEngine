#include "pch.h"
#include "Framework/Common/MemoryManager.hpp"


#ifndef ALIGN
#define ALIGN(x, a)         (((x) + ((a) - 1)) & ~((a) - 1))
#endif

using namespace Engine;

namespace 
{
	static constexpr uint32_t KBlockSizes[] =
	{
		// 4-increments
		4,  8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48,
		52, 56, 60, 64, 68, 72, 76, 80, 84, 88, 92, 96,
		// 32-increments
		128, 160, 192, 224, 256, 288, 320, 352, 384,
		416, 448, 480, 512, 544, 576, 608, 640,
		// 64-increments
		704, 768, 832, 896, 960, 1024
	};
	static constexpr uint32_t kPageSize = 8192;
	static constexpr uint32_t kAlignment = 4;

	// number of elements in the block size array
	static constexpr uint32_t kNumBlockSizes =sizeof(KBlockSizes) / sizeof(KBlockSizes[0]);
	// largest valid block size
	static constexpr uint32_t kMaxBlockSize = KBlockSizes[kNumBlockSizes - 1];
}


int Engine::MemoryManager::Initialize()
{
		// one-time initialization
		static bool sbInitialized = false;
		if (!sbInitialized) 
		{
		//initialize block size lookup table,map the size to allocator index
		p_block_size_lookup_ = new size_t[kMaxBlockSize + 1];
		size_t j = 0;
		for (size_t i = 0; i <= kMaxBlockSize; i++) 
		{
			if (i > KBlockSizes[j]) ++j;
			p_block_size_lookup_[i] = j;
		}
		// initialize the allocators 
		//
		p_allocator_ = new Allocator[kNumBlockSizes];
		for (size_t i = 0; i < kNumBlockSizes; i++) 
		{
			p_allocator_[i].Reset(KBlockSizes[i], kPageSize, kAlignment);
		}
		sbInitialized = true;
	}

	return 0;
}

void Engine::MemoryManager::Finalize()
{
	delete[] p_allocator_;
	delete[] p_block_size_lookup_;
}

void Engine::MemoryManager::Tick()
{
}

void* Engine::MemoryManager::Allocate(size_t size)
{
	Allocator* palloc = LookupAlloctor(size);
	if (palloc)
		return palloc->Allocate();
	else
		return ::operator new(size);
}

void* Engine::MemoryManager::Allocate(size_t size, size_t alignment)
{
	uint8_t* p;
	size += alignment;
	Allocator* pAlloc = LookupAlloctor(size);
	if (pAlloc)
		p = reinterpret_cast<uint8_t*>(pAlloc->Allocate());
	else
		p = reinterpret_cast<uint8_t*>(malloc(size));
	//TODO(): is aligned address vaild?
	p = reinterpret_cast<uint8_t*>(ALIGN(reinterpret_cast<size_t>(p), alignment));
	return static_cast<void*>(p);
}

void Engine::MemoryManager::Free(void* p, size_t size)
{
	Allocator* palloc = LookupAlloctor(size);
	if (palloc)
		return palloc->Free(p);
	else
		::operator delete(p);
}

Allocator* Engine::MemoryManager::LookupAlloctor(size_t size)
{
	if (size <= kMaxBlockSize)
		return p_allocator_ + p_block_size_lookup_[size];
	else
		return nullptr;
}
