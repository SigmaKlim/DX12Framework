#pragma once
#include <d3d12.h>
#include "d3dx12.h"
#include <cassert>
#include <string>
#include "Shader.h"

using namespace Microsoft::WRL;

class RenderHelper
{
public:
    inline static D3D12_PRIMITIVE_TOPOLOGY_TYPE ToTopologyType(D3D12_PRIMITIVE_TOPOLOGY topology)
    {
        switch (topology)
        {
            case D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
            case D3D11_PRIMITIVE_TOPOLOGY_POINTLIST:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
            case D3D11_PRIMITIVE_TOPOLOGY_LINELIST:
            case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP:
            case D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ:
            case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
            case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
            case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
            case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ:
            case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            default:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
        }
    }

    inline static D3D12_SHADER_BYTECODE& GetShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC& gpsDesc, E_ShaderStage stage)
    {
        switch (stage)
        {
            case eVertex:
                return gpsDesc.VS;
            case ePixel:
                return gpsDesc.PS;
            
                //other shaders are not implemented yet
            default:
                assert(0);
        }
    }

    inline static size_t CalculateCBufferByteSize(size_t rawSize)
    {
        return (rawSize + 255) & ~255;
    }
};

class GeneralHelper
{
public:
    inline static std::wstring ToWstring(const std::string& str)
    {
        return std::wstring(str.begin(), str.end());
    }
};


#define RND_ASSERT(command)             \
if (command != S_OK)                    \
{                                       \
    std::cout << command << std::endl;  \
    assert(0);                          \
}                                       