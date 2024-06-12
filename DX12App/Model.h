#pragma once
#include <d3d12.h>
#include "d3dx12.h"
#include <vector>

#include "Vertex.h"
#include "VBuffer.h"
#include "Shader.h"

using namespace Microsoft::WRL;

template <typename VERTEX>
class Model
{

public:
	Model(const std::vector<VERTEX>& vertices, D3D12_PRIMITIVE_TOPOLOGY topology = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST, ShaderInitData shaderData = {});

	unsigned					GetNumVertices() const;
	size_t						GetVertexByteSize() const;
	D3D12_PRIMITIVE_TOPOLOGY	GetPrimitiveTopology() const;
	D3D12_INPUT_LAYOUT_DESC		GetInputLayoutDesc() const;
	D3D12_VERTEX_BUFFER_VIEW	GetVertexBufferView() const;

private:
	std::vector<VERTEX>			_vertices;
	D3D12_PRIMITIVE_TOPOLOGY	_topology;
	VBufferT<VERTEX>			_vBuf;
	Shader						_shaders[E_ShaderStage::COUNT];
	friend class Render;
};

typedef Model<Vertex0> Model0;

template<typename VERTEX>
inline Model<VERTEX>::Model(const std::vector<VERTEX>& vertices, D3D12_PRIMITIVE_TOPOLOGY topology, ShaderInitData shaderData) :
	_vertices(vertices), _topology(topology), _vBuf(eGpu)
{
	assert(COUNT < sizeof(shaderData.CompileOnlineFlags) * 8);
	for (int i = 0; i < E_ShaderStage::COUNT; i++)
	{
		bool compileOnline = shaderData.CompileOnlineFlags & (1 << i);
		_shaders[i] = Shader(compileOnline, shaderData.FileNames[i], shaderData.EntryPointNames[i]);
	}
}

template<typename VERTEX>
inline unsigned Model<VERTEX>::GetNumVertices() const
{
	return _vBuf.GetNumVertices();
}

template<typename VERTEX>
inline size_t Model<VERTEX>::GetVertexByteSize() const
{
	return _vBuf.GetVertexByteSize();
}

template<typename VERTEX>
inline D3D12_PRIMITIVE_TOPOLOGY Model<VERTEX>::GetPrimitiveTopology() const
{
	return _topology;
}

template<typename VERTEX>
inline D3D12_INPUT_LAYOUT_DESC Model<VERTEX>::GetInputLayoutDesc() const
{
	return _vBuf.GetInputLayoutDesc();
}

template<typename VERTEX>
inline D3D12_VERTEX_BUFFER_VIEW Model<VERTEX>::GetVertexBufferView() const
{
	return _vBuf.GetVertexBufferView();
}
