#include "Render.h"
#include <iostream>
#include <cassert>

#define RND_ASSERT assert(_lastError == S_OK)

Render::Render(unsigned wndWidth, unsigned wndHeight) : _wndWidth(wndWidth), _wndHeight(wndHeight)
{
}

Render::~Render()
{
	if (_device != NULL)
		FlushCommandQueue(); //Let the command queue execute all commands before closing the window.
}

bool Render::Initialize(HWND hWindow)
{
	_hWindow = hWindow;
#pragma region Enable Debug
	_lastError = D3D12GetDebugInterface(IID_PPV_ARGS(_debugController.GetAddressOf()));
	RND_ASSERT;
	_debugController->EnableDebugLayer();
#pragma endregion
#pragma region Create Factory
	_lastError = CreateDXGIFactory1(IID_PPV_ARGS(_factory.GetAddressOf()));
	RND_ASSERT;
#pragma endregion
#pragma region Create Device
	_lastError = D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(_device.GetAddressOf()));
	RND_ASSERT;
#pragma endregion
#pragma region Create Fence
	_lastError = _device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_fence.GetAddressOf()));
	RND_ASSERT;
#pragma endregion
#pragma region Retrieve Descriptor Sizes
	_rtvDescSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	_dsvDescSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	_cbsruavDescSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
#pragma endregion
#pragma region Check MSAA Support
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQLevels;
	msQLevels.Format = BACKBUF_FORMAT;
	msQLevels.SampleCount = 4;
	msQLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQLevels.NumQualityLevels = 0;
	_lastError = _device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQLevels,
		sizeof(msQLevels));
	RND_ASSERT;
	_4xMsaaQualityLevels = msQLevels.NumQualityLevels;
	assert(_4xMsaaQualityLevels > 0);
#pragma endregion
#pragma region Create Command Objects
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	_lastError = _device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(_commandQueue.GetAddressOf()));
	RND_ASSERT;
	_lastError = _device->CreateCommandAllocator(cmdQueueDesc.Type, IID_PPV_ARGS(_commandAlloc.GetAddressOf()));
	RND_ASSERT;
	_lastError = _device->CreateCommandList(0,
		cmdQueueDesc.Type,
		_commandAlloc.Get(),
		NULL,
		IID_PPV_ARGS(_commandList.GetAddressOf()));
	RND_ASSERT;
#pragma endregion
#pragma region Create Swap Chain
	DXGI_SWAP_CHAIN_DESC swapDesc;
	swapDesc.BufferDesc.Width = _wndWidth;
	swapDesc.BufferDesc.Height = _wndHeight;
	swapDesc.BufferDesc.RefreshRate = { REFRESH_RATE, 1 };
	swapDesc.BufferDesc.Format = BACKBUF_FORMAT;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.SampleDesc.Count = _4xMsaaEnabled ? 4 : 1;
	swapDesc.SampleDesc.Quality = _4xMsaaEnabled ? (_4xMsaaQualityLevels - 1) : 0;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.BufferCount = SC_NUM_BUFFERS;
	swapDesc.OutputWindow = _hWindow;
	swapDesc.Windowed = true;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	_lastError = _factory->CreateSwapChain(_commandQueue.Get(), &swapDesc, _swapChain.GetAddressOf());
	RND_ASSERT;
#pragma endregion
#pragma region Create RTV and DSV Descriptor Heaps
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SC_NUM_BUFFERS;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	_lastError = _device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(_rtvHeap.GetAddressOf()));
	RND_ASSERT;
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	_lastError = _device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(_dsvHeap.GetAddressOf()));
	RND_ASSERT;
#pragma endregion
#pragma region Create RTVs	
	for (unsigned i = 0; i < SC_NUM_BUFFERS; i++)
	{
		_lastError = _swapChain->GetBuffer(i, IID_PPV_ARGS(_scBuffers[i].GetAddressOf()));
		RND_ASSERT;
		_device->CreateRenderTargetView(_scBuffers[i].Get(), NULL, GetScBufDesc(i)); //NULL for desc means description of the back buffer will be used
	}
