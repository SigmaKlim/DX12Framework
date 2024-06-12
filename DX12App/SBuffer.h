#pragma once
#include <d3d12.h>
#include "d3dx12.h"
#include <wrl.h>

using namespace Microsoft::WRL;
enum E_BufType
{
	eGpu
};

class SBuffer
{
public:
	SBuffer(E_BufType bufType);
	E_BufType GetType() const;
	//No methods below are allowed to be called before Initialization.
	ID3D12Resource* GetBuffer();
	UINT64 GetSize() const;
	
protected:
	E_BufType _type;
	UINT64 _byteSize;
	ComPtr<ID3D12Resource> _buf = nullptr;

	friend class Render;
};

