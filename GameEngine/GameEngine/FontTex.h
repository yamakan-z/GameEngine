#pragma once
//���͂ɕK�v�ȃw�b�_�[
#include<Windows.h>
#include<locale.h>
#include<wchar.h>
#include<memory>
#include<list>

using namespace std;

#include "Draw2DPolygon.h"

//�������ʃN���X
class CCherClass
{
public:
	//�R���X�g���N�^
	CCherClass()
	{
		m_pc.reset();
		m_pTexture = nullptr;
	}
	//�f�X�g���N�^
	~CCherClass()
	{
		m_pc.reset();
		SAFE_RELEASE(m_pTexResView);
		SAFE_RELEASE(m_pTexture);
	}

	void CreateCharTex(wchar_t c);//�����e�N�X�`���쐬

private:
	unique_ptr<wchar_t>         m_pc;         //���ʗp����
	ID3D11Texture2D*            m_pTexture;   //�����̃e�N�X�`�����
	ID3D11ShaderResourceView*   m_pTexResView;//�e�N�X�`�����V�F�[�_�ɑ������
};



typedef class CFontTex
{
public:
	CFontTex(){}
	~CFontTex(){}

	static void InitFontTex();
	static void DeleteFontTex();

private:
	//�t�H���g�쐬�ɕK�v�ȕϐ�
	static HFONT           m_hFont;     //�t�H���g�n���h���F�_���t�H���g�iGDI�I�u�W�F�N�g�j
	static HDC             m_hdc;       //�f�B�X�v���C�f�o�C�X�R���e�L�X�g�̃n���h��
	static HFONT           m_oldFont;   //�t�H���g�n���h���F�����t�H���g�iGDI�I�u�W�F�N�g�j
	static TEXTMETRIC      m_TM;        //�t�H���g���i�[�p

	static list<unique_ptr<CCherClass>>list_char_tex;//�������X�g

}Font;