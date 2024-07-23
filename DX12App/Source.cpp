#include <unordered_map>
#include <iostream>
#include "Engine.h"
#include "Scene.h"
#include "Helper.h"


//TODO:
//1) Implement GPS hashing so that GPS would not be changed if models have same shaders, etc.


int main(int argc, char** argv)
{
	ParseCommandLineArgs(argv);

	Engine engine;
	engine.InitializeEngine();

	//Scene assembly
	Scene scene;
	std::vector<Vertex0> vs(3);
	vs[0].Position = { 0.0f, 0.5f, 0.0f };
	vs[1].Position = { 0.5f, -0.5f, 0.0f };
	vs[2].Position = { -0.5f, -0.5f, 0.0f };
	vs[0].Color = { 1.0f, 0.0f, 0.0f, 1.0f };
	vs[1].Color = { 0.0f, 1.0f, 0.0f, 1.0f };
	vs[2].Color = { 0.0f, 0.0f, 1.0f, 1.0f };

	std::wstring projectDir = GeneralHelper::ToWstring(commandLineArgs["PROJECT_DIR"]);
	ShaderInitData sid;
	sid.CompileOnlineFlags = 3;
	sid.FilePaths = { projectDir + L"VsSimple.hlsl", projectDir + L"PsSimple.hlsl" };
	sid.EntryPointNames = { "main", "main" };
	
	scene.AddModel(Model0(vs, D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST, sid));
	engine.InitializeScene(&scene);
	//

	engine.BeginUpdateLoop();
}