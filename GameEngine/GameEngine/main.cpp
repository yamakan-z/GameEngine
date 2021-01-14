//�V�X�e���n�̃w�b�_�[�C���N���[�h------------------
#include<windows.h>
#include<D3D11.h>
#include<d3dCompiler.h>


//Game�V�X�e���p�w�b�_�[�i����j�C���N���[�h--------
#include "WindowCreate.h"

//�폜����Ă��Ȃ����������o�͂Ƀ_���v����-----------
#include<crtdbg.h>
#ifdef _DEBUG
  #ifndef DBG_NEW
    #define DBG_NEW new(_NORMAL_BLOCK,__FILE__,__LINE__)

    #define new DBG_NEW
  #endif
#endif  // _DEBUG

//LIB�̓o�^----------------
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dCompiler.lib")
#pragma comment(lib,"dxguid.lib")
//�������J���}�N��-----------
#define SAFE_DELETE(p)      {if(p){delete(p);        (p)=nullptr;}}
#define SAFE_DELETE_ARRAY(p){if(p){delete[](p);      (p)=nullptr;}}
#define SAFE_RELEASE(p)     {if(p){(p)->Release();   (p)=nullptr;}}

//DirectX�ɕK�v�ȕϐ�
ID3D11Device*             g_pDevice;              //   D3D11�f�o�C�X
ID3D11DeviceContext*      g_pDeviceContext;       //   D3D11�f�o�C�X�R���e�L�X�g
ID3D11RasterizerState*    g_pRS;                  //   D3D11���X�^�[���C�U�[
ID3D11RenderTargetView*   g_pRTV;                 //   D3D11�����_�[�^�[�Q�b�g
ID3D11BlendState*         g_pBlendState;          //   D3D11�u�����h�X�e�[�^�X
IDXGIAdapter*             g_pDXGIAdapter;         //   DXGI�A�_�v�^�[
IDXGIFactory*             g_pDXGIFactory;         //   DXGI�t�@�N�g���[
IDXGISwapChain*           g_pDXGISwapChain;       //   DXGI�X���b�v�`�F�[��
IDXGIOutput**             g_ppDXGIOutputArray;    //   DXGI�o�͌Q
UINT                      g_nDXGIOutputArraySize; //   DXGI�o�͌Q�T�C�Y
IDXGIDevice1*             g_pDXGIDevice;          //   DXGI�f�o�C�X
D3D_FEATURE_LEVEL         g_FeatureLevel;         //   D3D�@�\���x��


//�v���g�^�C�v�ϐ�
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);//�E�B���h�E�v���W�[�W���[
HRESULT APIENTRY InitDevice(HWND hWnd, int w, int h);//�f�o�C�X�̏�����

//Main�֐�
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR szCmdLine, int nCmdshow)
{
    //�������_���v�J�n
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    wchar_t name[] = { L"GameEngine" };//�E�B���h�E���^�C�g���l�[��
    MSG msg;                           //���b�Z�[�W�n���h��

    //�E�B���h�E�X�e�[�^�X
    WNDCLASSEX wcex = {
        sizeof(WNDCLASSEX),CS_HREDRAW | CS_VREDRAW,
        WndProc,0,0,hInstance,NULL,NULL,
        (HBRUSH)(COLOR_WINDOW + 1),NULL,name,NULL
    };

    //�E�B���h�E�N���X�쐬
    RegisterClassEx(&wcex);

    //�E�B���h�E�쐬
    CWindowCreate::NewWindow(800, 600, name, hInstance);

    //���b�Z�[�W���[�v
    do
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    } while (msg.message != WM_QUIT);

    //���̎��_�ŉ������Ă��Ȃ��������̏���\��
    _CrtDumpMemoryLeaks();
    return true;
    
    
}