#pragma endregion
#pragma region Create DSV and DS buffer
	D3D12_RESOURCE_DESC dsDesc;
	dsDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	dsDesc.Alignment = 0;
	dsDesc.Width = _wndWidth;
	dsDesc.Height = _wndHeight;
	dsDesc.DepthOrArraySize = 1;
	dsDesc.MipLevels = 1;
	dsDesc.Format = DEPTH_STENCIL_FORMAT;
	dsDesc.SampleDesc.Count = _4xMsaaEnabled ? 4 : 1;
	dsDesc.SampleDesc.Quality = _4xMsaaEnabled ? (_4xMsaaQualityLevels - 1) : 0;
	dsDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	dsDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	D3D12_CLEAR_VALUE dsClearVal;
	dsClearVal.Format = DEPTH_STENCIL_FORMAT;
	dsClearVal.DepthStencil.Depth = 1.0f;
	dsClearVal.DepthStencil.Stencil = 0;
	CD3DX12_HEAP_PROPERTIES heapP(D3D12_HEAP_TYPE_DEFAULT);
	_lastError = _device->CreateCommittedResource(	&heapP,
													D3D12_HEAP_FLAG_NONE,
													&dsDesc,
													D3D12_RESOURCE_STATE_COMMON,
													&dsClearVal,
													IID_PPV_ARGS(_depthStencilBuffer.GetAddressOf()));
	RND_ASSERT;
	_device->CreateDepthStencilView(_depthStencilBuffer.Get(), nullptr, GetDepthStencilBufDesc());
	auto rb = CD3DX12_RESOURCE_BARRIER::Transition(_depthStencilBuffer.Get(),
													D3D12_RESOURCE_STATE_COMMON,
													D3D12_RESOURCE_STATE_DEPTH_WRITE);
	_commandList->ResourceBarrier(1, &rb);
#pragma endregion
#pragma region Initialize Viewport
	_viewportRect.Width	= (float)_wndWidth;
	_viewportRect.Height	= (float)_wndHeight;
	_viewportRect.TopLeftX = 0.0f;
	_viewportRect.TopLeftY = 0.0f;
	_viewportRect.MinDepth = 0.0f;
	_viewportRect.MaxDepth = 1.0f;
#pragma endregion
#pragma region Initialize Scissors Rectangle
	_scissorsRect.top		= 0;
	_scissorsRect.left		= 0;
	_scissorsRect.bottom	= _wndHeight;
	_scissorsRect.right		= _wndWidth;
#pragma endregion
#pragma region Submit Command List
	_lastError = _commandList->Close();
	RND_ASSERT;
	ID3D12CommandList* cmdLists[] = { _commandList.Get() };
	_commandQueue->ExecuteCommandLists(1, cmdLists);
	FlushCommandQueue();
#pragma endregion
	return true;
}

bool Render::Draw()
{
	_lastError = _commandAlloc->Reset();
	RND_ASSERT;
	_lastError = _commandList->Reset(_commandAlloc.Get(), NULL);
	RND_ASSERT;
	auto bbt1 = CD3DX12_RESOURCE_BARRIER::Transition(GetBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	_commandList->ResourceBarrier(1, &bbt1);
	_commandList->RSSetViewports(1, &_viewportRect);
	_commandList->RSSetScissorRects(1, &_scissorsRect);
	_commandList->ClearRenderTargetView(GetBackBufferDesc(), BB_CLEAR_COLOR, 0, NULL);
	_commandList->ClearDepthStencilView(GetDepthStencilBufDesc(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	auto bbd = GetBackBufferDesc();
	auto dsb = GetDepthStencilBufDesc();
	_commandList->OMSetRenderTargets(1, &bbd, true, &dsb);

	auto bbt2 = CD3DX12_RESOURCE_BARRIER::Transition(GetBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	_commandList->ResourceBarrier(1, &bbt2);

	_backBufferId = _backBufferId + 1 < SC_NUM_BUFFERS ? _backBufferId + 1 : 0;

	_lastError = _commandList->Close();
	RND_ASSERT;
	ID3D12CommandList* cmdLists[] = { _commandList.Get() };
	_commandQueue->ExecuteCommandLists(1, cmdLists);

	_lastError = _swapChain->Present(0, 0);
	RND_ASSERT;

	FlushCommandQueue();
	return true;
}


void Render::FlushCommandQueue()
{
	_currentFence++;
	_lastError = _commandQueue->Signal(_fence.Get(), _currentFence); //Submit to GPU a command to increment fence value
	RND_ASSERT;
	auto completedFence = _fence->GetCompletedValue();
	if (completedFence < _currentFence) //unnecessary???
	{
		HANDLE eventHandle = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);
		_lastError = _fence->SetEventOnCompletion(_currentFence, eventHandle); //Assign the event to fire when fence reaches necessary value.
		RND_ASSERT;
		WaitForSingleObject(eventHandle, INFINITE); //Wait until the assigned event is fired.
		CloseHandle(eventHandle);
	}
}

ID3D12Resource* Render::GetBackBuffer()
{
	return _scBuffers[_backBufferId].Get();
}


D3D12_CPU_DESCRIPTOR_HANDLE Render::GetBackBufferDesc()
{
	return GetScBufDesc(_backBufferId);
}

D3D12_CPU_DESCRIPTOR_HANDLE Render::GetScBufDesc(UINT numBuf = 0)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	if (numBuf > 0)
		rtvHandle.Offset(numBuf, _rtvDescSize);
	return rtvHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE Render::GetDepthStencilBufDesc()
{
	return D3D12_CPU_DESCRIPTOR_HANDLE(_dsvHeap->GetCPUDescriptorHandleForHeapStart());
}
