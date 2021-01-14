//システム系のヘッダーインクルード------------------
#include<windows.h>
#include<D3D11.h>
#include<d3dCompiler.h>


//Gameシステム用ヘッダー（自作）インクルード--------
#include "WindowCreate.h"

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
//メモリ開放マクロ-----------
#define SAFE_DELETE(p)      {if(p){delete(p);        (p)=nullptr;}}
#define SAFE_DELETE_ARRAY(p){if(p){delete[](p);      (p)=nullptr;}}
#define SAFE_RELEASE(p)     {if(p){(p)->Release();   (p)=nullptr;}}

//DirectXに必要な変数
ID3D11Device*             g_pDevice;              //   D3D11デバイス
ID3D11DeviceContext*      g_pDeviceContext;       //   D3D11デバイスコンテキスト
ID3D11RasterizerState*    g_pRS;                  //   D3D11ラスターライザー
ID3D11RenderTargetView*   g_pRTV;                 //   D3D11レンダーターゲット
ID3D11BlendState*         g_pBlendState;          //   D3D11ブレンドステータス
IDXGIAdapter*             g_pDXGIAdapter;         //   DXGIアダプター
IDXGIFactory*             g_pDXGIFactory;         //   DXGIファクトリー
IDXGISwapChain*           g_pDXGISwapChain;       //   DXGIスワップチェーン
IDXGIOutput**             g_ppDXGIOutputArray;    //   DXGI出力群
UINT                      g_nDXGIOutputArraySize; //   DXGI出力群サイズ
IDXGIDevice1*             g_pDXGIDevice;          //   DXGIデバイス
D3D_FEATURE_LEVEL         g_FeatureLevel;         //   D3D機能レベル


//プロトタイプ変数
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);//ウィンドウプロジージャー
HRESULT APIENTRY InitDevice(HWND hWnd, int w, int h);//デバイスの初期化

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

