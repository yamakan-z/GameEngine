//�V�X�e���n�̃w�b�_�[�C���N���[�h------------------
#include<stdio.h>
#include<Windows.h>
#include<D3D11.h>
#include<d3dCompiler.h>
#include"DirectXTex.h"
#include"WICTextureLoader11.h"
#include<XAudio2.h>


//Game�V�X�e���p�w�b�_�[�i����j�C���N���[�h--------
#include "WindowCreate.h"
#include "DeviceCreate.h"
#include "Draw2DPolygon.h"
#include "Input.h"

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
//RIFF�t�@�C���t�H�[�}�b�g
class ChunkInfo
{
public:
    ChunkInfo():Size(0),pData(nullptr){}
    unsigned int   Size;   //�`�����N�f�[�^���̃T�C�Y
    unsigned char* pData;  //�`�����N�f�[�^���̐擪�|�C���^
};


//�O���[�o���ϐ�------
IXAudio2*                g_pXAudio2;          //XAudio2�I�u�W�F�N�g
IXAudio2MasteringVoice*  g_pMasteringVoice;   //�}�X�^�[�{�C�X
ChunkInfo*               g_DataChunk;         //�T�E���h���
unsigned char*           g_pResourceData;     //�T�E���h�t�@�C���������|�C���^
IXAudio2SourceVoice*     g_pSourceVoice;      //�T�E���h�{�C�X�C���^�[�t�F�[�X
IXAudio2SubmixVoice*     g_pSFXSubmixVoice;   //�T�u�~�N�X�C���^�[�t�F�[�X


//�v���g�^�C�v�ϐ�
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);//�E�B���h�E�v���W�[�W���[
void  InitAudio();
void  DeleteAudio();
WORD  GetWord(const unsigned char* pData);
DWORD GetDword(const unsigned char* pData);
ChunkInfo FindChunk(const unsigned char* pData, const char* pChunkName);


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
    InitAudio();

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

    DeleteAudio();//�I�[�f�B�I���̔j��

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

//XAudio2�ɂ��Audio���\�z�֐�
void InitAudio()
{
    unsigned XAudio2CreateFlags = 0;

    //XAudio2�C���^�[�t�F�[�X�쐬
    XAudio2Create(&g_pXAudio2, XAudio2CreateFlags);

    //�}�X�^�[�{�C�X�쐬
    g_pXAudio2->CreateMasteringVoice(&g_pMasteringVoice);

    //�~�b�N�X�{�C�X�쐬
    g_pXAudio2->CreateSubmixVoice(&g_pSFXSubmixVoice, 1, 44100, 0, 0, 0, 0);

    //wave�t�@�C���̃I�[�v��
    FILE* fp;
    const wchar_t* FILENAME = L"SETrigger.wav";
    _wfopen_s(&fp,FILENAME, L"rb");

    //�t�@�C���T�C�Y���擾
    unsigned Size = 0;
    fseek(fp, 0, SEEK_END);
    Size = ftell(fp);
    g_pResourceData = new unsigned char[Size];

    //�t�@�C���f�[�^���������Ɉڂ�
    fseek(fp, 0, SEEK_SET);
    fread(reinterpret_cast<char*>(g_pResourceData), Size, 1, fp);
    fclose(fp);

    //RIFF�t�@�C�����
    WAVEFORMATEX WaveformatEx = { 0 };

    //RIFF�f�[�^�̐擪�A�h���X��RIFF�f�[�^�T�C�Y��n��
    ChunkInfo WaveChunk = FindChunk(g_pResourceData, "fmt");
    unsigned char* p = WaveChunk.pData;

    //wave���擾
    WaveformatEx.wFormatTag = GetWord(p);
    p += sizeof(WORD);
    WaveformatEx.nChannels = GetWord(p);
    p += sizeof(WORD);
    WaveformatEx.nSamplesPerSec = GetDword(p);

}

