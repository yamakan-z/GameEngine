//�V�X�e���n�̃w�b�_�[�C���N���[�h------------------
#include<Windows.h>
#include<D3D11.h>
#include<d3dCompiler.h>
#include"DirectXTex.h"
#include"WICTextureLoader11.h"


//Game�V�X�e���p�w�b�_�[�i����j�C���N���[�h--------
#include "WindowCreate.h"
#include "DeviceCreate.h"

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



//�|���S���\���ŕK�v�ȕϐ�----------
//GPU�ň����p
ID3D11VertexShader* g_pVertexShader;   //�o�[�e�b�N�X�V�F�[�_�[
ID3D11PixelShader*  g_pPixelShader;    //�s�N�Z���V�F�[�_�[
ID3D11InputLayout*  g_pVertexLayout;   //���_���̓��C�A�E�g
ID3D11Buffer*       g_pConstantBuffer; //�R���X�^���g�o�b�t�@
//�|���S�����o�^�p�o�b�t�@
ID3D11Buffer*       g_pVertexBuffer;   //�o�[�e�b�N�X�o�b�t�@
ID3D11Buffer*       g_pIndexBuffer;    //�C���f�b�N�X�o�b�t�@

//�e�N�X�`���ɕK�v�Ȃ���
ID3D11SamplerState*       g_pSampleLinear;  //�e�N�X�`���[�T���v���[
ID3D11ShaderResourceView* g_pTexture;       //�e�N�X�`���[���\�[�X

//�\����----------------------
//���_���C�A�E�g�\���́i���_�������j
struct POINT_LAYOUT
{
    float pos[3];  //X-Y-Z  :���_
    float color[4];//R-G-B-A:�F
    float uv[2];   //U-V    :�e�N�X�`���ʒu
};

//�R���X�^���g�o�b�t�@�\����
struct POLYGON_BUFFER
{
    float color[4]; //R-G-B-A:�|���S���J���[
};


//�v���g�^�C�v�ϐ�
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);//�E�B���h�E�v���W�[�W���[
HRESULT InitPolygonRender();//�|���S���\�����̏�����
void DeletePolygonRender(); //�|���S���\�����̔j��

