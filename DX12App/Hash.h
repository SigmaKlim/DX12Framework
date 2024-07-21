#pragma once
#include <boost/container_hash/hash.hpp>
#include <d3d12.h>
inline std::size_t hash_value(D3D12_DEPTH_STENCILOP_DESC dsOp)
{
	size_t hash = 0;
	boost::hash_combine(hash, dsOp.StencilDepthFailOp);
	boost::hash_combine(hash, dsOp.StencilFailOp);
	boost::hash_combine(hash, dsOp.StencilFunc);
	boost::hash_combine(hash, dsOp.StencilPassOp);
	return hash;
}

inline std::size_t hash_value(D3D12_DEPTH_STENCIL_DESC dsDesc)
{
	size_t hash = 0;
	boost::hash_combine(hash, dsDesc.BackFace);
	boost::hash_combine(hash, dsDesc.FrontFace);
	boost::hash_combine(hash, dsDesc.DepthEnable);
	boost::hash_combine(hash, dsDesc.StencilEnable);
	boost::hash_combine(hash, dsDesc.DepthFunc);
	boost::hash_combine(hash, dsDesc.DepthWriteMask);
	boost::hash_combine(hash, dsDesc.StencilWriteMask);
	return hash;
}

inline std::size_t hash_value(D3D12_RENDER_TARGET_BLEND_DESC rtBlendDesc)
{
	size_t hash = 0;
	boost::hash_combine(hash, rtBlendDesc.BlendEnable);
	boost::hash_combine(hash, rtBlendDesc.BlendOp);
	boost::hash_combine(hash, rtBlendDesc.BlendOpAlpha);
	boost::hash_combine(hash, rtBlendDesc.DestBlend);
	boost::hash_combine(hash, rtBlendDesc.DestBlendAlpha);
	boost::hash_combine(hash, rtBlendDesc.LogicOp);
	boost::hash_combine(hash, rtBlendDesc.LogicOpEnable);
	boost::hash_combine(hash, rtBlendDesc.RenderTargetWriteMask);
	boost::hash_combine(hash, rtBlendDesc.SrcBlend);
	return hash;
}

inline size_t hash_value(D3D12_BLEND_DESC blendDesc)
{
	size_t hash = 0;
	boost::hash_combine(hash, blendDesc.AlphaToCoverageEnable);
	boost::hash_combine(hash, blendDesc.IndependentBlendEnable);
	boost::hash_range(hash, blendDesc.RenderTarget, blendDesc.RenderTarget + sizeof(blendDesc.RenderTarget));
	return hash;
}

inline size_t hash_value(D3D12_RASTERIZER_DESC blendDesc)
{

}