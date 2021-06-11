#include "FontTex.h"


HFONT           CFontTex::m_hFont;     //�t�H���g�n���h���F�_���t�H���g�iGDI�I�u�W�F�N�g�j
HDC             CFontTex::m_hdc;       //�f�B�X�v���C�f�o�C�X�R���e�L�X�g�̃n���h��
HFONT           CFontTex::m_oldFont;   //�t�H���g�n���h���F�����t�H���g�iGDI�I�u�W�F�N�g�j
TEXTMETRIC      CFontTex::m_TM;        //�t�H���g���i�[�p
list<unique_ptr<CCherClass>> CFontTex::list_char_tex;//�������X�g

//-----CCherClass---------

//�����e�N�X�`���쐬���\�b�h
void CCherClass::CreateCharTex(wchar_t c)
{

}

//-----CFontTex-----------

//���������\�b�h
void CFontTex::InitFontTex()
{

	//���X�g������
	list_char_tex.clear();


	//�������̃J�e�S���ɁA�������̍��ʃR�[�h�ɉe����^����
	//���̏ꍇ�AUnicode�̕�������{�R�[�h�ɂ���ƂȂ�
	setlocale(LC_CTYPE, "jpn");

	//�_���t�H���g�ݒ�
	HFONT hFont = CreateFont(
		32,
		0,0,0,0,
		FALSE,FALSE,FALSE,
		SHIFTJIS_CHARSET,
		OUT_TT_ONLY_PRECIS,
		CLIP_DEFAULT_PRECIS,
		PROOF_QUALITY,
		FIXED_PITCH|FF_MODERN,
		L"MS �S�V�b�N"//�g�p�t�H���g
	);

	//�w�肳�ꂽ�E�B���h�E�̃N���C�A���g�̈�܂��͉�ʑS�̂�\��
	//�f�B�X�v���C�f�o�C�X�R���e�L�X�g�̃n���h�����擾
	m_hdc = GetDC(NULL);

	//Windows�́ASelectObject�Ńf�o�C�X�R���e�L�X�g�ɂ���B
	//�����ݒ肷��Ƙ_���t�H���g�ɍł��߂������t�H���g���Z�b�g���܂�
	m_oldFont = (HFONT)SelectObject(m_hdc, hFont);

	//���ݑI������Ă���t�H���g�̏����w�肳�ꂽ�o�b�t�@�Ɋi�[
	GetTextMetrics(m_hdc, &m_TM);

	//�_���t�H���g�������o�ɓn��
	m_hFont = hFont;
}

//�폜���\�b�h
void CFontTex::DeleteFontTex()
{
	//���X�g�j��
	list_char_tex.clear();

	//�����GDI�I�u�W�F�N�g��j��
	DeleteObject(m_oldFont);
	DeleteObject(m_hFont);
	//�f�B�X�v���C�f�o�C�X�R���e�L�X�g�n���h�����
	ReleaseDC(NULL, m_hdc);
}