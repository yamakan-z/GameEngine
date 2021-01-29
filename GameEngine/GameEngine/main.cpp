//システム系のヘッダーインクルード------------------
#include<Windows.h>
#include<D3D11.h>
#include<d3dCompiler.h>
#include"DirectXTex.h"
#include"WICTextureLoader11.h"


//Gameシステム用ヘッダー（自作）インクルード--------
#include "WindowCreate.h"
#include "DeviceCreate.h"

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



//ポリゴン表示で必要な変数----------
//GPUで扱う用
ID3D11VertexShader* g_pVertexShader;   //バーテックスシェーダー
ID3D11PixelShader*  g_pPixelShader;    //ピクセルシェーダー
ID3D11InputLayout*  g_pVertexLayout;   //頂点入力レイアウト
ID3D11Buffer*       g_pConstantBuffer; //コンスタントバッファ
//ポリゴン情報登録用バッファ
ID3D11Buffer*       g_pVertexBuffer;   //バーテックスバッファ
ID3D11Buffer*       g_pIndexBuffer;    //インデックスバッファ

//テクスチャに必要なもの
ID3D11SamplerState*       g_pSampleLinear;  //テクスチャーサンプラー
ID3D11ShaderResourceView* g_pTexture;       //テクスチャーリソース

//構造体----------------------
//頂点レイアウト構造体（頂点が持つ情報）
struct POINT_LAYOUT
{
    float pos[3];  //X-Y-Z  :頂点
    float color[4];//R-G-B-A:色
    float uv[2];   //U-V    :テクスチャ位置
};

//コンスタントバッファ構造体
struct POLYGON_BUFFER
{
    float color[4]; //R-G-B-A:ポリゴンカラー
};


//プロトタイプ変数
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);//ウィンドウプロジージャー
HRESULT InitPolygonRender();//ポリゴン表示環境の初期化
void DeletePolygonRender(); //ポリゴン表示環境の破棄

