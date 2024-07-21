#include "Engine.h"
#include "Scene.h"


//TODO:
//1) Implement GPS hashing so that GPS would not be changed if models have same shaders, etc.


int main()
{
	Engine engine;
	engine.InitializeEngine();

	//Scene assembly
	Scene scene;
	std::vector<Vertex0> vs(3);
	vs[0].Position = { 0.0f, 0.5f, 0.0f };
	vs[1].Position = { -0.5f, -0.5f, 0.0f };
	vs[2].Position = { 0.5f, -0.5f, 0.0f };
	vs[0].Color = { 1.0f, 0.0f, 0.0f, 1.0f };
	vs[1].Color = { 0.0f, 1.0f, 0.0f, 1.0f };
	vs[2].Color = { 0.0f, 0.0f, 1.0f, 1.0f };

	ShaderInitData sid;
	sid.CompileOnlineFlags = 3;
	sid.FilePaths = { L"X:\\CodingProjects\\DX12Framework\\DX12App\\VsSimple.hlsl", L"X:\\CodingProjects\\DX12Framework\\DX12App\\PsSimple.hlsl" };
	sid.EntryPointNames = { "main", "main" };
	
	scene.AddModel(Model0(vs, D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST, sid));
	engine.InitializeScene(&scene);
	//

	engine.BeginUpdateLoop();
}