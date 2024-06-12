#pragma once
#include <d3d12.h>
#include "Vertex.h"

template <typename VERTEX>
class VertexProvider
{

};
template <>
class VertexProvider<Vertex0>
{
public:
	inline static D3D12_INPUT_LAYOUT_DESC GetVertexLayout()
	{
		static D3D12_INPUT_ELEMENT_DESC elemDesc[2] = {
			{
				"POSITION",
				0,
				DXGI_FORMAT_R32G32B32_FLOAT,
				0,
				0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
				0
			},
			{
				"COLOR",
				0,
				DXGI_FORMAT_R32G32B32A32_FLOAT,
				0,
				12,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
				0
			}
		};
		return { elemDesc, 2 };
	}
	inline static size_t GetVertexByteSize()
	{
		return sizeof(Vertex0);
	}
};

