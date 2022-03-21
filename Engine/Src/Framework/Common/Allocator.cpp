#include "pch.h"
#include <cassert>

#include "Framework/Common/Allocator.hpp"

using namespace Engine;

//Completing x to a multiple of a, a 2^n
//https://zhidao.baidu.com/question/490149950.html?
#ifndef ALIGN
#define ALIGN(x,a)	(((x) + ((a)-1)) & ~((a) -1))
#endif // !ALIGN




Engine::Allocator::Allocator(size_t size, size_t page_size, size_t align) : p_pagelist_(nullptr),p_freelist_(nullptr)
{
	Reset(size, page_size, align);
}

Engine::Allocator::~Allocator()
{
	FreeAll();
}

void Engine::Allocator::Reset(size_t size, size_t page_size, size_t align)
{
	FreeAll();
	data_size_ = size;
	page_size_ = page_size;
	size_t minimal_size = (sizeof(BlockHeader) > data_size_) ? sizeof(BlockHeader) : data_size_;
#ifdef _DEBUG
	//make sure align is 2^n
	assert(align > 0 && ((align & (align - 1))) == 0);
#endif
	block_size_ = ALIGN(minimal_size, align);
	allgnment_size_ = block_size_ - minimal_size;
	block_per_page_ = static_cast<size_t>((page_size_ - sizeof(PageHeader)) / block_size_);
}

void* Engine::Allocator::Allocate()
{
	if (!p_freelist_)
	{
		PageHeader* p_new_page = reinterpret_cast<PageHeader*>(new uint8_t[page_size_]);
		++num_page_;
		num_block_ += block_per_page_;
		num_freeblock_ += block_per_page_;
#ifdef _DEBUG
		FillFreePage(p_new_page);
#endif // _DEBUG
		if (p_pagelist_) p_new_page->p_next_ = p_pagelist_;
		p_pagelist_ = p_new_page;
		BlockHeader* p_block = p_new_page->Blocks();
		// link each page in block
		for (uint32_t i = 0; i < block_per_page_ - 1; i++)
		{
			p_block->p_next_ = NextBlock(p_block);
			p_block = NextBlock(p_block);
		}
		p_block->p_next_ = nullptr;
		p_freelist_ = p_new_page->Blocks();
	}
	BlockHeader* p_free_block = p_freelist_;
	p_freelist_ = p_freelist_->p_next_;
	--num_freeblock_;
#ifdef _DEBUG
	FillAllocatedBlock(p_free_block);
#endif // _DEBUG
	return reinterpret_cast<void*>(p_free_block);
}

void Engine::Allocator::Free(void* p)
{
	BlockHeader* block = reinterpret_cast<BlockHeader*>(p);
#ifdef _DEBUG
	FillFreeBlock(block);
#endif // _DEBUG
	block->p_next_ = p_freelist_;
	p_freelist_ = block;
	++num_freeblock_;
}

void Engine::Allocator::FreeAll()
{
	PageHeader* p_page = p_pagelist_;
	while (p_page)
	{
		PageHeader* p = p_page;
		p_page = p_page->p_next_;
		delete[] reinterpret_cast<uint8_t*>(p);
 	}
	p_pagelist_ = nullptr;
	p_freelist_ = nullptr;
	num_page_ = 0;
	num_block_ = 0;
	num_freeblock_ = 0;
}

#ifdef _DEBUG

void Engine::Allocator::FillFreePage(PageHeader* p_page)
{
	p_page->p_next_ = nullptr;
	BlockHeader* p_block = p_page->Blocks();
	for (uint32_t i = 0; i < block_per_page_; i++)
	{
		FillFreeBlock(p_block);
		p_block = NextBlock(p_block);
	}
}

void Engine::Allocator::FillFreeBlock(BlockHeader* p_block)
{
	std::memset(p_block, kPATTERN_FREE, block_size_ - allgnment_size_);
	std::memset(reinterpret_cast<uint8_t*>(p_block) + block_size_ - allgnment_size_, kPATTERN_ALIGN, allgnment_size_);
}

void Engine::Allocator::FillAllocatedBlock(BlockHeader* p_block)
{
	std::memset(p_block, kPATTERN_ALLOC, block_size_ - allgnment_size_);
	std::memset(reinterpret_cast<uint8_t*>(p_block) + block_size_ - allgnment_size_, kPATTERN_ALLOC, allgnment_size_);
}

#endif // _DEBUG




BlockHeader* Engine::Allocator::NextBlock(BlockHeader* p_block)
{
	return reinterpret_cast<BlockHeader*>(reinterpret_cast<uint8_t*>(p_block) + block_size_);
}
