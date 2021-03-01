//システム系のヘッダーインクルード------------------
#include<stdio.h>
#include<Windows.h>
#include<D3D11.h>
#include<d3dCompiler.h>
#include"DirectXTex.h"
#include"WICTextureLoader11.h"
#include<XAudio2.h>


//Gameシステム用ヘッダー（自作）インクルード--------
#include "WindowCreate.h"
#include "DeviceCreate.h"
#include "Draw2DPolygon.h"
#include "Input.h"

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
//RIFFファイルフォーマット
class ChunkInfo
{
public:
    ChunkInfo():Size(0),pData(nullptr){}
    unsigned int   Size;   //チャンクデータ部のサイズ
    unsigned char* pData;  //チャンクデータ部の先頭ポインタ
};


//グローバル変数------
IXAudio2*                g_pXAudio2;          //XAudio2オブジェクト
IXAudio2MasteringVoice*  g_pMasteringVoice;   //マスターボイス
ChunkInfo*               g_DataChunk;         //サウンド情報
unsigned char*           g_pResourceData;     //サウンドファイル情報を持つポインタ
IXAudio2SourceVoice*     g_pSourceVoice;      //サウンドボイスインターフェース
IXAudio2SubmixVoice*     g_pSFXSubmixVoice;   //サブミクスインターフェース


//プロトタイプ変数
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);//ウィンドウプロジージャー
void  InitAudio();
void  DeleteAudio();
WORD  GetWord(const unsigned char* pData);
DWORD GetDword(const unsigned char* pData);
ChunkInfo FindChunk(const unsigned char* pData, const char* pChunkName);


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

    //ウィンドウ作成
    CWindowCreate::NewWindow(800, 600, name, hInstance);

    //オーディオ作成
    InitAudio();

    //DirectX Deviceの初期化
    CDeviceCreate::InitDevice(CWindowCreate::GetWnd(), 800, 600);

    //ポリゴン表示環境の初期化
    Draw::InitPolygonRender();
    Draw::LoadImage(0, L"Texture.png");//0番目に"Texture.png"を読み込み
    Draw::LoadImage(1, L"tex1.png");
    Draw::LoadImage(2, L"tex2.png");
    Draw::LoadImage(3, L"tex3.png");

    //入力用のクラス初期化
    Input::InitInput();

    //メッセージループ
    do
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        //レンダリングターゲットセットとレンダリング画面クリア
        float color[] = { 0.0f,0.25f,0.45f,1.0f };
        Dev::GetDeviceContext()->OMSetRenderTargets(1, Dev::GetppRTV(), NULL);//レンダリング先をカラーバッファ（バックバッファ）にセット
        Dev::GetDeviceContext()->ClearRenderTargetView(Dev::GetRTV(), color);//画面をcolorでクリア
        Dev::GetDeviceContext()->RSSetState(Dev::GetRS());//ラスタライズをセット
        //ここからレンダリング開始
        static float x = 0.0f;

        //Aキーが押されたとき
        if (GetAsyncKeyState('A')==true)
        {
            x += 1.0f;
        }
        //システムキー「カーソルキー↑」が押されたとき
        if (GetAsyncKeyState(VK_UP)==true)
        {
            x += 1.0f;
        }
        //システムキー　マウス右クリック
        if (GetAsyncKeyState(VK_RBUTTON)==true)
        {
            x += 1.0f;
        }

        static float time = 0.0f;
        time += 1.0f;
        Draw::Draw2D(0,128+x,100,1.0f,1.0f,time);//テクスチャ付き四角ポリゴン描画
        Draw::Draw2D(1,550, 300);
        Draw::Draw2D(2,Input::GetMouX(), Input::GetMouY(), 1.0f, 1.0f,-time);
        Draw::Draw2D(3,256, 256+64, time);


        //レンダリング終了
        Dev::GetSwapChain()->Present(1, 0);//60FPSでバックバッファとプライマリバッファの交換

    } while (msg.message != WM_QUIT);

    Draw::DeletePolygonRender();////ポリゴン表示環境の破棄

    CDeviceCreate::ShutDown();//DirectXの環境破棄

    DeleteAudio();//オーディオ環境の破棄

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

//XAudio2によるAudio環境構築関数
void InitAudio()
{
    unsigned XAudio2CreateFlags = 0;

    //XAudio2インターフェース作成
    XAudio2Create(&g_pXAudio2, XAudio2CreateFlags);

    //マスターボイス作成
    g_pXAudio2->CreateMasteringVoice(&g_pMasteringVoice);

    //ミックスボイス作成
    g_pXAudio2->CreateSubmixVoice(&g_pSFXSubmixVoice, 1, 44100, 0, 0, 0, 0);

    //waveファイルのオープン
    FILE* fp;
    const wchar_t* FILENAME = L"SETrigger.wav";
    _wfopen_s(&fp,FILENAME, L"rb");

    //ファイルサイズを取得
    unsigned Size = 0;
    fseek(fp, 0, SEEK_END);
    Size = ftell(fp);
    g_pResourceData = new unsigned char[Size];

    //ファイルデータをメモリに移す
    fseek(fp, 0, SEEK_SET);
    fread(reinterpret_cast<char*>(g_pResourceData), Size, 1, fp);
    fclose(fp);

    //RIFFファイル解析
    WAVEFORMATEX WaveformatEx = { 0 };

    //RIFFデータの先頭アドレスとRIFFデータサイズを渡す
    ChunkInfo WaveChunk = FindChunk(g_pResourceData, "fmt");
    unsigned char* p = WaveChunk.pData;

    //wave情報取得
    WaveformatEx.wFormatTag = GetWord(p);
    p += sizeof(WORD);
    WaveformatEx.nChannels = GetWord(p);
    p += sizeof(WORD);
    WaveformatEx.nSamplesPerSec = GetDword(p);

}

