#pragma once
//���͂ɕK�v�ȃw�b�_�[
#include<Windows.h>
#include<locale.h>
#include<wchar.h>
#include<memory>
#include<list>

using namespace std;

#include "DeviceCreate.h"
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

	wchar_t* GetChar() { return m_pc.get(); }//�o�^���Ă��镶���Ԃ�

	void CreateCharTex(wchar_t c,HDC hdc,TEXTMETRIC TM);//�����e�N�X�`���쐬
	ID3D11ShaderResourceView* GetTexResView() { return m_pTexResView; }//�����̃��\�[�X�r���[

private:
	unique_ptr<wchar_t>         m_pc;         //���ʗp����
	ID3D11Texture2D*            m_pTexture;   //�����̃e�N�X�`�����
	ID3D11ShaderResourceView*   m_pTexResView;//�e�N�X�`�����V�F�[�_�ɑ������
};


//�t�H���g�`��N���X
typedef class CFontTex
{
public:
	CFontTex(){}
	~CFontTex(){}

	static void InitFontTex();
	static void DeleteFontTex();

	//const
	static void CreateStrTex(const wchar_t* str);//����������ɕ����e�N�X�`�����쐬
	static void StrDraw(const wchar_t* str, float x, float y, float s, float r, float g, float b, float a);//�����`��

private:
	//�t�H���g�쐬�ɕK�v�ȕϐ�
	static HFONT           m_hFont;     //�t�H���g�n���h���F�_���t�H���g�iGDI�I�u�W�F�N�g�j
	static HDC             m_hdc;       //�f�B�X�v���C�f�o�C�X�R���e�L�X�g�̃n���h��
	static HFONT           m_oldFont;   //�t�H���g�n���h���F�����t�H���g�iGDI�I�u�W�F�N�g�j
	static TEXTMETRIC      m_TM;        //�t�H���g���i�[�p

	static list<unique_ptr<CCherClass>>*list_char_tex;//�������X�g

}Font;