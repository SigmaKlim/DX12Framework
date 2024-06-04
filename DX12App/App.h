#pragma once
#include <Windows.h>
#include <string>

class AppWindow
{
public:
	AppWindow(unsigned wndWidth, unsigned wndHeight, const std::wstring& wndTitle, const std::wstring& wcName);
	~AppWindow();
	bool Initialize();
	bool ProcessMessages();
	HWND GetHWnd() const;
private:
	bool RegisterWndClass();
	bool CreateWnd();
	

	HINSTANCE _hInstance;
	HWND _hWindow;
	unsigned _wndWidth;
	unsigned _wndHeight;
	const std::wstring WND_TITLE;
	const std::wstring WND_CLASS_NAME;
	int _lastError;
};