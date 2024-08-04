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
#include "Camera.h"
//#define RND_ASSERT assert(_lastError == S_OK)

using namespace Microsoft::WRL;
using namespace DirectX::SimpleMath;

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
	void InitializeScene(Scene* scene);

	bool Draw();
private:
	void TestInit();
	void TestDraw();


	template<typename VERTEX>
	void InitializeModel(Model<VERTEX>& model, ID3D12Resource** uploadBuffer, D3D12_GRAPHICS_PIPELINE_STATE_DESC& gpsDesc);
	
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
	D3D12_CPU_DESCRIPTOR_HANDLE GetScreenBufDesc(UINT numBuf);
	D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilBufDesc();
	bool ValidatePipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& gpsDesc);

	void MakeProjMatrix(Matrix& matrix);

	unsigned _wndWidth, _wndHeight;

	DirectX::SimpleMath::Matrix proj;

	HWND _hWindow = HWND();

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
	ComPtr<ID3D12DescriptorHeap>		_cbvHeap;

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

	//render pass params
	D3D12_VIEWPORT				_viewportRect;
	D3D12_RECT					_scissorsRect;
	D3D12_RASTERIZER_DESC		_rasterizerState;
	D3D12_BLEND_DESC			_blendState;
	D3D12_DEPTH_STENCIL_DESC	_dsState;
	size_t						_gpsHash; //only render pass paramaters hash

	Scene* _scene;
	Camera camera;
};

template<typename VERTEX>
inline void Render::InitializeModel(Model<VERTEX>& model, ID3D12Resource** uploadBuffer, D3D12_GRAPHICS_PIPELINE_STATE_DESC& gpsDesc)
{
#pragma region Compile or load shaders
	InitializeVBuffer(model._vBuf, model._topology, model._vertices, uploadBuffer);
	for (int i = 0; i < E_ShaderStage::COUNT; i++)
		if (model._shaders[i]._compileOnline == true)
			CompileShader(model._shaders[i], (E_ShaderStage)i);
		else
			LoadShaderBinaryData(model._shaders[i]);
#pragma endregion
#pragma region Initialize transform constant buffer
	model._transformCB.Initialize(_device.Get(), 1, E_UpdateBufferType::eConstant);
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = model._transformCB.GetResource()->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = RenderHelper::CalculateCBufferByteSize(sizeof(ModelTransform));
	_device->CreateConstantBufferView(&cbvDesc, _cbvHeap->GetCPUDescriptorHandleForHeapStart());
#pragma endregion
#pragma region Create Root Signature
	//temp: root signature
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.NumParameters = 1;
	rootSignatureDesc.pParameters = slotRootParameter;
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pStaticSamplers = nullptr;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	ComPtr<ID3DBlob> signatureBlob;
	ComPtr<ID3DBlob> errorBlob;
	RND_ASSERT(D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&signatureBlob,
		&errorBlob));
	RND_ASSERT(_device->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(model._rootSignature.GetAddressOf())));
#pragma endregion
#pragma region Initialize Pipeline State
	gpsDesc.InputLayout = model.GetInputLayoutDesc();
	gpsDesc.PrimitiveTopologyType = RenderHelper::ToTopologyType(model.GetPrimitiveTopology());
	gpsDesc.VS.pShaderBytecode = (BYTE*)model._shaders[E_ShaderStage::eVertex].GetByteCode();
	gpsDesc.VS.BytecodeLength = model._shaders[E_ShaderStage::eVertex].GetByteCodeLength();
	gpsDesc.PS.pShaderBytecode = (BYTE*)model._shaders[E_ShaderStage::ePixel].GetByteCode();
	gpsDesc.PS.BytecodeLength = model._shaders[E_ShaderStage::ePixel].GetByteCodeLength();
	gpsDesc.pRootSignature = model._rootSignature.Get();
#pragma endregion
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
