#include<windows.h>
#include"WindowCreate.h"

//削除されていないメモリを出力にダンプする
#include<crtdbg.h>
#ifdef _DEBUG
  #ifndef DBG_NEW
    #define DBG_NEW new(_NOMAL_BLOCK,__FILE__,__LINE__)

    #define new DBG_NEW
  #endif
#endif  // _DEBUG

//プロトタイプ変数
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM Param, LPARAM lParam);

//Main関数
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR szCmdLine, int nCmdshow)
{
    //メモリダンプ開始
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    wchar_t name[] = { L"GameEngine" };//ウィンドウ＆タイトルネーム
    MSG msg;                           //メッセージハンドル

    //ウィンドウステータス
    WNDCLASSEX wcex = {
        sizeof(WNDCLASSEX),CS_HREDRAW | CS_VREDRAW,
        WndProc,0,0,hInstance,NULL,NULL,
        (HBRUSH)(COLOR_WINDOW + 1),NULL,name,NULL
    };

    //ウィンドウクラス作成
    RegisterClassEx(&wcex);

    //ウィンドウ作成
    CWindowCreate::NewWindow(800, 600, name, hInstance);

    //メッセージループ
    do
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    } while (msg.message != WM_QUIT);

    //この時点で解放されていないメモリの情報を表示
    _CrtDumpMemoryLeaks();
    return true;
}

//コールバック関数
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM  lParam)
{
    switch (uMsg)
    {
    case WM_KEYDOWN:  //ESCキーで終了
        switch (wParam)
        {
        case VK_ESCAPE:
            PostQuitMessage(0);
            break;
        }
        break;
    case WM_CLOSE://ウィンドウを閉じる場合
        PostQuitMessage(0);
    case WM_DESTROY://終了する場合
        return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
