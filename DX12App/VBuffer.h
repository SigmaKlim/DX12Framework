#pragma once
#include "SBuffer.h"

class VBuffer : public SBuffer
{
public:
	VBuffer(E_BufType bufType);
	//These methods can only be called after the buffer has been initialized.
	size_t GetVertexByteSize() const;
	UINT64 GetNumVertices() const;
	D3D12_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const;
	D3D12_INPUT_LAYOUT_DESC GetInputLayoutDesc() const;
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const;
	
private:
	size_t _vertexSize;
	UINT64 _numVertices;
	D3D12_PRIMITIVE_TOPOLOGY _topology;
	D3D12_INPUT_LAYOUT_DESC _inputLayoutDesc;
	D3D12_VERTEX_BUFFER_VIEW _vbView;

	friend class Render;
};

template <typename VERTEX>
class VBufferT : public VBuffer
{
public:
	VBufferT(E_BufType bufType);
};

template<typename VERTEX>
inline VBufferT<VERTEX>::VBufferT(E_BufType bufType) : VBuffer(bufType)
{
}

struct Vertex0;
typedef VBufferT<Vertex0> VBuffer0;