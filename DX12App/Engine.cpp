#include "Engine.h"
#include <cassert>
#include <Windows.h>
#include <iostream>

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
