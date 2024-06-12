#pragma once
#include <initguid.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <wrl.h>
#include <dxgi1_4.h>
#include "VertexProvider.h"
#include "Scene.h"
#include "Model.h"
#include "Shader.h"

using namespace Microsoft::WRL;
class SBuffer;
class VBuffer;
template <typename VERTEX>
class VBufferT;

class Render
{
public:
	Render(unsigned wndWidth, unsigned wndHeight);
	~Render();
	bool InitializeRender(HWND hWindow);
	bool Draw();

	void InitializeScene(Scene* scene);

private:
	void TestInit();
	void TestDraw();


	template<typename VERTEX>
	void InitializeModel(Model<VERTEX>& model, ID3D12Resource** uploadBuffer);
	
	//Caller is responsible for releasing upload buffer after the target buffer has been created.
	template <typename VERTEX>
	void InitializeVBuffer(VBufferT<VERTEX>& buf, D3D12_PRIMITIVE_TOPOLOGY topology, const std::vector<VERTEX>& vertices, ID3D12Resource** uploadBuffer);

	void InitializeVBuffer(VBuffer& buf, D3D12_PRIMITIVE_TOPOLOGY topology, D3D12_INPUT_LAYOUT_DESC inputLayoutDesc, size_t vertexByteSize, UINT64 numVertices, const void* initData = NULL, ID3D12Resource** uploadBuffer = NULL);
	void InitializeSBuffer(SBuffer& buf, UINT64 byteSize, const void* initData = NULL, ID3D12Resource** uploadBuffer = NULL);
	
	void LoadShaderBinaryData(Shader& shader);
	void CompileShader(Shader& shader, E_ShaderStage stage);

	void FlushCommandQueue();
	ID3D12Resource* GetBackBuffer(); 
	D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferDesc();
	D3D12_CPU_DESCRIPTOR_HANDLE GetScBufDesc(UINT numBuf);
	D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilBufDesc();
	unsigned _wndWidth, _wndHeight;
	HWND _hWindow = HWND();

	HRESULT _lastError;

	ComPtr<ID3D12Debug>					_debugController;
	ComPtr<IDXGIFactory4>				_factory;
	ComPtr<ID3D12Device>				_device;
	ComPtr<ID3D12Fence>					_fence;
	ComPtr<ID3D12CommandQueue>			_commandQueue;
	ComPtr<ID3D12CommandAllocator>		_commandAlloc;
	ComPtr<ID3D12GraphicsCommandList>	_commandList;
	ComPtr<IDXGISwapChain>				_swapChain;
	ComPtr<ID3D12DescriptorHeap>		_rtvHeap; //note that we only render to back buffer, we will need to modify code when we add render to texture
	ComPtr<ID3D12DescriptorHeap>		_dsvHeap;

	static const unsigned SC_NUM_BUFFERS = 2;
	unsigned _backBufferId = 0;

	ComPtr<ID3D12Resource>				_scBuffers[SC_NUM_BUFFERS];
	ComPtr<ID3D12Resource>				_depthStencilBuffer;

	unsigned _rtvDescSize = 0;
	unsigned _dsvDescSize = 0;
	unsigned _cbsruavDescSize = 0;

	const DXGI_FORMAT BACKBUF_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
	const DXGI_FORMAT DEPTH_STENCIL_FORMAT = DXGI_FORMAT_D24_UNORM_S8_UINT;

	const float BB_CLEAR_COLOR[4] = { 0.5f, 0.5f, 0.5f, 1.0f };

	unsigned _4xMsaaQualityLevels = 0;
	bool _4xMsaaEnabled = false;
	const unsigned REFRESH_RATE = 60;


	UINT64 _currentFence = 0;

	D3D12_VIEWPORT	_viewportRect;
	D3D12_RECT		_scissorsRect;

	Scene* _scene;
};


template<typename VERTEX>
inline void Render::InitializeModel(Model<VERTEX>& model, ID3D12Resource** uploadBuffer)
{
	InitializeVBuffer(model._vBuf, model._topology, model._vertices, uploadBuffer);
	for (int i = 0; i < E_ShaderStage::COUNT; i++)
		if (model._shaders[i]._compileOnline == true)
			CompileShader(model._shaders[i], (E_ShaderStage)i);
		else
			LoadShaderBinaryData(model._shaders[i]);
}

template<typename VERTEX>
inline void Render::InitializeVBuffer(VBufferT<VERTEX>& buf, D3D12_PRIMITIVE_TOPOLOGY topology, const std::vector<VERTEX>& vertices, ID3D12Resource** uploadBuffer)
{
	InitializeVBuffer(	buf, 
						topology,
						VertexProvider<VERTEX>::GetVertexLayout(),
						VertexProvider<VERTEX>::GetVertexByteSize(),
						vertices.size(),
						vertices.data(),
						uploadBuffer);
}
