#include "Render.h"
#include <iostream>
#include <cassert>
#include <fstream>
#include <d3dcompiler.h>

#include "VBuffer.h"
#include "Helper.h"
#include "Hash.h"

using namespace DirectX::SimpleMath;

//#define RND_ASSERT assert(_lastError == S_OK)

VBuffer0 testVBuf(eGpu);

Render::Render(unsigned wndWidth, unsigned wndHeight) : _wndWidth(wndWidth), _wndHeight(wndHeight)
{
	MakeProjMatrix(proj);
}

Render::~Render()
{
	if (_device != NULL)
		FlushCommandQueue(); //Let the command queue execute all commands before closing the window.
}

bool Render::InitializeRender(HWND hWindow)
{
	
	_hWindow = hWindow;
#pragma region Enable Debug
	RND_ASSERT(D3D12GetDebugInterface(IID_PPV_ARGS(_debugController.GetAddressOf())));
	_debugController->EnableDebugLayer();
#pragma endregion
#pragma region Create Factory
	RND_ASSERT(CreateDXGIFactory1(IID_PPV_ARGS(_factory.GetAddressOf())));
	#pragma endregion
#pragma region Create Device
	RND_ASSERT(D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(_device.GetAddressOf())));
#pragma endregion
#pragma region Create Fence
	RND_ASSERT(_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_fence.GetAddressOf())));
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
	RND_ASSERT(_device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQLevels,
		sizeof(msQLevels)));
	_4xMsaaQualityLevels = msQLevels.NumQualityLevels;
	assert(_4xMsaaQualityLevels > 0);
#pragma endregion
#pragma region Create Command Objects
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	RND_ASSERT(_device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(_commandQueue.GetAddressOf())));
	RND_ASSERT(_device->CreateCommandAllocator(cmdQueueDesc.Type, IID_PPV_ARGS(_commandAlloc.GetAddressOf())));
	RND_ASSERT(_device->CreateCommandList(0,
		cmdQueueDesc.Type,
		_commandAlloc.Get(),
		NULL,
		IID_PPV_ARGS(_commandList.GetAddressOf())));
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
	RND_ASSERT(_factory->CreateSwapChain(_commandQueue.Get(), &swapDesc, _swapChain.GetAddressOf()));
#pragma endregion
#pragma region Create RTV and DSV Descriptor Heaps
	D3D12_DESCRIPTOR_HEAP_DESC cbvDesc;
	cbvDesc.NumDescriptors = 1;
	cbvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvDesc.NodeMask = 0;
	RND_ASSERT(_device->CreateDescriptorHeap(&cbvDesc, IID_PPV_ARGS(_cbvHeap.GetAddressOf())));
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SC_NUM_BUFFERS;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	RND_ASSERT(_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(_rtvHeap.GetAddressOf())));
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	RND_ASSERT(_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(_dsvHeap.GetAddressOf())));
#pragma endregion
#pragma region Create RTVs	
	for (unsigned i = 0; i < SC_NUM_BUFFERS; i++)
	{
		RND_ASSERT(_swapChain->GetBuffer(i, IID_PPV_ARGS(_scBuffers[i].GetAddressOf())));
		_device->CreateRenderTargetView(_scBuffers[i].Get(), NULL, GetScreenBufDesc(i)); //NULL for desc means description of the back buffer will be used
	}
#pragma endregion
#pragma region Create DSV and DS buffer
	D3D12_RESOURCE_DESC dsBufDesc;
	dsBufDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	dsBufDesc.Alignment = 0;
	dsBufDesc.Width = _wndWidth;
	dsBufDesc.Height = _wndHeight;
	dsBufDesc.DepthOrArraySize = 1;
	dsBufDesc.MipLevels = 1;
	dsBufDesc.Format = DEPTH_STENCIL_FORMAT;
	dsBufDesc.SampleDesc.Count = _4xMsaaEnabled ? 4 : 1;
	dsBufDesc.SampleDesc.Quality = _4xMsaaEnabled ? (_4xMsaaQualityLevels - 1) : 0;
	dsBufDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	dsBufDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	D3D12_CLEAR_VALUE dsClearVal;
	dsClearVal.Format = DEPTH_STENCIL_FORMAT;
	dsClearVal.DepthStencil.Depth = 1.0f;
	dsClearVal.DepthStencil.Stencil = 0;
	CD3DX12_HEAP_PROPERTIES heapP(D3D12_HEAP_TYPE_DEFAULT);
	RND_ASSERT(_device->CreateCommittedResource(	&heapP,
													D3D12_HEAP_FLAG_NONE,
													&dsBufDesc,
													D3D12_RESOURCE_STATE_COMMON,
													&dsClearVal,
													IID_PPV_ARGS(_depthStencilBuffer.GetAddressOf())));
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
#pragma region Initialize Rasterizer State
	_rasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//_rasterizerState.CullMode = D3D12_CULL_MODE_NONE;
