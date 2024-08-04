#pragma once
#include <d3d12.h>
#include "d3dx12.h"
#include <vector>

#include "Vertex.h"
#include "VBuffer.h"
#include "Shader.h"
#include "Helper.h"
#include "UploadBuffer.h"
#include "Transform.h"

using namespace Microsoft::WRL;
using namespace DirectX::SimpleMath;

template <typename VERTEX>
class Model
{

public:
	Model(const std::vector<VERTEX>& vertices, D3D12_PRIMITIVE_TOPOLOGY topology = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST, ShaderInitData shaderData = {});
	Model(const Model& other) = delete;
	Model& operator=(const Model& other) = delete;
	unsigned					GetNumVertices() const;
	size_t						GetVertexByteSize() const;
	D3D12_PRIMITIVE_TOPOLOGY	GetPrimitiveTopology() const;
	D3D12_INPUT_LAYOUT_DESC		GetInputLayoutDesc() const;
	D3D12_VERTEX_BUFFER_VIEW	GetVertexBufferView() const;
	Vector3						GetPosition() const;
	Quaternion					GetRotation() const;
	Vector3						GetScale() const;
	//Checks if gpsDesc hash coincides with the cached hash. Modifies the cached hash on mismatch. (only model parameters of the desc are compared)
	bool ValidatePipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc);
private:
	std::vector<VERTEX>			_vertices;
	D3D12_PRIMITIVE_TOPOLOGY	_topology;
	VBufferT<VERTEX>			_vBuf;
	Shader						_shaders[E_ShaderStage::COUNT];
	UploadBuffer<ModelTransform> _transformCB;
	ComPtr<ID3D12RootSignature> _rootSignature;

	DirectX::SimpleMath::Vector3	_position = DirectX::SimpleMath::Vector3::Zero;
	DirectX::SimpleMath::Quaternion _rotation = DirectX::SimpleMath::Quaternion::Identity;
	DirectX::SimpleMath::Vector3	_scale = DirectX::SimpleMath::Vector3::One;

	std::size_t					_gpsHash; //only the model part
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
		_shaders[i] = Shader(compileOnline, shaderData.FilePaths[i], shaderData.EntryPointNames[i]);
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

template<typename VERTEX>
inline Vector3 Model<VERTEX>::GetPosition() const
{
	return _position;
}

template<typename VERTEX>
inline Quaternion Model<VERTEX>::GetRotation() const
{
	return _rotation;
}

template<typename VERTEX>
inline Vector3 Model<VERTEX>::GetScale() const
{
	return _scale;
}


template<typename VERTEX>
inline bool Model<VERTEX>::ValidatePipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc)
{
	//size_t hash = 0;
	//boost::hash_combine(hash, gpsDesc.InputLayout);
	//boost::hash_combine(hash, gpsDesc.PrimitiveTopologyType);
	//for (int i = 0; i < E_ShaderStage::SHADER_STAGES_MAX_NUM; i++)
	//{
	//	boost::hash_combine(hash, Helper::GetShader(gpsDesc, (E_ShaderStage)i).pShaderBytecode);
	//	boost::hash_combine(hash, Helper::GetShader(gpsDesc, (E_ShaderStage)i).BytecodeLength);
	//}
	//if (hash == _gpsHash)
	//	return true;
	//_gpsHash = hash;
	return true;
}
