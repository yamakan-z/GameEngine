//STLデバッグ機能をOFFにする
#define _SECURE_SCL (0)
#define _HAS_ITERATOR_DEBUGGING (0)

//システム系のヘッダーインクルード------------------
#include<stdio.h>
#include<Windows.h>
#include<D3D11.h>
#include<d3dCompiler.h>
#include"DirectXTex.h"
#include"WICTextureLoader11.h"
#include <thread>
#include <atomic>



//Gameシステム用ヘッダー（自作）インクルード--------
#include "WindowCreate.h"
#include "DeviceCreate.h"
#include "Draw2DPolygon.h"
#include "Input.h"
#include "Audio.h"
#include "TaskSystem.h"
#include "FontTex.h"
#include "Collision.h"
#include "Render3D.h"

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
atomic<bool>g_ls_game_end = false; //スレッド用ゲーム終了フラグ
CMODEL* mod;                       //モデル情報を持つポインタ

//プロトタイプ変数
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);//ウィンドウプロジージャー

//マルチスレッドにする関数----------
//テクスチャ読み込み関数
unsigned __stdcall TextureLoadThread(void* p)
{
    //COM初期化
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    //イメージ読み込み
    CDraw2DPolygon::LoadImage(0, L"Texture.png");

    CoUninitialize();//COM解除
    return 0;
}

//ミュージック読み込み関数
unsigned __stdcall MusicLoadThread(void* p)
{
    //COM初期化
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    //ミュージック情報取得
    CAudio::LoadBackMusic("テスト.ogg");

    CoUninitialize();//COM解除
    return 0;
}

//ゲームメイン関数
unsigned __stdcall GameMainThread(void* p)
{

    //カラーバッファクリア色
    float color[] = { 0.0f,0.25f,0.45f,1.0f };
    //COM初期化
    //CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    while (1)
    {
        //アクション部分----------
        //ゲームメイン
        TaskSystem::ListAction();//リスト内のアクション実行
        //衝突判定実行
        Collision::CheckStart();

        //描画部分-----------
        //描画バッファ・Zバッファクリア
        Dev::GetDeviceContext()->ClearRenderTargetView(Dev::GetRTV(), color);//画面をcolorでクリア
        Dev::GetDeviceContext()->ClearDepthStencilView(Dev::GetDSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        //3D描画設定
        Dev::GetDeviceContext()->RSSetState(Dev::GetRS3D());//3D用ラスタライザーセット
        Dev::GetDeviceContext()->OMSetRenderTargets(1, Dev::GetppRTV(), Dev::GetDSV());//レンダリング先設定（カラーバッファ・Zバッファ）
        //ここから3D描画開始
        Render::Render(mod);

        //2D描画設定
        Dev::GetDeviceContext()->RSSetState(Dev::GetRS());//2D用ラスタライズをセット
        Dev::GetDeviceContext()->OMSetRenderTargets(1, Dev::GetppRTV(), NULL);//レンダリング先をカラーバッファ（バックバッファ）にセット
        //ここから2D描画開始

        //リスト内のドロー実行
        TaskSystem::ListDraw();
        Collision::DrawDebug();

        //レンダリング終了
        Dev::GetSwapChain()->Present(1, 0);//60FPSでバックバッファとプライマリバッファの交換

        if (g_ls_game_end==true)
        {
            break;
        }
    }
    
    //CoUninitialize();//COM解除
    return 0;
}

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
    CDeviceCreate::Init3DEnvironment();//3D環境構築
    CRender3D::Init();//3D描画環境作成
    Audio::InitAudio();//オーディオ作成
    Input::InitInput();//入力用のクラス初期化
    Draw::InitPolygonRender(); //ポリゴン表示環境の初期化
    TaskSystem::InitTaskSystem(); //タスクシステム初期化
    Font::InitFontTex();//フォント初期化
    Collision::InitHitBox();//コリジョンの初期化
    //リソース読み込み-------
    thread* thread_i = new thread(TextureLoadThread, nullptr);//テクスチャ読み込み
    thread* thread_m = new thread(MusicLoadThread, nullptr);//ミュージック読み込み
    //読み込みthread終了まで待機
    thread_i->join();
    thread_m->join();
    //メモリ開放
    delete thread_i;
    delete thread_m;
    
 
    //デバッグ用オブジェクト作成
    CHero* hero = new CHero();
    hero->m_priority = 90;
    TaskSystem::InsertObj(hero);

    hero = new CHero();
    hero->m_priority = 80;
    TaskSystem::InsertObj(hero);

    hero = new CHero();
    hero->m_priority = 70;
    TaskSystem::InsertObj(hero);

    TaskSystem::SortPriority();//描画順位変更

    //3Dモデル読み込み
    mod = new CMODEL();
    mod->CreateSampleTriangularpyramid();

    thread* thread_main = new thread(GameMainThread, nullptr);//ゲームメインスレッド開始
    //メッセージループ
    do
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

    } while (msg.message != WM_QUIT);
    thread_main->join();//ゲームメインスレッド終了待ち
    delete thread_main;

    //3Dモデル破棄
    delete mod;

    //ゲームシステム破棄

    TaskSystem::DeleteTaskSystem();//タスクシステムの破棄
    Collision::DeleteHitBox();//コリジョンの破棄
    Font::DeleteFontTex();//フォントの破棄
    Draw::DeletePolygonRender();////ポリゴン表示環境の破棄
    CRender3D::Delete();//3D描画環境の破棄
    CDeviceCreate::Delete3DEnvironment();//3D環境破棄
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
            g_ls_game_end = true;
            PostQuitMessage(0);
            break;
        }
        break;
    case WM_CLOSE://ウィンドウを閉じる場合
        g_ls_game_end = true;
        PostQuitMessage(0);
    case WM_DESTROY://終了する場合
        return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}



