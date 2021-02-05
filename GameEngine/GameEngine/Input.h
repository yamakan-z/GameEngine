#pragma once
//���͂ɕK�v�ȃw�b�_�[
#include <Windows.h>

typedef class CInput
{
public:
	CInput(){}
	~CInput(){}

	//������
	static void InitInput();

	//�}�E�X�ʒu�擾�p�@�E�B���h�E�v���W�[�W���[���ɐ錾
	static void SetMouPos(UINT* uMsg, LPARAM* lParam);

	//�L�[�E�}�E�X�{�^���̃v�b�V���m�F
	static bool KeyPush(int key);

	//�}�E�X�̈ʒu���擾
	static int GetMouX() { return m_mou_x; }
	static int GetMouY() { return m_mou_y; }

private:
	static int m_mou_x;  //�}�E�X�̈ʒuX
	static int m_mou_y;  //�}�E�X�̈ʒuY

}Input;