//デバイスの初期化
HRESULT APIENTRY InitDevice(HWND hWnd, int w, int h)
{

    HRESULT hr = S_OK;

    //デバイスのインターフェース
    ID3D11Device* pDevice = NULL;
    ID3D11DeviceContext* pDeviceContext = NULL;
    D3D_FEATURE_LEVEL featureLevel = (D3D_FEATURE_LEVEL)0;

    IDXGIDevice1* pDXGIDevice = NULL;
    IDXGIAdapter* pDXGIAdapter = NULL;
    IDXGIFactory* pDXGIFactory = NULL;
    IDXGISwapChain* pDXGISwapChain = NULL;
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

    //初期化順
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0,D3D_FEATURE_LEVEL_10_1,D3D_FEATURE_LEVEL_10_0 };


    //デバイスの初期化
    hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, featureLevels,
        sizeof(featureLevels) / sizeof(D3D_FEATURE_LEVEL), D3D11_SDK_VERSION,
        &pDevice, &featureLevel, &pDeviceContext);

    if (FAILED(hr))
    {
        //初期化に失敗した場合、ソフトウェアエミュレートを試行
        hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_SOFTWARE, NULL, 0, featureLevels,
            sizeof(featureLevels) / sizeof(D3D_FEATURE_LEVEL), D3D11_SDK_VERSION,
            &pDevice, &featureLevel, &pDeviceContext);
        if (FAILED(hr))
            return hr;
    }

    //デバイスからインターフェースを抽出
    hr = pDevice->QueryInterface(__uuidof(IDXGIDevice1), (LPVOID*)&g_pDXGIDevice);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pDevice);
        SAFE_RELEASE(pDeviceContext);
        return hr;
    }
    hr = pDXGIDevice->GetAdapter(&pDXGIAdapter);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pDXGIDevice);
        SAFE_RELEASE(pDevice);
        SAFE_RELEASE(pDeviceContext);
        return hr;
    }
    hr = pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (LPVOID*)&pDXGIFactory);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pDXGIAdapter);
        SAFE_RELEASE(pDXGIDevice);
        SAFE_RELEASE(pDevice);
        SAFE_RELEASE(pDeviceContext);

        return hr;
    }

    //ラスタライザーの設定
    D3D11_RASTERIZER_DESC drd =
    {
        D3D11_FILL_SOLID,  //描画モード
        D3D11_CULL_NONE,   //ポリゴンの描画方向D3D11_CULL_BACK
        true,              //三角形の面方向 TRUE-左回り
        0,                 //ピクセルの加算深度数
        0.0f,              //ピクセルの最大深度バイアス
        0.0f,              //指定ピクセルのスロープに対するスカラー
        TRUE,              //指定に基づいてグリッピングするか
        FALSE,             //シザー短形力リングを有効にするか
        TRUE,              //マルチサンプリングを有効にするか
        TRUE               //線のアンチエイリアスを有効にするか
    };
    ID3D11RasterizerState* pRS = NULL;
    hr = pDevice->CreateRasterizerState(&drd, &pRS);

    if (FAILED(hr))
    {
        SAFE_RELEASE(pDXGIFactory);
        SAFE_RELEASE(pDXGIAdapter);
        SAFE_RELEASE(pDXGIDevice);
        SAFE_RELEASE(pDevice);
        SAFE_RELEASE(pDeviceContext);
        return hr;
    }
    //画面モードを列挙
    UINT OutputCount = 0;
    for (OutputCount = 0;;OutputCount++)
    {
        IDXGIOutput* pDXGIOutput = NULL;
        if (FAILED(pDXGIAdapter->EnumOutputs(OutputCount, &pDXGIOutput)))
            break;
        SAFE_RELEASE(pDXGIOutput);
    }
    IDXGIOutput** ppDXGIOutputArray = new IDXGIOutput * [OutputCount];
    if (ppDXGIOutputArray == NULL)
    {
        SAFE_RELEASE(pDXGIFactory);
        SAFE_RELEASE(pDXGIAdapter);
        SAFE_RELEASE(pDXGIDevice);
        SAFE_RELEASE(pDevice);
        SAFE_RELEASE(pDeviceContext);
        return E_OUTOFMEMORY;
    }
    for (UINT iOutput = 0;iOutput < OutputCount;iOutput++)
    {
        pDXGIAdapter->EnumOutputs(iOutput, ppDXGIOutputArray + iOutput);
    }
    //アウトプット配列を書きだし
    g_ppDXGIOutputArray = ppDXGIOutputArray;
    g_nDXGIOutputArraySize = OutputCount;

    //スワップチェーンの初期化と生成
    swapChainDesc.BufferDesc.Width = w;
    swapChainDesc.BufferDesc.Height = h;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.OutputWindow = hWnd;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    hr = pDXGIFactory->CreateSwapChain(pDevice, &swapChainDesc, &pDXGISwapChain);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pDXGIFactory);
        SAFE_RELEASE(pDXGIAdapter);
        SAFE_RELEASE(pDXGIDevice);
        SAFE_RELEASE(pDevice);
        SAFE_RELEASE(pDeviceContext);
        return hr;
    }
    //D3D11インターフェースの書き出し
    g_pDevice = pDevice;
    g_pDeviceContext = pDeviceContext;
    g_pDXGIAdapter = pDXGIAdapter;
    g_pDXGIFactory = pDXGIFactory;
    g_pDXGISwapChain = pDXGISwapChain;
    g_FeatureLevel = featureLevel;
    g_pDXGIDevice = pDXGIDevice;
    //レンダリングターゲットを生成
    ID3D11RenderTargetView* pRTV = NULL;

    //バックバッファを取得
    ID3D11Texture2D* pBackBuffer = NULL;
    hr = pDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pDXGISwapChain);
        SAFE_RELEASE(pDXGIFactory);
        SAFE_RELEASE(pDXGIAdapter);
        SAFE_RELEASE(pDevice);
        SAFE_RELEASE(pDeviceContext);
        return hr;
    }
    D3D11_TEXTURE2D_DESC BackBufferSurfaceDesc;
    pBackBuffer->GetDesc(&BackBufferSurfaceDesc);

    //レンダリングターゲットを生成
    hr = pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRTV);
    //バックバッファ開放
    SAFE_RELEASE(pBackBuffer);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pDXGISwapChain);
        SAFE_RELEASE(pDXGIFactory);
        SAFE_RELEASE(pDXGIAdapter);
        SAFE_RELEASE(pDevice);
        SAFE_RELEASE(pDeviceContext);
        return hr;
    }
    //ビューポートの設定
    D3D11_VIEWPORT vp;
    vp.Width = (float)w;
    vp.Height = (float)h;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    pDeviceContext->RSSetViewports(1, &vp);

    //ブレンドステートを生成
    D3D11_BLEND_DESC BlendDesc = { FALSE,FALSE };
    for (int i = 0;i < 8;i++)
    {
        BlendDesc.RenderTarget[i].BlendEnable           = TRUE;
        BlendDesc.RenderTarget[i].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
        BlendDesc.RenderTarget[i].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
        BlendDesc.RenderTarget[i].BlendOp               = D3D11_BLEND_OP_ADD;
        BlendDesc.RenderTarget[i].SrcBlendAlpha         = D3D11_BLEND_ONE;
        BlendDesc.RenderTarget[i].DestBlendAlpha        = D3D11_BLEND_ZERO;
        BlendDesc.RenderTarget[i].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
        BlendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    }
    g_pDevice->CreateBlendState(&BlendDesc, &g_pBlendState);

    //ブレンディング
    g_pDeviceContext->OMSetBlendState(g_pBlendState, NULL, 0xFFFF);

    //ステータス、ビューなどを書きだし
    g_pRS  = pRS;
    g_pRTV = pRTV;
    return hr;
}


