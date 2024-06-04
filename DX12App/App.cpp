#include <cassert>
#include "App.h"

AppWindow::AppWindow(unsigned wndWidth, unsigned wndHeight, const std::wstring& wndTitle, const std::wstring& wcName)
	: _wndWidth(wndWidth), _wndHeight(wndHeight), WND_TITLE(wndTitle), WND_CLASS_NAME(wcName) 
{

}

AppWindow::~AppWindow()
{
	if (_hWindow != NULL)
	{
		UnregisterClass(WND_CLASS_NAME.c_str(), _hInstance);
		DestroyWindow(_hWindow);
	}
}

bool AppWindow::Initialize()
{
	assert(RegisterWndClass());
	assert(CreateWnd());
	return true;
}

bool AppWindow::RegisterWndClass()
{
	WNDCLASSEX wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = DefWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = _hInstance;
	wc.hIcon = NULL;
	wc.hIconSm = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WND_CLASS_NAME.c_str();
	wc.cbSize = sizeof(WNDCLASSEX);
	if (RegisterClassEx(&wc) == 0)
	{
		_lastError = GetLastError();
		return false;
	}
	return true;
}

bool AppWindow::CreateWnd()
{
	_hWindow = CreateWindowEx(0,
							WND_CLASS_NAME.c_str(),
							WND_TITLE.c_str(),
							WS_CAPTION | WS_MINIMIZE | WS_SYSMENU,
							0,
							0,
							_wndWidth,
							_wndHeight,
							NULL,
							NULL,
							_hInstance,
							NULL);
	if (_hWindow == 0)
	{
		_lastError = GetLastError();
		return false;
	}
	ShowWindow(_hWindow, SW_SHOW);
	SetForegroundWindow(_hWindow);
	SetFocus(_hWindow);
	return true;
}

bool AppWindow::ProcessMessages()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	if (PeekMessage(&msg, _hWindow, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	switch (msg.message)
	{
		case WM_NULL :
			if (!IsWindow(_hWindow))
			{
				_hWindow = NULL;
				UnregisterClass(WND_CLASS_NAME.c_str(), _hInstance);
				return false;
			}
		break;

	}
	return true;
}

HWND AppWindow::GetHWnd() const
{
	return _hWindow;
}
