#pragma once
#include "App.h"
#include "Render.h"
#include "Timer.h"
#include <string>
#include "Scene.h"
class Engine
{
public:
	Engine();
	bool InitializeEngine();
	void InitializeScene(Scene* scene);
	void BeginUpdateLoop();
private:
	unsigned	 _wndWidth = 800;
	unsigned	 _wndHeight = 600;
	const unsigned DESIRED_FPS = 60;

	std::wstring _wndTitle = L"DX12App";
	AppWindow	 _app;
	Render		 _render;
	Timer		 _timer;

	Scene*		_scene;
};