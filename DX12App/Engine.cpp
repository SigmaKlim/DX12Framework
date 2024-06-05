#include "Engine.h"
#include <cassert>
#include <Windows.h>
#include <iostream>

Engine::Engine() : _app(_wndWidth, _wndHeight, _wndTitle, L"MyWndClass"), _render(_wndWidth, _wndHeight)
{

}

bool Engine::Initialize()
{
	assert(_app.Initialize());
	assert(_render.Initialize(_app.GetHWnd()));
	return true;
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
