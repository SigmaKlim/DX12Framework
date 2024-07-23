#include "Engine.h"
#include <cassert>
#include <Windows.h>
#include <iostream>

std::unordered_map<std::string, std::string> commandLineArgs;
void ParseCommandLineArgs(char** argv)
{
#pragma region Project directory
	std::string exeDir = argv[0];
	auto cropPos = exeDir.find("x64\\Debug\\DX12App.exe");
	assert(cropPos != std::string::npos);
	commandLineArgs.insert({ "PROJECT_DIR", (exeDir.substr(0, cropPos) + "DX12App\\") });
#pragma endregion
}


Engine::Engine() : _app(_wndWidth, _wndHeight, _wndTitle, L"MyWndClass"), _render(_wndWidth, _wndHeight)
{

}

bool Engine::InitializeEngine()
{
	assert(_app.Initialize());
	assert(_render.InitializeRender(_app.GetHWnd()));
	return true;
}

void Engine::InitializeScene(Scene* scene)
{
	_scene = scene;
	_render.InitializeScene(scene);
}

void Engine::BeginUpdateLoop()
{
	while (_app.ProcessMessages())
	{
		_render.Draw();
		_timer.Tick();
		std::cout << _timer.GetDelta() << std::endl;
	}

}
