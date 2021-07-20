#pragma once
//3D���f���`��----
#include <Windows.h>
#include <D3D11.h>

#include <d3dCompiler.h>

//����������}�N��-------------
#define SAFE_DELETE(p)         {if(p){delete (p);     (p)=nullptr;}}
#define SAFE_DELETE_ARRAY(p)   {if(p){delete[](p);    (p)=nullptr;}}
#define SAFE_RELEASE(p)        {if(p){(p)->Release(); (p)=nullptr;}}

//�R���X�^���g�o�b�t�@�N���X
class CMODEL3D_BUFFER
{
public:
	float m_mat[16];//�g�����X�t�H�[���p�s��
};


//���_���C�A�E�g�N���X�i���_�������j
class CPOINT3D_LAYOUT
{
public:
	float m_pos[3];     //X-Y-Z:���_
	float m_normal[3];  //X-Y-Z:�@��
	float m_uv[2];      //U-V�@:�e�N�X�`���ʒu
	float m_color[4];   //R-G-B-A:�F
};

//�}�e���A���N���X(�ގ����)
class CMATERIAL
{
public:
	float m_diffuse[3];                  //�f�B���[�Y�i�g�U���ˁj
	float m_specular[3];                 //�X�y�L�����i���ʔ��ˁj
	float m_specular_power;              //�X�y�L�����p���[
	float m_ambient[3];                  //�A���r�G���g�i�����j
	float m_emissive[4];                 //�G�~�b�V�u�i���ː��i���Ȕ����j�j
	ID3D11ShaderResourceView* m_pTexture;//�e�N�X�`��
};

//���f���N���X
class CMODEL
{
public:
	~CMODEL();
	int* m_pvertex_size;                 //�ő咸�_��
	int* m_pindex_size;                  //�ő�ʐ�
	CMATERIAL*         m_Material;      //���f���̍ގ����
	ID3D11Buffer**     m_ppVertexBuffer;//�o�[�e�b�N�X�o�b�t�@�i���f���̒��_���j
	ID3D11Buffer**     m_ppIndexBuffer; //�C���f�b�N�X�o�b�t�@�i���f���̖ʏ��j
	int m_material_max;                 //�}�e���A���̐�

	//�O�p��Model�쐬
	void CreateSampleTriangularpyramid();//�T���v���p�̎O�p�����쐬
};

//3D�����_�����O�N���X
typedef class CRender3D
{
public:
	CRender3D(){}
	~CRender3D(){}
	static void Init();
	static void Delete();

	static void Render(CMODEL* modle,float mat[16]); //���f���������_�����O����

private:
	static ID3D11VertexShader* m_pVertexShader;   //�o�[�e�b�N�X�V�F�[�_�[
	static ID3D11PixelShader*  m_pPixelShader;    //�s�N�Z���V�F�[�_�[
	static ID3D11InputLayout*  m_pVertexLayout;   //���_���̓��C�A�E�g
	static ID3D11Buffer*       m_pConstantBuffer; //�R���X�^���g�o�b�t�@

}Render;