#pragma endregion
#pragma region Initialize Blend State
	_blendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
#pragma endregion
#pragma region Initialize Depth Stencil State
	_dsState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
#pragma endregion
#pragma region Submit Command List

	RND_ASSERT(_commandList->Close());
	ID3D12CommandList* cmdLists[] = { _commandList.Get() };
	_commandQueue->ExecuteCommandLists(1, cmdLists);

	FlushCommandQueue();
#pragma endregion
	return true;
}

void Render::InitializeScene(Scene* scene)
{
	_commandList->Reset(_commandAlloc.Get(), NULL);
	_scene = scene;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};
	gpsDesc.BlendState = _blendState;
	gpsDesc.DepthStencilState = _dsState;
	gpsDesc.NumRenderTargets = 1;
	gpsDesc.RasterizerState = _rasterizerState;
	gpsDesc.RTVFormats[0] = BACKBUF_FORMAT;
	gpsDesc.DSVFormat = DEPTH_STENCIL_FORMAT;
	gpsDesc.SampleDesc.Count = _4xMsaaEnabled ? 4 : 1;
	gpsDesc.SampleDesc.Quality = _4xMsaaEnabled ? (_4xMsaaQualityLevels - 1) : 0;
	gpsDesc.SampleMask = UINT_MAX;
	std::vector<ComPtr<ID3D12Resource>> updateBuffers(_scene->_models.size());
	for (size_t i = 0; i < _scene->_models.size(); i++)
	{
		auto& model = _scene->_models[i];
		InitializeModel(*model, updateBuffers[i].GetAddressOf(), gpsDesc);
		RND_ASSERT(_device->CreateGraphicsPipelineState(&gpsDesc, IID_PPV_ARGS(_scene->_gpStates[i].GetAddressOf())));
	}
	RND_ASSERT(_commandList->Close());
	ID3D12CommandList* cmdLists[] = { _commandList.Get() };
	_commandQueue->ExecuteCommandLists(1, cmdLists);
	FlushCommandQueue();
}

void Render::InitializeVBuffer(VBuffer& buf, D3D12_PRIMITIVE_TOPOLOGY topology, D3D12_INPUT_LAYOUT_DESC inputLayoutDesc, size_t vertexByteSize, UINT64 numVertices, const void* initData, ID3D12Resource** uploadBuffer)
{
	buf._inputLayoutDesc = inputLayoutDesc;
	buf._vertexSize = vertexByteSize;
	buf._numVertices = numVertices;
	buf._topology = topology;
	InitializeSBuffer(buf, vertexByteSize * numVertices, initData, uploadBuffer);
	buf._vbView.BufferLocation = buf._buf->GetGPUVirtualAddress();
	buf._vbView.SizeInBytes = buf._byteSize;
	buf._vbView.StrideInBytes = buf._vertexSize;

}