//Main�֐�--------------------
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

    //DirectX Device�̏�����
    CDeviceCreate::InitDevice(CWindowCreate::GetWnd(), 800, 600);

    //�|���S���\�����̏�����
    InitPolygonRender();

    //���b�Z�[�W���[�v
    do
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        //�����_�����O�^�[�Q�b�g�Z�b�g�ƃ����_�����O��ʃN���A
        float color[] = { 0.0f,0.25f,0.45f,1.0f };
        g_pDeviceContext->OMSetRenderTargets(1, &g_pRTV, NULL);//�����_�����O����J���[�o�b�t�@�i�o�b�N�o�b�t�@�j�ɃZ�b�g
        g_pDeviceContext->ClearRenderTargetView(g_pRTV, color);//��ʂ�color�ŃN���A
        g_pDeviceContext->RSSetState(g_pRS);//���X�^���C�Y���Z�b�g
        //�������烌���_�����O�J�n

        //���_���C�A�E�g
        g_pDeviceContext->IASetInputLayout(g_pVertexLayout);

        //�g�p����V�F�[�_�[�̓o�^
        g_pDeviceContext->VSSetShader(g_pVertexShader, NULL, 0);
        g_pDeviceContext->PSSetShader(g_pPixelShader, NULL, 0);

        //�R���X�^���g�o�b�t�@���g�p����V�F�[�_�[�ɓo�^
        g_pDeviceContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
        g_pDeviceContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);

        //�v���~�e�B�u�E�g�|���W�[���Z�b�g
        g_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        //�o�[�e�b�N�X�o�b�t�@�o�^
        UINT stride = sizeof(POINT_LAYOUT);
        UINT offset = 0;
        g_pDeviceContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

        //�C���f�b�N�X�o�b�t�@�o�^
        g_pDeviceContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

        //�R���X�^���g�o�b�t�@�̃f�[�^�o�^
        D3D11_MAPPED_SUBRESOURCE pData;
        if (SUCCEEDED(g_pDeviceContext->Map(g_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
        {
            POLYGON_BUFFER data;
            data.color[0] = 1.0f;
            data.color[1] = 1.0f;
            data.color[2] = 1.0f;
            data.color[3] = 1.0f;

            memcpy_s(pData.pData, pData.RowPitch, (void*)&data, sizeof(POLYGON_BUFFER));
            //�R���X�^���g�o�b�t�@���V�F�[�_�ɗA��
            g_pDeviceContext->Unmap(g_pConstantBuffer, 0);
        }

        //�e�N�X�`���[�T���v�����g�p
        g_pDeviceContext->PSSetSamplers(0, 1, &g_pSampleLinear);
        //�e�N�X�`����o�^
        g_pDeviceContext->PSSetShaderResources(0,1,&g_pTexture);


        //�o�^�����������Ƀ|���S����`��
        g_pDeviceContext->DrawIndexed(6, 0, 0);

        //�����_�����O�I��
        g_pDXGISwapChain->Present(1, 0);//60FPS�Ńo�b�N�o�b�t�@�ƃv���C�}���o�b�t�@�̌���

    } while (msg.message != WM_QUIT);

    DeletePolygonRender();////�|���S���\�����̔j��

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

//�|���S���\�����̏�����
HRESULT InitPolygonRender()
{
    HRESULT hr = S_OK;

    //hlsl�t�@�C����
    const wchar_t* hlsl_name = L"PolygonDraw.hlsl";

    //hlsl�t�@�C����ǂݍ��݁@�u���u�쐬�@�u���u�Ƃ̓V�F�[�_�[�̉�݂����Ȃ���
    //XX�V�F�[�_�[�Ƃ��ē����������Ȃ��B��Ŋe��V�F�[�_�[�ƂȂ�
    ID3DBlob* pCompiledShader = NULL;
    ID3DBlob* pErrors         = NULL;

    //�u���u����o�[�e�b�N�X�V�F�[�_�[�R���p�C��
    hr = D3DCompileFromFile(hlsl_name, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "vs", "vs_4_0", 0, NULL, &pCompiledShader, &pErrors);

    if (FAILED(hr))
    {
        //�G���[������ꍇ�A�����f�o�b�O��������
        char* c = (char*)pErrors->GetBufferPointer();
        MessageBox(0, L"hlsl�ǂݍ��ݎ��s1", NULL, MB_OK);
        SAFE_RELEASE(pErrors);
        return hr;
    }
    //�R���p�C�������o�[�f�b�N�X�V�F�[�_�[�����ɃC���^�[�t�F�[�X���쐬
    hr = g_pDevice->CreateVertexShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(),
       NULL,&g_pVertexShader );
    if (FAILED(hr))
    {
        SAFE_RELEASE(pCompiledShader);
        MessageBox(0, L"�o�[�e�b�N�X�V�F�[�_�[�쐬���s", NULL, MB_OK);
        return hr;
    }
    //���_�C���v�b�g���C�A�E�g���`
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"UV"   ,0,DXGI_FORMAT_R32G32_FLOAT,      0,28,D3D11_INPUT_PER_VERTEX_DATA,0},
    };
    UINT numElements = sizeof(layout) / sizeof(layout[0]);

    //���_�C���v�b�g���C�A�E�g���쐬�E���C�A�E�g���Z�b�g
    hr = g_pDevice->CreateInputLayout(layout, numElements, pCompiledShader->GetBufferPointer(),
        pCompiledShader->GetBufferSize(), & g_pVertexLayout);
    if (FAILED(hr))
    {
        MessageBox(0, L"���C�A�E�g�쐬���s", NULL, MB_OK);
        return hr;
    }
    SAFE_RELEASE(pCompiledShader);

    //�u���u����s�N�Z���V�F�[�_�[�R���p�C��
    hr = D3DCompileFromFile(hlsl_name, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "ps", "ps_4_0", 0, NULL, &pCompiledShader, &pErrors);
    if (FAILED(hr))
    {
        //�G���[������ꍇ�A�����f�o�b�N��������
        char* c = (char*)pErrors->GetBufferPointer();
        MessageBox(0, L"hlsl�ǂݍ��ݎ��s2", NULL, MB_OK);
        SAFE_RELEASE(pErrors);
        return hr;
    }
    //�R���p�C�������s�N�Z���V�F�[�_�[�ŃC���^�[�t�F�[�X���쐬
    hr = g_pDevice->CreatePixelShader(pCompiledShader->GetBufferPointer(),
        pCompiledShader->GetBufferSize(), NULL, &g_pPixelShader);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pCompiledShader);
        MessageBox(0, L"�s�N�Z���V�F�[�_�[�쐬���s", NULL, MB_OK);
    }
    SAFE_RELEASE(pCompiledShader);

  //�O�p�|���S���̊e���_�̏��
    POINT_LAYOUT vertices[] =
    {  //  x     y    z     r     g   b    a      u     v
        {{0.0f,0.0f,0.0f},{0.5f,0.5f,0.5f,1.0f},{0.0f,1.0f},},//���_1
        {{0.5f,0.0f,0.0f},{0.5f,0.5f,0.5f,1.0f},{1.0f,1.0f},},//���_2
        {{0.5f,0.5f,0.0f},{0.5f,0.5f,0.5f,1.0f},{1.0f,0.0f},},//���_3
        {{0.0f,0.5f,0.0f},{0.5f,0.5f,0.5f,1.0f},{0.0f,0.0f},},//���_4
    };
    //�o�b�t�@�Ƀo�[�e�b�N�X�X�e�[�^�X�ݒ�
    D3D11_BUFFER_DESC bd;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(POINT_LAYOUT) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;

    //�o�b�t�@�ɓ����f�[�^��ݒ�
    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = vertices;

    //�X�e�[�^�X�ƃo�b�t�@�ɓ����f�[�^�����Ƀo�[�e�b�N�X�o�b�t�@�쐬
    hr = g_pDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
    if (FAILED(hr))
    {
        MessageBox(0, L"�o�[�e�b�N�X�o�b�t�@�쐬���s", NULL, MB_OK);
        return hr;
    }
    //�|���S���̃C���f�b�N�X���
    unsigned short hIndexData[2][3] =
    {
        {0,1,2,},//1��
        {0,2,3,},//2��
    };

    //�o�b�t�@�ɃC���f�b�N�X�X�e�[�^�X�ݒ�
    D3D11_BUFFER_DESC hBufferDesc;
    hBufferDesc.ByteWidth = sizeof(hIndexData);
    hBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    hBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    hBufferDesc.CPUAccessFlags = 0;
    hBufferDesc.MiscFlags = 0;
    hBufferDesc.StructureByteStride = sizeof(unsigned short);

    //�o�b�t�@�ɓ����f�[�^��ݒ�
    D3D11_SUBRESOURCE_DATA hSubResourceData;
    hSubResourceData.pSysMem = hIndexData;
    hSubResourceData.SysMemPitch = 0;
    hSubResourceData.SysMemSlicePitch = 0;

    //�X�e�[�^�X�ƃo�b�t�@�ɓ����f�[�^�����ɃC���f�b�N�X�o�b�t�@�쐬
    hr = g_pDevice->CreateBuffer(&hBufferDesc, &hSubResourceData, &g_pIndexBuffer);
    if (FAILED(hr))
    {
        MessageBox(0, L"�C���f�b�N�X�o�b�t�@�쐬���s", NULL, MB_OK);
        return hr;
    }

    //�o�b�t�@�ɃR���X�^���g�o�b�t�@�i�V�F�[�_�Ƀf�[�^�󂯓n���p�j�X�e�[�^�X�쐬
    D3D11_BUFFER_DESC cb;
    cb.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
    cb.ByteWidth           = sizeof(POLYGON_BUFFER);
    cb.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
    cb.MiscFlags           = 0;
    cb.StructureByteStride = 0;
    cb.Usage               = D3D11_USAGE_DYNAMIC;

    //�X�e�[�^�X�����ɃR���X�^���g�o�b�t�@���쐬
    hr = g_pDevice->CreateBuffer(&cb, NULL, &g_pConstantBuffer);
    if (FAILED(hr))
    {
        MessageBox(0, L"�R���X�^���g�o�b�t�@�쐬���s", NULL, MB_OK);
        return hr;
    }

    //�e�N�X�`���[�p�T���v���[�쐬
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

    //�e�N�X�`���[�쐬
    DirectX::CreateWICTextureFromFile(g_pDevice, g_pDeviceContext, L"Texture.png", nullptr, &g_pTexture, 0U);

    return hr;
}

//�|���S���\�����̔j��
void DeletePolygonRender()
{
    //�e�N�X�`�����̔j��
    SAFE_RELEASE(g_pSampleLinear);
    SAFE_RELEASE(g_pTexture);

    //GPU���ň����p
    SAFE_RELEASE(g_pVertexShader);
    SAFE_RELEASE(g_pPixelShader);
    SAFE_RELEASE(g_pVertexLayout);

    //�|���S�����o�^�p�o�b�t�@
    SAFE_RELEASE(g_pConstantBuffer);
    SAFE_RELEASE(g_pVertexBuffer);
    SAFE_RELEASE(g_pIndexBuffer);
}


