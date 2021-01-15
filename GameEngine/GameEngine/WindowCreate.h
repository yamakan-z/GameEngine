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
	static HWND m_hWnd; //�E�B���h�E�n���h��
	static int  m_width; //�E�B���h�E�̉���
	static int  m_height;//�E�B���h�E�̉���
};