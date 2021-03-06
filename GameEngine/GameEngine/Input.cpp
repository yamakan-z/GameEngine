#include "Input.h"

int CInput::m_mou_x;  //マウスの位置X
int CInput::m_mou_y;  //マウスの位置Y

void CInput::InitInput()
{
	m_mou_x = 0;
	m_mou_y = 0;
}

//マウス取得用　ウィンドウプロジーシャー内に宣言
void CInput::SetMouPos(UINT* uMsg, LPARAM* lParam)
{
    switch (*uMsg)
    {
    case WM_MOUSEMOVE:     //マウスを操作した時
    {
        POINT point = { LOWORD(*lParam),HIWORD((*lParam)) };

        m_mou_x = point.x;   //カーソルのｘ座標
        m_mou_y = point.y;   //カーソルのｙ情報
    }
    break;
    }
}

//キー・マウスボタンのプッシュ確認
bool CInput::KeyPush(int key)
{
    if (GetAsyncKeyState(key) & 0x8000)
    {
        return true;
    }
    return false;
}