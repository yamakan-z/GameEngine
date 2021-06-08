//�V�X�e���n�̃w�b�_�[�C���N���[�h------------------
#include<stdio.h>
#include<Windows.h>
#include<D3D11.h>
#include<d3dCompiler.h>
#include"DirectXTex.h"
#include"WICTextureLoader11.h"



//Game�V�X�e���p�w�b�_�[�i����j�C���N���[�h--------
#include "WindowCreate.h"
#include "DeviceCreate.h"
#include "Draw2DPolygon.h"
#include "Input.h"
#include "Audio.h"

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

//Object�N���X
class CObj
{
public://���J��(�O������̃A�N�Z�X�j
    virtual void Action() { printf("ObjAction\n"); };
    virtual void Draw()   { printf("ObjDraw\n"); };
private://���I���i���̃N���X�̂݃A�N�Z�X�j
   
};


//��l���N���X
class CHero :public CObj
{
public://���J��
    void Action() { printf("HeroAction\n"); };
    void Draw()   { printf("HeroDraw\n"); };
private://���I��
    
};

//�G�N���X
class CEnemy :public CObj
{
public://���J��
    void Action() { printf("EnemyAction\n"); };
    void Draw()   { printf("EnemyDraw\n"); };
private://���I��
   
};

//�w�i�N���X
class CBackImg :public CObj
{
public: //���J��
    void Action() { printf("BackImgAction\n"); };
    void Draw()   { printf("BackImgDraw\n"); };
private: //���I��
  
};

void main()
{
    CObj* obj_array[3];
    //�o�^
    obj_array[0] = new CHero();
    obj_array[1] = new CEnemy();
    obj_array[2] = new CBackImg();

    //�A�N�V�������s
    for (int i = 0; i < 3; i++)
    {
        obj_array[i]->Action();
    }

    //�h���[
    for (int i = 0; i < 3; i++)
    {
        obj_array[i]->Draw();
    }

    //�폜
    for (int i = 0; i < 3; i++)
    {
        delete obj_array[i];
    }

    getchar();
}
//�O���[�o���ϐ�------

//�v���g�^�C�v�ϐ�
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);//�E�B���h�E�v���W�[�W���[


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

    //�E�B���h�E�쐬
    CWindowCreate::NewWindow(800, 600, name, hInstance);

    //�I�[�f�B�I�쐬
    Audio::InitAudio();

    //�~���[�W�b�N���擾
    Audio::LoadBackMusic("�e�X�g.ogg");
    Audio::LoadSEMusic   (0, L"SEBom.wav");
    Audio::LoadSEMusic(1, L"SETrigger.wav");

    Audio::StartLoopMusic();
    Audio::StartMusic(0);
    Audio::MasterVolume(1.0f);

    //DirectX Device�̏�����
    CDeviceCreate::InitDevice(CWindowCreate::GetWnd(), 800, 600);

    //�|���S���\�����̏�����
    Draw::InitPolygonRender();
    Draw::LoadImage(0, L"Texture.png");//0�Ԗڂ�"Texture.png"��ǂݍ���
    Draw::LoadImage(1, L"tex1.png");
    Draw::LoadImage(2, L"tex2.png");
    Draw::LoadImage(3, L"tex3.png");

    //���͗p�̃N���X������
    Input::InitInput();

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
        static float x = 0.0f;
        
        //�~���[�W�b�N�`�F�b�N�p
        static bool IsKeyOn = true;
        if (Input::KeyPush('Z') == true)
        {
            if (IsKeyOn == true)
            {
                IsKeyOn = false;
                Audio::StartMusic(0);
            }
        }
        else
        {
            IsKeyOn = true;
        }

        static bool IsKeyOnQ = true;
        if (Input::KeyPush('Q') == true)
        {
            if (IsKeyOnQ == true)
            {
                IsKeyOnQ = false;
                Audio::StartMusic(1);
            }
        }
        else
        {
            IsKeyOnQ = true;
        }
        
        //���[�v�~���[�W�b�N�{�����[��
        static float t = 0.0f;
        if (Input::KeyPush('X'))
        {
            t += 0.1f;
            Audio::LoopMusicVolume(t);
        }
        if (Input::KeyPush('C'))
        {
            t -= 0.1f;
            Audio::LoopMusicVolume(t);
        }
        //���ʉ�ID=0
        static float t1 = 0.0f;
        if (Input::KeyPush('S'))
        {
            t1 += 0.1f;
            Audio::SEMusicVolume(0,t1);
        }
        if (Input::KeyPush('D'))
        {
            t1 -= 0.1f;
            Audio::SEMusicVolume(0, t1);
        }
        //���ʉ�ID=1
        static float t2 = 0.0f;
        if (Input::KeyPush('W'))
        {
            t2 += 0.1f;
            Audio::SEMusicVolume(1, t2);
        }
        if (Input::KeyPush('E'))
        {
            t2 -= 0.1f;
            Audio::SEMusicVolume(1, t2);
        }


        //A�L�[�������ꂽ�Ƃ�
        if (GetAsyncKeyState('A')==true)
        {
            x += 1.0f;
        }
        //�V�X�e���L�[�u�J�[�\���L�[���v�������ꂽ�Ƃ�
        if (GetAsyncKeyState(VK_UP)==true)
        {
            x += 1.0f;
        }
        //�V�X�e���L�[�@�}�E�X�E�N���b�N
        if (GetAsyncKeyState(VK_RBUTTON)==true)
        {
            x += 1.0f;
        }

        static float time = 0.0f;
        time += 1.0f;
        Draw::Draw2D(0,128+x,100,1.0f,1.0f,time);//�e�N�X�`���t���l�p�|���S���`��
        Draw::Draw2D(1,550, 300);
        Draw::Draw2D(2,Input::GetMouX(), Input::GetMouY(), 1.0f, 1.0f,-time);
        Draw::Draw2D(3,256, 256+64, time);


        //�����_�����O�I��
        Dev::GetSwapChain()->Present(1, 0);//60FPS�Ńo�b�N�o�b�t�@�ƃv���C�}���o�b�t�@�̌���

    } while (msg.message != WM_QUIT);

    Draw::DeletePolygonRender();////�|���S���\�����̔j��

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



