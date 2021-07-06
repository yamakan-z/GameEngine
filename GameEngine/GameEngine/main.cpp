//STL�f�o�b�O�@�\��OFF�ɂ���
#define _SECURE_SCL (0)
#define _HAS_ITERATOR_DEBUGGING (0)

//�V�X�e���n�̃w�b�_�[�C���N���[�h------------------
#include<stdio.h>
#include<Windows.h>
#include<D3D11.h>
#include<d3dCompiler.h>
#include"DirectXTex.h"
#include"WICTextureLoader11.h"
#include <thread>
#include <atomic>



//Game�V�X�e���p�w�b�_�[�i����j�C���N���[�h--------
#include "WindowCreate.h"
#include "DeviceCreate.h"
#include "Draw2DPolygon.h"
#include "Input.h"
#include "Audio.h"
#include "TaskSystem.h"
#include "FontTex.h"
#include "Collision.h"
#include "Render3D.h"

//�f�o�b�O�p�I�u�W�F�N�g�w�b�_--------------
#include"Hero.h"

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
#pragma comment(lib,"XAudio2.lib")

//�\����----------------------------


//�O���[�o���ϐ�------
atomic<bool>g_ls_game_end = false; //�X���b�h�p�Q�[���I���t���O
CMODEL* mod;                       //���f���������|�C���^

//�v���g�^�C�v�ϐ�
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);//�E�B���h�E�v���W�[�W���[

//�}���`�X���b�h�ɂ���֐�----------
//�e�N�X�`���ǂݍ��݊֐�
unsigned __stdcall TextureLoadThread(void* p)
{
    //COM������
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    //�C���[�W�ǂݍ���
    CDraw2DPolygon::LoadImage(0, L"Texture.png");

    CoUninitialize();//COM����
    return 0;
}

//�~���[�W�b�N�ǂݍ��݊֐�
unsigned __stdcall MusicLoadThread(void* p)
{
    //COM������
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    //�~���[�W�b�N���擾
    CAudio::LoadBackMusic("�e�X�g.ogg");

    CoUninitialize();//COM����
    return 0;
}

