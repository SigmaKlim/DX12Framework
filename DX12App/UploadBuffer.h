#pragma once
#include <d3d12.h>
#include "d3dx12.h"
#include <wrl.h>
#include "Helper.h"
using namespace Microsoft::WRL;
enum E_UpdateBufferType
{
	eDefault,
	eConstant
};
template <typename T_Data>
class UploadBuffer
{
public:
	UploadBuffer() = default;
	void Initialize(	ID3D12Device* device,
				unsigned elementCount,
				E_UpdateBufferType bufferType)
	{
		_elementByteSize = bufferType == eConstant ? RenderHelper::CalculateCBufferByteSize(sizeof(T_Data)) : sizeof(T_Data);
		RND_ASSERT(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(_elementByteSize * elementCount),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			NULL,
			IID_PPV_ARGS(_uploadBuffer.GetAddressOf())));
		RND_ASSERT(_uploadBuffer->Map(0, NULL,
			reinterpret_cast<void**>(&_mappedData)));
	}
	UploadBuffer(const UploadBuffer& rhs) = delete;
	UploadBuffer& operator=(const UploadBuffer& rhs) = delete;
	~UploadBuffer()
	{
		if (_uploadBuffer != NULL)
			_uploadBuffer->Unmap(0, nullptr);

		_mappedData = NULL;
	}

	ID3D12Resource* GetResource() const
	{
		return _uploadBuffer.Get();
	}
	void CopyData(int elementIndex, const T_Data& data)
	{
		memcpy(&_mappedData[elementIndex * _elementByteSize],
			&data, sizeof(T_Data));
	}

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> _uploadBuffer = NULL;
	BYTE* _mappedData = NULL;
	unsigned _elementByteSize = 0;
};