//�R�[���o�b�N�֐�
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM  lParam)
{
    switch (uMsg)
    {
    case WM_KEYDOWN:  //ESC�L�[�ŏI��
        switch (wParam)
        {
        case VK_ESCAPE:
            PostQuitMessage(0);
            break;
        }
        break;
    case WM_CLOSE://�E�B���h�E�����ꍇ
        PostQuitMessage(0);
    case WM_DESTROY://�I������ꍇ
        return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//�f�o�C�X�̏�����
HRESULT APIENTRY InitDevice(HWND hWnd, int w, int h)
{

    HRESULT hr = S_OK;

    //�f�o�C�X�̃C���^�[�t�F�[�X
    ID3D11Device* pDevice = NULL;
    ID3D11DeviceContext* pDeviceContext = NULL;
    D3D_FEATURE_LEVEL featureLevel = (D3D_FEATURE_LEVEL)0;

    IDXGIDevice1* pDXGIDevice = NULL;
    IDXGIAdapter* pDXGIAdapter = NULL;
    IDXGIFactory* pDXGIFactory = NULL;
    IDXGISwapChain* pDXGISwapChain = NULL;
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

    //��������
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0,D3D_FEATURE_LEVEL_10_1,D3D_FEATURE_LEVEL_10_0 };


    //�f�o�C�X�̏�����
    hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, featureLevels,
        sizeof(featureLevels) / sizeof(D3D_FEATURE_LEVEL), D3D11_SDK_VERSION,
        &pDevice, &featureLevel, &pDeviceContext);

    if (FAILED(hr))
    {
        //�������Ɏ��s�����ꍇ�A�\�t�g�E�F�A�G�~�����[�g�����s
        hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_SOFTWARE, NULL, 0, featureLevels,
            sizeof(featureLevels) / sizeof(D3D_FEATURE_LEVEL), D3D11_SDK_VERSION,
            &pDevice, &featureLevel, &pDeviceContext);
        if (FAILED(hr))
            return hr;
    }

    //�f�o�C�X����C���^�[�t�F�[�X�𒊏o
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

    //���X�^���C�U�[�̐ݒ�
    D3D11_RASTERIZER_DESC drd =
    {
        D3D11_FILL_SOLID,  //�`�惂�[�h
        D3D11_CULL_NONE,   //�|���S���̕`�����D3D11_CULL_BACK
        true,              //�O�p�`�̖ʕ��� TRUE-�����
        0,                 //�s�N�Z���̉��Z�[�x��
        0.0f,              //�s�N�Z���̍ő�[�x�o�C�A�X
        0.0f,              //�w��s�N�Z���̃X���[�v�ɑ΂���X�J���[
        TRUE,              //�w��Ɋ�Â��ăO���b�s���O���邩
        FALSE,             //�V�U�[�Z�`�̓����O��L���ɂ��邩
        TRUE,              //�}���`�T���v�����O��L���ɂ��邩
        TRUE               //���̃A���`�G�C���A�X��L���ɂ��邩
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
    //��ʃ��[�h���
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
    //�A�E�g�v�b�g�z�����������
    g_ppDXGIOutputArray = ppDXGIOutputArray;
    g_nDXGIOutputArraySize = OutputCount;

    //�X���b�v�`�F�[���̏������Ɛ���
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
    //D3D11�C���^�[�t�F�[�X�̏����o��
    g_pDevice = pDevice;
    g_pDeviceContext = pDeviceContext;
    g_pDXGIAdapter = pDXGIAdapter;
    g_pDXGIFactory = pDXGIFactory;
    g_pDXGISwapChain = pDXGISwapChain;
    g_FeatureLevel = featureLevel;
    g_pDXGIDevice = pDXGIDevice;
    //�����_�����O�^�[�Q�b�g�𐶐�
    ID3D11RenderTargetView* pRTV = NULL;

    //�o�b�N�o�b�t�@���擾
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

    //�����_�����O�^�[�Q�b�g�𐶐�
    hr = pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRTV);
    //�o�b�N�o�b�t�@�J��
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
    //�r���[�|�[�g�̐ݒ�
    D3D11_VIEWPORT vp;
    vp.Width = (float)w;
    vp.Height = (float)h;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    pDeviceContext->RSSetViewports(1, &vp);

    //�u�����h�X�e�[�g�𐶐�
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

    //�u�����f�B���O
    g_pDeviceContext->OMSetBlendState(g_pBlendState, NULL, 0xFFFF);

    //�X�e�[�^�X�A�r���[�Ȃǂ���������
    g_pRS  = pRS;
    g_pRTV = pRTV;
    return hr;
}


