#ifndef __COMMAND_QUEUE__
#define __COMMAND_QUEUE__
#include "../../../pch.h"
#include <d3d12.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;
namespace Engine
{
	class CommandQueue
	{
	public:
		CommandQueue(ComPtr<ID3D12Device> device,D3D12_COMMAND_LIST_TYPE type);
		~CommandQueue();
		/// <summary>
		/// Prioritize popping up the available list from the maintained command list queue, and if the commands in the allocator corresponding 
		/// to that list are not executed, a new list of available lists and allocators is created and associated
		/// </summary>
		/// <returns>An available command list</returns>
		ComPtr<ID3D12GraphicsCommandList> GetCommandList();
		uint64_t ExecuteCommandList(ComPtr<ID3D12GraphicsCommandList> cmd_list);
		/// <summary>
		/// Make the currently maintained fence point +1, set a fence point from the GPU with this value
		/// </summary>
		/// <returns>The currently set fence value</returns>
		uint64_t Signal();
		bool IsFenceComplete(uint64_t fence_value);
		void WaitForFenceValue(uint64_t fence_value);
		void Flush();
		ComPtr<ID3D12CommandQueue> GetD3D12CommandQueue() const;
	protected:
		ComPtr<ID3D12CommandAllocator> CreateCommandAllocator();
		ComPtr<ID3D12GraphicsCommandList> CreateCommandList(ComPtr<ID3D12CommandAllocator> allocator);
	private:
		//Store the allocator and its corresponding fence value to prevent the allocator from being reused before the execution is complete.
		struct CommandAllocatorEntry
		{
			uint64_t fence_value;
			ComPtr<ID3D12CommandAllocator> cmd_alloc;
		};
		using CommandAllocatorQueue = std::queue<CommandAllocatorEntry>;
		using CommandListQueue = std::queue<ComPtr<ID3D12GraphicsCommandList>>;
		D3D12_COMMAND_LIST_TYPE                     cmd_list_type_;
		ComPtr<ID3D12Device>       p_device_;
		ComPtr<ID3D12CommandQueue>  p_cmd_queue_;
		ComPtr<ID3D12Fence>         p_fence_;
		HANDLE                                      fence_event_;
		uint64_t                                    fence_var_;
		CommandAllocatorQueue                       cmd_alloc_queue_;
		CommandListQueue                            cmd_list_queue_;
	};
}
#endif // !__COMMAND_QUEUE__

