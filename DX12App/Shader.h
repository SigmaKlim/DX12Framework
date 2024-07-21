#pragma once

#include <string>
#include <array>
#include <d3d12.h>
#include <wrl.h>
typedef unsigned char uint_8;

enum E_ShaderStage : uint_8
{
	eVertex,
	ePixel,

	COUNT,
};	

struct ShaderInitData
{
	uint_8 CompileOnlineFlags = 0; 
	std::array<std::wstring, E_ShaderStage::COUNT> FilePaths = {L"..\\x64\\Debug\\VsSimple.cso", L"..\\x64\\Debug\\PsSimple.cso"};
	std::array<std::string, E_ShaderStage::COUNT> EntryPointNames = { "main", "main" };
};

using namespace Microsoft::WRL;

class Shader
{
public:
	Shader() = default; // for array initialization
	Shader(bool compileOnline, const std::wstring& fileName, const std::string& entryPointName);
	bool CompileOnline() const;
	const std::wstring& GetFileName() const;
	const BYTE* GetByteCode() const;
	size_t GetByteCodeLength() const;
	void GetShader(D3D12_SHADER_BYTECODE& shader);
	static constexpr char* ShaderVersions[] =
	{
		"vs_5_1",	//eVertex
		"ps_5_1"	//ePixel
	};

private:
	bool _compileOnline = false;
	std::wstring _fileName; //hlsl or cso
	std::string _entryPointName;
	ComPtr<ID3DBlob> _byteCode;

	friend class Render;
};

