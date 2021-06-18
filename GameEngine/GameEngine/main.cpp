//システム系のヘッダーインクルード------------------
#include<stdio.h>
#include<Windows.h>
#include<D3D11.h>
#include<d3dCompiler.h>
#include"DirectXTex.h"
#include"WICTextureLoader11.h"



//Gameシステム用ヘッダー（自作）インクルード--------
#include "WindowCreate.h"
#include "DeviceCreate.h"
#include "Draw2DPolygon.h"
#include "Input.h"
#include "Audio.h"
#include "TaskSystem.h"
#include "FontTex.h"

//デバッグ用オブジェクトヘッダ--------------
#include"Hero.h"

//削除されていないメモリを出力にダンプする-----------
#include<crtdbg.h>
#ifdef _DEBUG
  #ifndef DBG_NEW
    #define DBG_NEW new(_NORMAL_BLOCK,__FILE__,__LINE__)

    #define new DBG_NEW
  #endif
#endif  // _DEBUG

//LIBの登録----------------
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dCompiler.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"XAudio2.lib")

//構造体----------------------------


//グローバル変数------

//プロトタイプ変数
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);//ウィンドウプロジージャー


//Main関数--------------------
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR szCmdLine, int nCmdshow)
{
    //メモリダンプ開始
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

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

    //ゲーム各システム初期化---
    CWindowCreate::NewWindow(800, 600, name, hInstance);//ウィンドウ作成
    CDeviceCreate::InitDevice(CWindowCreate::GetWnd(), 800, 600);//DirectX Deviceの初期化
    Audio::InitAudio();//オーディオ作成
    Input::InitInput();//入力用のクラス初期化
    Draw::InitPolygonRender(); //ポリゴン表示環境の初期化
    TaskSystem::InitTaskSystem(); //タスクシステム初期化
    Font::InitFontTex();//フォント初期化
    //リソース読み込み-------
    //ミュージック情報取得
    Audio::LoadBackMusic("テスト.ogg");
    Audio::StartLoopMusic();//バックミュージックスタート
   
    //イメージ読み込み
    Draw::LoadImage(0, L"Texture.png");//0番目に"Texture.png"を読み込み
 
    //デバッグ用オブジェクト作成
    CHero* hero = new CHero();
    TaskSystem::InsertObj(hero);
    hero = new CHero();
    TaskSystem::InsertObj(hero);
    hero = new CHero();
    TaskSystem::InsertObj(hero);

    //メッセージループ
    do
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        TaskSystem::ListAction();//リスト内のアクション実行

        //レンダリングターゲットセットとレンダリング画面クリア
        float color[] = { 0.0f,0.25f,0.45f,1.0f };
        Dev::GetDeviceContext()->OMSetRenderTargets(1, Dev::GetppRTV(), NULL);//レンダリング先をカラーバッファ（バックバッファ）にセット
        Dev::GetDeviceContext()->ClearRenderTargetView(Dev::GetRTV(), color);//画面をcolorでクリア
        Dev::GetDeviceContext()->RSSetState(Dev::GetRS());//ラスタライズをセット
        //ここからレンダリング開始
       
        TaskSystem::ListDraw();//リスト内のドロー実行
        Font::StrDraw(L"あ", 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

        //レンダリング終了
        Dev::GetSwapChain()->Present(1, 0);//60FPSでバックバッファとプライマリバッファの交換

    } while (msg.message != WM_QUIT);

    //ゲームシステム破棄
    Font::DeleteFontTex();//フォントの破棄
    TaskSystem::DeleteTaskSystem();//タスクシステムの破棄
    Draw::DeletePolygonRender();////ポリゴン表示環境の破棄
    CDeviceCreate::ShutDown();//DirectXの環境破棄
    Audio::DeleteAudio();//オーディオ環境の破棄

    CoUninitialize();//COMの終了命令,ColnitializeExを用いたら必ず使用する

    //この時点で解放されていないメモリの情報を表示
    _CrtDumpMemoryLeaks();
    return true;
    
    
}

//コールバック関数
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM  lParam)
{
    Input::SetMouPos(&uMsg, &lParam);

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



