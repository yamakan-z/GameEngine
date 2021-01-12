#include<windows.h>
#include"WindowCreate.h"

//�폜����Ă��Ȃ����������o�͂Ƀ_���v����
#include<crtdbg.h>
#ifdef _DEBUG
  #ifndef DBG_NEW
    #define DBG_NEW new(_NOMAL_BLOCK,__FILE__,__LINE__)

    #define new DBG_NEW
  #endif
#endif  // _DEBUG

//�v���g�^�C�v�ϐ�
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM Param, LPARAM lParam);

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
