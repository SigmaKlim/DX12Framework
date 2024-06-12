#include "VBuffer.h"

VBuffer::VBuffer(E_BufType bufType) : SBuffer(bufType) {}

size_t VBuffer::GetVertexByteSize() const
{
	return _vertexSize;
}

UINT64 VBuffer::GetNumVertices() const
{
	return _numVertices;
}

D3D12_PRIMITIVE_TOPOLOGY VBuffer::GetPrimitiveTopology() const
{
	return _topology;
}

D3D12_INPUT_LAYOUT_DESC VBuffer::GetInputLayoutDesc() const
{
	return _inputLayoutDesc;
}

D3D12_VERTEX_BUFFER_VIEW VBuffer::GetVertexBufferView() const
{
	return _vbView;
}
