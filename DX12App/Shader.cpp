#include "Shader.h"

Shader::Shader(bool compileOnline, const std::wstring& fileName, const std::string& entryPointName)
    : _compileOnline(compileOnline), _fileName(fileName), _entryPointName(entryPointName) {}
bool Shader::CompileOnline() const
{
    return _compileOnline;
}

const std::wstring Shader::GetFileName() const
{
    return _fileName;
}