void Render::InitializeSBuffer(SBuffer& buf, UINT64 byteSize, const void* initData, ID3D12Resource** uploadBuffer)
{
	assert(byteSize > 0);
	buf._byteSize = byteSize;
	assert(initData == NULL || buf._type != eGpu || uploadBuffer != NULL); //if we want to upload init data to a buffer which is not cpu-read-write, an upload buffer must be provided.
	HRESULT lastError;
	if (buf.GetType() == eGpu)
	{
		auto dHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto dBufDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
		RND_ASSERT(_device->CreateCommittedResource(
			&dHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&dBufDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(buf._buf.GetAddressOf())));
	}
	else
		assert(0); //currently only gpu buffers are accessible
	if (initData != NULL && buf._type == eGpu && uploadBuffer != NULL)
	{
		auto uHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto uBufDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
		RND_ASSERT(_device->CreateCommittedResource(
			&uHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&uBufDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadBuffer)));
		D3D12_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pData = initData;
		subResourceData.RowPitch = byteSize;
		subResourceData.SlicePitch = subResourceData.RowPitch;

		auto bufTrans1 = CD3DX12_RESOURCE_BARRIER::Transition(buf._buf.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
		_commandList->ResourceBarrier(1, &bufTrans1);
		auto bufTrans2 = CD3DX12_RESOURCE_BARRIER::Transition(buf._buf.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
		UpdateSubresources<1>(_commandList.Get(), buf._buf.Get(), *uploadBuffer, 0, 0, 1, &subResourceData);
		_commandList->ResourceBarrier(1, &bufTrans2);
	}
}

void Render::LoadShaderBinaryData(Shader& shader)
{
	std::ifstream fin(shader._fileName, std::ios::binary);
	assert(fin.is_open());
	fin.seekg(0, std::ios_base::end);
	std::ifstream::pos_type size = fin.tellg(); //assess file size
	fin.seekg(0, std::ios_base::beg);
	D3DCreateBlob((size_t)size, shader._byteCode.GetAddressOf());
	fin.read((char*)shader._byteCode->GetBufferPointer(), size);
	fin.close();
}

void Render::CompileShader(Shader& shader, E_ShaderStage stage)
{
	unsigned compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	ComPtr<ID3DBlob> errors;
	RND_ASSERT(D3DCompileFromFile(shader._fileName.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		shader._entryPointName.c_str(),
		Shader::ShaderVersions[stage],
		compileFlags,
		0,
		shader._byteCode.GetAddressOf(),
		errors.GetAddressOf()));
	
	if (errors != NULL)
		OutputDebugStringA((char*)errors->GetBufferPointer());
}

bool Render::Draw()
{
	RND_ASSERT(_commandAlloc->Reset());
	RND_ASSERT(_commandList->Reset(_commandAlloc.Get(), _scene->_gpStates[0].Get()));

	auto bbt1 = CD3DX12_RESOURCE_BARRIER::Transition(GetBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	_commandList->ResourceBarrier(1, &bbt1);
	_commandList->RSSetViewports(1, &_viewportRect);
	_commandList->RSSetScissorRects(1, &_scissorsRect);
	_commandList->ClearRenderTargetView(GetBackBufferDesc(), BB_CLEAR_COLOR, 0, NULL);
	_commandList->ClearDepthStencilView(GetDepthStencilBufDesc(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	auto bbd = GetBackBufferDesc();
	auto dsb = GetDepthStencilBufDesc();
	_commandList->OMSetRenderTargets(1, &bbd, true, &dsb);
	for (int i = 0; i < _scene->_models.size(); i++)
	{
		auto& model = _scene->_models[i];
		_commandList->IASetPrimitiveTopology(model->GetPrimitiveTopology());
		D3D12_VERTEX_BUFFER_VIEW rbVs[] = { model->GetVertexBufferView() };
		_commandList->IASetVertexBuffers(0, 1, rbVs);
		
		//Setup transform
		auto positionMatrix = Matrix::CreateTranslation(model->GetPosition());
		auto rotationMatrix = Matrix::CreateFromQuaternion(model->GetRotation());
		auto scaleMatrix = Matrix::CreateScale(model->GetScale());
		auto transform = scaleMatrix * rotationMatrix * positionMatrix;
		Matrix view;
		camera.MakeViewMatrix(view);
		model->_transformCB.CopyData(0, { transform * view * proj });

		_commandList->SetGraphicsRootSignature(model->_rootSignature.Get());
		CD3DX12_GPU_DESCRIPTOR_HANDLE cbv(_cbvHeap->GetGPUDescriptorHandleForHeapStart());
		_commandList->SetGraphicsRootDescriptorTable(0, cbv);
		_commandList->SetPipelineState(_scene->_gpStates[i].Get());
		_commandList->DrawInstanced(model->GetNumVertices(),	1, 0, 0);
	}
	auto bbt2 = CD3DX12_RESOURCE_BARRIER::Transition(GetBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	_commandList->ResourceBarrier(1, &bbt2);
	_backBufferId = (_backBufferId + 1 < SC_NUM_BUFFERS) ? _backBufferId + 1 : 0;

	RND_ASSERT(_commandList->Close());
	ID3D12CommandList* cmdLists[] = { _commandList.Get() };
	_commandQueue->ExecuteCommandLists(1, cmdLists);

	RND_ASSERT(_swapChain->Present(0, 0));

	FlushCommandQueue();
	return true;
}

void Render::TestInit()
{
	std::vector<Vertex0> vs(3);
	vs[0].Position = { 0.0f, 0.5f, -1.0f };
	vs[1].Position = { -0.5f, -0.5f, -1.0f };
	vs[1].Position = { 0.5f, -0.5f, -1.0f };
	vs[0].Color = { 1.0f, 0.0f, 0.0f, 1.0f };
	vs[1].Color = { 0.0f, 1.0f, 0.0f, 1.0f };
	vs[2].Color = { 0.0f, 0.0f, 1.0f, 1.0f };
	ComPtr<ID3D12Resource> ubuf;
	InitializeVBuffer(testVBuf, D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST, vs, ubuf.GetAddressOf());

	_commandList->Close();
	ID3D12CommandList* cmdLists[] = { _commandList.Get() };
	_commandQueue->ExecuteCommandLists(1, cmdLists);
	FlushCommandQueue();
	RND_ASSERT(_commandList->Reset(_commandAlloc.Get(), nullptr));
}

void Render::TestDraw()
{
	_commandList->IASetPrimitiveTopology(testVBuf.GetPrimitiveTopology());
	D3D12_VERTEX_BUFFER_VIEW rbVs[] = { testVBuf.GetVertexBufferView() };
	_commandList->IASetVertexBuffers(0, 1, rbVs);
	_commandList->DrawInstanced(testVBuf.GetNumVertices(), 1, 0, 0);
}

void Render::FlushCommandQueue()
{
	_currentFence++;
	RND_ASSERT(_commandQueue->Signal(_fence.Get(), _currentFence)); //Submit to GPU a command to increment fence value
	auto completedFence = _fence->GetCompletedValue();
	if (completedFence < _currentFence) //unnecessary???
	{
		HANDLE eventHandle = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);
		RND_ASSERT(_fence->SetEventOnCompletion(_currentFence, eventHandle)); //Assign the event to fire when fence reaches necessary value.
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
	return GetScreenBufDesc(_backBufferId);
}

D3D12_CPU_DESCRIPTOR_HANDLE Render::GetScreenBufDesc(UINT numBuf = 0)
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(  _rtvHeap->GetCPUDescriptorHandleForHeapStart(),
											numBuf,
											_rtvDescSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE Render::GetDepthStencilBufDesc()
{
	return D3D12_CPU_DESCRIPTOR_HANDLE(_dsvHeap->GetCPUDescriptorHandleForHeapStart());
}

bool Render::ValidatePipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& gpsDesc)
{
	/*std::size_t hash = 0;
	boost::hash_combine(hash, gpsDesc.pRootSignature);
	boost::hash_combine(hash, gpsDesc.DepthStencilState);
	boost::hash_combine(hash, gpsDesc.BlendState);
	boost::hash_combine(hash, gpsDesc.DSVFormat);
	boost::hash_combine(hash, gpsDesc.NumRenderTargets);
	boost::hash_combine(hash, gpsDesc.RasterizerState);*/

	return true;
}

void Render::MakeProjMatrix(Matrix& matrix)
{
	matrix = DirectX::SimpleMath::Matrix::CreatePerspectiveOffCenter(_wndWidth / -2.0f, _wndWidth / 2.0f, _wndHeight / -2.0f, _wndHeight / 2.0f, 1.0f, 1000.0f);
}
