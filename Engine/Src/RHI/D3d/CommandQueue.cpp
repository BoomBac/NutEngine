#include "pch.h"
#include "RHI/D3d/CommandQueue.h"

namespace Engine
{
	CommandQueue::CommandQueue(ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type)
		: fence_var_(0),cmd_list_type_(type),p_device_(device)
	{
		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = type;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		ThrowIfFailed(p_device_->CreateCommandQueue(&desc, IID_PPV_ARGS(&p_cmd_queue_)));
		ThrowIfFailed(p_device_->CreateFence(fence_var_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&p_fence_)));

		fence_event_ = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		assert(fence_event_ && "Failed to create fence event handle.");
	}
	CommandQueue::~CommandQueue()
	{
	}
	ComPtr<ID3D12GraphicsCommandList> CommandQueue::GetCommandList()
	{
		ComPtr<ID3D12CommandAllocator> cmd_alloc;
		ComPtr<ID3D12GraphicsCommandList> cmd_list;
		if (!cmd_alloc_queue_.empty() && IsFenceComplete(cmd_alloc_queue_.front().fence_value))
		{
			cmd_alloc = cmd_alloc_queue_.front().cmd_alloc;
			cmd_alloc_queue_.pop();
			ThrowIfFailed(cmd_alloc->Reset());
		}
		else
			cmd_alloc = CreateCommandAllocator();
		if (!cmd_list_queue_.empty())
		{
			cmd_list = cmd_list_queue_.front();
			cmd_list_queue_.pop();
			ThrowIfFailed(cmd_list->Reset(cmd_alloc.Get(), nullptr));
		}
		else
			cmd_list = CreateCommandList(cmd_alloc);
		// Associate the command allocator with the command list so that it can be
		// retrieved when the command list is executed.
		ThrowIfFailed(cmd_list->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), cmd_alloc.Get()));

		return cmd_list;
	}

	uint64_t CommandQueue::ExecuteCommandList(ComPtr<ID3D12GraphicsCommandList> cmd_list)
	{
		cmd_list->Close();
		ID3D12CommandAllocator* cmd_alloc;
		UINT data_size = sizeof(cmd_alloc);
		ThrowIfFailed(cmd_list->GetPrivateData(__uuidof(ID3D12CommandAllocator), &data_size, &cmd_alloc));
		ID3D12CommandList* const ppCommandLists[] = {cmd_list.Get()};
		p_cmd_queue_->ExecuteCommandLists(1, ppCommandLists);
		uint64_t fence_value = Signal();
		cmd_alloc_queue_.emplace(CommandAllocatorEntry{ fence_value, cmd_alloc });
		cmd_list_queue_.push(cmd_list);
		// The ownership of the command allocator has been transferred to the ComPtr
		// in the command allocator queue. It is safe to release the reference 
		// in this temporary COM pointer here.
		cmd_alloc->Release();
		return fence_value;
	}
	uint64_t CommandQueue::Signal()
	{
		uint64_t fence_var = ++fence_var_;
		p_cmd_queue_->Signal(p_fence_.Get(), fence_var);
		return fence_var;
	}
	bool CommandQueue::IsFenceComplete(uint64_t fence_value)
	{
		return p_fence_->GetCompletedValue() >= fence_value;
	}
	void CommandQueue::WaitForFenceValue(uint64_t fence_value)
	{	
		if(!IsFenceComplete(fence_value))
		{
			p_fence_->SetEventOnCompletion(fence_value,fence_event_);
			::WaitForSingleObject(fence_event_,DWORD_MAX);
		}
	}
	void CommandQueue::Flush()
	{
		WaitForFenceValue(Signal());
	}
	ComPtr<ID3D12CommandQueue> CommandQueue::GetD3D12CommandQueue() const
	{
		return p_cmd_queue_;
	}
	ComPtr<ID3D12CommandAllocator> CommandQueue::CreateCommandAllocator()
	{
		ComPtr<ID3D12CommandAllocator> command_alloc;
		ThrowIfFailed(p_device_->CreateCommandAllocator(cmd_list_type_, IID_PPV_ARGS(&command_alloc)));
		return command_alloc;
	}
	ComPtr<ID3D12GraphicsCommandList> CommandQueue::CreateCommandList(ComPtr<ID3D12CommandAllocator> allocator)
	{
		ComPtr<ID3D12GraphicsCommandList> cmd_list;
		ThrowIfFailed(p_device_->CreateCommandList(0, cmd_list_type_, allocator.Get(), nullptr, IID_PPV_ARGS(&cmd_list)));
		return cmd_list;
	}
}
