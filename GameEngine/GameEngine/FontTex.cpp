#include "FontTex.h"


HFONT           CFontTex::m_hFont;     //�t�H���g�n���h���F�_���t�H���g�iGDI�I�u�W�F�N�g�j
HDC             CFontTex::m_hdc;       //�f�B�X�v���C�f�o�C�X�R���e�L�X�g�̃n���h��
HFONT           CFontTex::m_oldFont;   //�t�H���g�n���h���F�����t�H���g�iGDI�I�u�W�F�N�g�j
TEXTMETRIC      CFontTex::m_TM;        //�t�H���g���i�[�p
list<unique_ptr<CCherClass>> CFontTex::list_char_tex;//�������X�g

//-----CCherClass---------

//�����e�N�X�`���쐬���\�b�h
void CCherClass::CreateCharTex(wchar_t c,HDC hdc,TEXTMETRIC TM)
{
	//���ʕ����p
	UINT code = 0;//�쐬���镶���R�[�h

	//�����t�H���g�`��p
	BYTE* ptr;      //�����̃O���t�B�b�N�i�r�b�g�}�b�v�j������ꏊ
	DWORD size;     //������\������̂ɕK�v�ȃ������̑傫��
	GLYPHMETRICS GM;//�ی`�����̏�񂪊i�[
	const MAT2 Mat = { {0,1},{0,0},{0,0},{0,1} };//�t�H���g�������݌���

	//�e�N�X�`���`�����ݗp�|�C���^
	D3D11_MAPPED_SUBRESOURCE mapped;//���\�[�X�ɃA�N�Z�X����|�C���^
	BYTE* pBits;                    //�e�N�X�`���̃s�N�Z���������|�C���^

	//���ʕ����R�[�h�o�^
	m_pc.reset(new wchar_t(c));
	code = (UINT)*m_pc.get();

	//�t�H���g��񂩂當���̃r�b�g�}�b�v�擾
	//�����̃r�b�g�}�b�v�̑傫�����擾
	size = GetGlyphOutline(hdc, code, GGO_GRAY4_BITMAP, &GM, 0, NULL, &Mat);
	ptr = new BYTE[size];
	//�����̃r�b�g�}�b�v����ptr�ɓ����
	GetGlyphOutline(hdc, code, GGO_GRAY4_BITMAP, &GM, size, ptr, &Mat);

	//��e�N�X�`���̐ݒ�
	D3D11_TEXTURE2D_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;             //�e�N�X�`���t�H�[�}�b�gR8G8B8��24bit
	desc.SampleDesc.Count = 1;                            //�T���v�����O��1�s�N�Z���̂�
	desc.Usage = D3D11_USAGE_DYNAMIC;                     //CPU�������݉\
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;          //�V�F�[�_���\�[�X
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;         //CPU���珑�����݃A�N�Z�X��
	desc.Height = 32;                                     //�c�̃T�C�Y
	desc.Width = 32;                                      //���̃T�C�Y

	//�ݒ�����ɋ�e�N�X�`�����쐬
	Dev::GetDevice()->CreateTexture2D(&desc, 0, &m_pTexture);

	//�e�N�X�`�������擾����
	D3D11_TEXTURE2D_DESC texDesc;
	m_pTexture->GetDesc(&texDesc);
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