//Main関数--------------------
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

    //DirectX Deviceの初期化
    CDeviceCreate::InitDevice(CWindowCreate::GetWnd(), 800, 600);

    //ポリゴン表示環境の初期化
    InitPolygonRender();

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
        g_pDeviceContext->OMSetRenderTargets(1, &g_pRTV, NULL);//レンダリング先をカラーバッファ（バックバッファ）にセット
        g_pDeviceContext->ClearRenderTargetView(g_pRTV, color);//画面をcolorでクリア
        g_pDeviceContext->RSSetState(g_pRS);//ラスタライズをセット
        //ここからレンダリング開始

        //頂点レイアウト
        g_pDeviceContext->IASetInputLayout(g_pVertexLayout);

        //使用するシェーダーの登録
        g_pDeviceContext->VSSetShader(g_pVertexShader, NULL, 0);
        g_pDeviceContext->PSSetShader(g_pPixelShader, NULL, 0);

        //コンスタントバッファを使用するシェーダーに登録
        g_pDeviceContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
        g_pDeviceContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);

        //プリミティブ・トポロジーをセット
        g_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        //バーテックスバッファ登録
        UINT stride = sizeof(POINT_LAYOUT);
        UINT offset = 0;
        g_pDeviceContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

        //インデックスバッファ登録
        g_pDeviceContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

        //コンスタントバッファのデータ登録
        D3D11_MAPPED_SUBRESOURCE pData;
        if (SUCCEEDED(g_pDeviceContext->Map(g_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
        {
            POLYGON_BUFFER data;
            data.color[0] = 1.0f;
            data.color[1] = 1.0f;
            data.color[2] = 1.0f;
            data.color[3] = 1.0f;

            memcpy_s(pData.pData, pData.RowPitch, (void*)&data, sizeof(POLYGON_BUFFER));
            //コンスタントバッファをシェーダに輸送
            g_pDeviceContext->Unmap(g_pConstantBuffer, 0);
        }

        //テクスチャーサンプラを使用
        g_pDeviceContext->PSSetSamplers(0, 1, &g_pSampleLinear);
        //テクスチャを登録
        g_pDeviceContext->PSSetShaderResources(0,1,&g_pTexture);


        //登録した情報を元にポリゴンを描画
        g_pDeviceContext->DrawIndexed(6, 0, 0);

        //レンダリング終了
        g_pDXGISwapChain->Present(1, 0);//60FPSでバックバッファとプライマリバッファの交換

    } while (msg.message != WM_QUIT);

    DeletePolygonRender();////ポリゴン表示環境の破棄

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

//ポリゴン表示環境の初期化
HRESULT InitPolygonRender()
{
    HRESULT hr = S_OK;

    //hlslファイル名
    const wchar_t* hlsl_name = L"PolygonDraw.hlsl";

    //hlslファイルを読み込み　ブロブ作成　ブロブとはシェーダーの塊みたいなもの
    //XXシェーダーとして特徴を持たない。後で各種シェーダーとなる
    ID3DBlob* pCompiledShader = NULL;
    ID3DBlob* pErrors         = NULL;

    //ブロブからバーテックスシェーダーコンパイル
    hr = D3DCompileFromFile(hlsl_name, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "vs", "vs_4_0", 0, NULL, &pCompiledShader, &pErrors);

    if (FAILED(hr))
    {
        //エラーがある場合、ｃがデバッグ情報を持つ
        char* c = (char*)pErrors->GetBufferPointer();
        MessageBox(0, L"hlsl読み込み失敗1", NULL, MB_OK);
        SAFE_RELEASE(pErrors);
        return hr;
    }
    //コンパイルしたバーデックスシェーダーを元にインターフェースを作成
    hr = g_pDevice->CreateVertexShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(),
       NULL,&g_pVertexShader );
    if (FAILED(hr))
    {
        SAFE_RELEASE(pCompiledShader);
        MessageBox(0, L"バーテックスシェーダー作成失敗", NULL, MB_OK);
        return hr;
    }
    //頂点インプットレイアウトを定義
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"UV"   ,0,DXGI_FORMAT_R32G32_FLOAT,      0,28,D3D11_INPUT_PER_VERTEX_DATA,0},
    };
    UINT numElements = sizeof(layout) / sizeof(layout[0]);

    //頂点インプットレイアウトを作成・レイアウトをセット
    hr = g_pDevice->CreateInputLayout(layout, numElements, pCompiledShader->GetBufferPointer(),
        pCompiledShader->GetBufferSize(), & g_pVertexLayout);
    if (FAILED(hr))
    {
        MessageBox(0, L"レイアウト作成失敗", NULL, MB_OK);
        return hr;
    }
    SAFE_RELEASE(pCompiledShader);

    //ブロブからピクセルシェーダーコンパイル
    hr = D3DCompileFromFile(hlsl_name, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "ps", "ps_4_0", 0, NULL, &pCompiledShader, &pErrors);
    if (FAILED(hr))
    {
        //エラーがある場合、ｃがデバック情報を持つ
        char* c = (char*)pErrors->GetBufferPointer();
        MessageBox(0, L"hlsl読み込み失敗2", NULL, MB_OK);
        SAFE_RELEASE(pErrors);
        return hr;
    }
    //コンパイルしたピクセルシェーダーでインターフェースを作成
    hr = g_pDevice->CreatePixelShader(pCompiledShader->GetBufferPointer(),
        pCompiledShader->GetBufferSize(), NULL, &g_pPixelShader);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pCompiledShader);
        MessageBox(0, L"ピクセルシェーダー作成失敗", NULL, MB_OK);
    }
    SAFE_RELEASE(pCompiledShader);

  //三角ポリゴンの各頂点の情報
    POINT_LAYOUT vertices[] =
    {  //  x     y    z     r     g   b    a      u     v
        {{0.0f,0.0f,0.0f},{0.5f,0.5f,0.5f,1.0f},{0.0f,1.0f},},//頂点1
        {{0.5f,0.0f,0.0f},{0.5f,0.5f,0.5f,1.0f},{1.0f,1.0f},},//頂点2
        {{0.5f,0.5f,0.0f},{0.5f,0.5f,0.5f,1.0f},{1.0f,0.0f},},//頂点3
        {{0.0f,0.5f,0.0f},{0.5f,0.5f,0.5f,1.0f},{0.0f,0.0f},},//頂点4
    };
    //バッファにバーテックスステータス設定
    D3D11_BUFFER_DESC bd;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(POINT_LAYOUT) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;

    //バッファに入れるデータを設定
    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = vertices;

    //ステータスとバッファに入れるデータを元にバーテックスバッファ作成
    hr = g_pDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
    if (FAILED(hr))
    {
        MessageBox(0, L"バーテックスバッファ作成失敗", NULL, MB_OK);
        return hr;
    }
    //ポリゴンのインデックス情報
    unsigned short hIndexData[2][3] =
    {
        {0,1,2,},//1面
        {0,2,3,},//2面
    };

    //バッファにインデックスステータス設定
    D3D11_BUFFER_DESC hBufferDesc;
    hBufferDesc.ByteWidth = sizeof(hIndexData);
    hBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    hBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    hBufferDesc.CPUAccessFlags = 0;
    hBufferDesc.MiscFlags = 0;
    hBufferDesc.StructureByteStride = sizeof(unsigned short);

    //バッファに入れるデータを設定
    D3D11_SUBRESOURCE_DATA hSubResourceData;
    hSubResourceData.pSysMem = hIndexData;
    hSubResourceData.SysMemPitch = 0;
    hSubResourceData.SysMemSlicePitch = 0;

    //ステータスとバッファに入れるデータを元にインデックスバッファ作成
    hr = g_pDevice->CreateBuffer(&hBufferDesc, &hSubResourceData, &g_pIndexBuffer);
    if (FAILED(hr))
    {
        MessageBox(0, L"インデックスバッファ作成失敗", NULL, MB_OK);
        return hr;
    }

    //バッファにコンスタントバッファ（シェーダにデータ受け渡し用）ステータス作成
    D3D11_BUFFER_DESC cb;
    cb.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
    cb.ByteWidth           = sizeof(POLYGON_BUFFER);
    cb.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
    cb.MiscFlags           = 0;
    cb.StructureByteStride = 0;
    cb.Usage               = D3D11_USAGE_DYNAMIC;

    //ステータスを元にコンスタントバッファを作成
    hr = g_pDevice->CreateBuffer(&cb, NULL, &g_pConstantBuffer);
    if (FAILED(hr))
    {
        MessageBox(0, L"コンスタントバッファ作成失敗", NULL, MB_OK);
        return hr;
    }

    //テクスチャー用サンプラー作成
    D3D11_SAMPLER_DESC SamDesc;
    ZeroMemory(&SamDesc, sizeof(D3D11_SAMPLER_DESC));

    SamDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    SamDesc.BorderColor[0] = 0.0f;
    SamDesc.BorderColor[1] = 0.0f;
    SamDesc.BorderColor[2] = 0.0f;
    SamDesc.BorderColor[3] = 0.0f;
    SamDesc.MipLODBias = 0.0f;
    SamDesc.MaxAnisotropy = 2;
    SamDesc.MinLOD = 0.0f;
    SamDesc.MaxLOD = D3D11_FLOAT32_MAX;
    SamDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    g_pDevice->CreateSamplerState(&SamDesc, &g_pSampleLinear);

    //テクスチャー作成
    DirectX::CreateWICTextureFromFile(g_pDevice, g_pDeviceContext, L"Texture.png", nullptr, &g_pTexture, 0U);

    return hr;
}

//ポリゴン表示環境の破棄
void DeletePolygonRender()
{
    //テクスチャ情報の破棄
    SAFE_RELEASE(g_pSampleLinear);
    SAFE_RELEASE(g_pTexture);

    //GPU側で扱う用
    SAFE_RELEASE(g_pVertexShader);
    SAFE_RELEASE(g_pPixelShader);
    SAFE_RELEASE(g_pVertexLayout);

    //ポリゴン情報登録用バッファ
    SAFE_RELEASE(g_pConstantBuffer);
    SAFE_RELEASE(g_pVertexBuffer);
    SAFE_RELEASE(g_pIndexBuffer);
}


