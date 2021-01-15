#pragma once
#include<Windows.h>

class CWindowCreate
{
public:
	CWindowCreate(){}
	~CWindowCreate(){}

	static void NewWindow(int w, int h, wchar_t* name, HINSTANCE hInstance);
	static HWND GetWnd() { return m_hWnd; }
private:
	static HWND m_hWnd; //ウィンドウハンドル
	static int  m_width; //ウィンドウの横幅
	static int  m_height;//ウィンドウの横幅
};