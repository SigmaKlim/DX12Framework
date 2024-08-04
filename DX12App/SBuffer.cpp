#include "SBuffer.h"
#include <cassert>
//#define RND_ASSERT assert(lastError == S_OK)
SBuffer::SBuffer(E_BufType bufType) : _type(bufType)
{
}


ID3D12Resource* SBuffer::GetBuffer()
{
	return _buf.Get();
}


UINT64 SBuffer::GetSize() const
{
	return _byteSize;
}

E_BufType SBuffer::GetType() const
{
	return _type;
}
