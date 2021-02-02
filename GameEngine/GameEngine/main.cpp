//�V�X�e���n�̃w�b�_�[�C���N���[�h------------------
#include<Windows.h>
#include<D3D11.h>
#include<d3dCompiler.h>
#include"DirectXTex.h"
#include"WICTextureLoader11.h"


//Game�V�X�e���p�w�b�_�[�i����j�C���N���[�h--------
#include "WindowCreate.h"
#include "DeviceCreate.h"
#include "Draw2DPolygon.h"

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

//�v���g�^�C�v�ϐ�
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);//�E�B���h�E�v���W�[�W���[

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
    Draw::InitPolygonRender();

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
        Dev::GetDeviceContext()->OMSetRenderTargets(1, Dev::GetppRTV(), NULL);//�����_�����O����J���[�o�b�t�@�i�o�b�N�o�b�t�@�j�ɃZ�b�g
        Dev::GetDeviceContext()->ClearRenderTargetView(Dev::GetRTV(), color);//��ʂ�color�ŃN���A
        Dev::GetDeviceContext()->RSSetState(Dev::GetRS());//���X�^���C�Y���Z�b�g
        //�������烌���_�����O�J�n

        Draw::Draw2D(0,0,1.0f,1.0f);//�e�N�X�`���t���l�p�|���S���`��

        //�����_�����O�I��
        Dev::GetSwapChain()->Present(1, 0);//60FPS�Ńo�b�N�o�b�t�@�ƃv���C�}���o�b�t�@�̌���

    } while (msg.message != WM_QUIT);

    Draw::DeletePolygonRender();////�|���S���\�����̔j��

    CDeviceCreate::ShutDown();//DirectX�̊��j��

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



