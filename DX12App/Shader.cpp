#include "Shader.h"

Shader::Shader(bool compileOnline, const std::wstring& fileName, const std::string& entryPointName)
    : _compileOnline(compileOnline), _fileName(fileName), _entryPointName(entryPointName) {}
bool Shader::CompileOnline() const
{
    return _compileOnline;
}

const std::wstring& Shader::GetFileName() const
{
    return _fileName;
}

const BYTE* Shader::GetByteCode() const
{
    return (BYTE*)_byteCode->GetBufferPointer();
}

size_t Shader::GetByteCodeLength() const
{
    return _byteCode->GetBufferSize();
}

void Shader::GetShader(D3D12_SHADER_BYTECODE& shader)
{
    shader.pShaderBytecode = GetByteCode();
    shader.BytecodeLength = GetByteCodeLength();
}
