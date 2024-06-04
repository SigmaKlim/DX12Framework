#pragma once
#include <d3d12.h>
#include "d3dx12.h"
#include <wrl.h>
#include <dxgi1_4.h>
using namespace Microsoft::WRL;

class Render
{
public:
	Render(unsigned wndWidth, unsigned wndHeight);
	~Render();
	bool Initialize(HWND hWindow);
	bool Draw();
private:
	void AdvanceBackBuffer();
	void FlushCommandQueue();
	unsigned _wndWidth, _wndHeight;
	HWND _hWindow = HWND();

	HRESULT _lastError;

	ComPtr<ID3D12Debug>		_debugController;
	ComPtr<IDXGIFactory4>	_factory;
	ComPtr<ID3D12Device>	_device;
	ComPtr<ID3D12Fence>		_fence;
	ComPtr<ID3D12CommandQueue>			_commandQueue;
	ComPtr<ID3D12CommandAllocator>		_commandAlloc;
	ComPtr<ID3D12GraphicsCommandList>	_commandList;
	ComPtr<IDXGISwapChain>				_swapChain;
	ComPtr<ID3D12DescriptorHeap>	_rtvHeap;
	ComPtr<ID3D12DescriptorHeap>	_dsvHeap;
	ComPtr<ID3D12Resource>			_depthStencilBuffer;
	unsigned _rtvDescSize = 0;
	unsigned _dsvDescSize = 0;
	unsigned _cbsruavDescSize = 0;

	const DXGI_FORMAT BACKBUF_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
	const DXGI_FORMAT DEPTH_STENCIL_FORMAT = DXGI_FORMAT_D24_UNORM_S8_UINT;

	unsigned _4xMsaaQualityLevels = 0;
	bool _4xMsaaEnabled = false;
	const unsigned REFRESH_RATE = 60;

	static const unsigned SC_NUM_BUFFERS = 2;

	unsigned _backBufferId = 0;
	UINT64 _currentFence = 0;
};