//�Q�[�����C���֐�
unsigned __stdcall GameMainThread(void* p)
{

    //�J���[�o�b�t�@�N���A�F
    float color[] = { 0.0f,0.25f,0.45f,1.0f };
    //COM������
    //CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    while (1)
    {
        //�A�N�V��������----------
        //�Q�[�����C��
        TaskSystem::ListAction();//���X�g���̃A�N�V�������s
        //�Փ˔�����s
        Collision::CheckStart();

        //�`�敔��-----------
        //�`��o�b�t�@�EZ�o�b�t�@�N���A
        Dev::GetDeviceContext()->ClearRenderTargetView(Dev::GetRTV(), color);//��ʂ�color�ŃN���A
        Dev::GetDeviceContext()->ClearDepthStencilView(Dev::GetDSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        //3D�`��ݒ�
        Dev::GetDeviceContext()->RSSetState(Dev::GetRS3D());//3D�p���X�^���C�U�[�Z�b�g
        Dev::GetDeviceContext()->OMSetRenderTargets(1, Dev::GetppRTV(), Dev::GetDSV());//�����_�����O��ݒ�i�J���[�o�b�t�@�EZ�o�b�t�@�j
        //��������3D�`��J�n
        Render::Render(mod);

        //2D�`��ݒ�
        Dev::GetDeviceContext()->RSSetState(Dev::GetRS());//2D�p���X�^���C�Y���Z�b�g
        Dev::GetDeviceContext()->OMSetRenderTargets(1, Dev::GetppRTV(), NULL);//�����_�����O����J���[�o�b�t�@�i�o�b�N�o�b�t�@�j�ɃZ�b�g
        //��������2D�`��J�n

        //���X�g���̃h���[���s
        TaskSystem::ListDraw();
        Collision::DrawDebug();

        //�����_�����O�I��
        Dev::GetSwapChain()->Present(1, 0);//60FPS�Ńo�b�N�o�b�t�@�ƃv���C�}���o�b�t�@�̌���

        if (g_ls_game_end==true)
        {
            break;
        }
    }
    
    //CoUninitialize();//COM����
    return 0;
}

//Main�֐�--------------------
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR szCmdLine, int nCmdshow)
{
    //�������_���v�J�n
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

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

    //�Q�[���e�V�X�e��������---
    CWindowCreate::NewWindow(800, 600, name, hInstance);//�E�B���h�E�쐬
    CDeviceCreate::InitDevice(CWindowCreate::GetWnd(), 800, 600);//DirectX Device�̏�����
    CDeviceCreate::Init3DEnvironment();//3D���\�z
    CRender3D::Init();//3D�`����쐬
    Audio::InitAudio();//�I�[�f�B�I�쐬
    Input::InitInput();//���͗p�̃N���X������
    Draw::InitPolygonRender(); //�|���S���\�����̏�����
    TaskSystem::InitTaskSystem(); //�^�X�N�V�X�e��������
    Font::InitFontTex();//�t�H���g������
    Collision::InitHitBox();//�R���W�����̏�����
    //���\�[�X�ǂݍ���-------
    thread* thread_i = new thread(TextureLoadThread, nullptr);//�e�N�X�`���ǂݍ���
    thread* thread_m = new thread(MusicLoadThread, nullptr);//�~���[�W�b�N�ǂݍ���
    //�ǂݍ���thread�I���܂őҋ@
    thread_i->join();
    thread_m->join();
    //�������J��
    delete thread_i;
    delete thread_m;
    
 
    //�f�o�b�O�p�I�u�W�F�N�g�쐬
    CHero* hero = new CHero();
    hero->m_priority = 90;
    TaskSystem::InsertObj(hero);

    hero = new CHero();
    hero->m_priority = 80;
    TaskSystem::InsertObj(hero);

    hero = new CHero();
    hero->m_priority = 70;
    TaskSystem::InsertObj(hero);

    TaskSystem::SortPriority();//�`�揇�ʕύX

    //3D���f���ǂݍ���
    mod = new CMODEL();
    mod->CreateSampleTriangularpyramid();

    thread* thread_main = new thread(GameMainThread, nullptr);//�Q�[�����C���X���b�h�J�n
    //���b�Z�[�W���[�v
    do
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

    } while (msg.message != WM_QUIT);
    thread_main->join();//�Q�[�����C���X���b�h�I���҂�
    delete thread_main;

    //3D���f���j��
    delete mod;

    //�Q�[���V�X�e���j��

    TaskSystem::DeleteTaskSystem();//�^�X�N�V�X�e���̔j��
    Collision::DeleteHitBox();//�R���W�����̔j��
    Font::DeleteFontTex();//�t�H���g�̔j��
    Draw::DeletePolygonRender();////�|���S���\�����̔j��
    CRender3D::Delete();//3D�`����̔j��
    CDeviceCreate::Delete3DEnvironment();//3D���j��
    CDeviceCreate::ShutDown();//DirectX�̊��j��
    Audio::DeleteAudio();//�I�[�f�B�I���̔j��

    CoUninitialize();//COM�̏I������,ColnitializeEx��p������K���g�p����

    //���̎��_�ŉ������Ă��Ȃ��������̏���\��
    _CrtDumpMemoryLeaks();
    return true;
    
    
}

//�R�[���o�b�N�֐�
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM  lParam)
{
    Input::SetMouPos(&uMsg, &lParam);

    switch (uMsg)
    {
    case WM_KEYDOWN:  //ESC�L�[�ŏI��
        switch (wParam)
        {
        case VK_ESCAPE:
            g_ls_game_end = true;
            PostQuitMessage(0);
            break;
        }
        break;
    case WM_CLOSE://�E�B���h�E�����ꍇ
        g_ls_game_end = true;
        PostQuitMessage(0);
    case WM_DESTROY://�I������ꍇ
        return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}



