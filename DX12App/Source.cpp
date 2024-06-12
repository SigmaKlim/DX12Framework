#include "Engine.h"
#include "Scene.h"

int main()
{
	Engine engine;
	engine.InitializeEngine();

	//Scene assembly
	Scene scene;
	std::vector<Vertex0> vs(3);
	vs[0].Position = { 0.0f, 0.5f, -1.0f };
	vs[1].Position = { -0.5f, -0.5f, -1.0f };
	vs[1].Position = { 0.5f, -0.5f, -1.0f };
	vs[0].Color = { 1.0f, 0.0f, 0.0f, 1.0f };
	vs[1].Color = { 0.0f, 1.0f, 0.0f, 1.0f };
	vs[2].Color = { 0.0f, 0.0f, 1.0f, 1.0f };

	ShaderInitData sid;
	sid.CompileOnlineFlags = 0;
	sid.FileNames = { L"..\\x64\\Debug\\VsSimple.cso", L"..\\x64\\Debug\\PsSimple.cso" };
	sid.EntryPointNames = { "", "" }; //no need as we use offline compilation

	scene.Models.push_back(Model0(vs, D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST, sid));
	engine.InitializeScene(&scene);
	//

	engine.BeginUpdateLoop();
}