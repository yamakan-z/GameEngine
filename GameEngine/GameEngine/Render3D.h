#pragma once
//3D���f���`��----
#include <Windows.h>
#include <D3D11.h>

#include <d3dCompiler.h>

//����������}�N��-------------
#define SAFE_DELETE(p)         {if(p){delete (p);     (p)=nullptr;}}
#define SAFE_DELETE_ARRAY(p)   {if(p){delete[](p);    (p)=nullptr;}}
#define SAFE_RELEASE(p)        {if(p){(p)->Release(); (p)=nullptr;}}

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
	float m_ambient[3];                  //�A���r�G���g�i�����j
	ID3D11ShaderResourceView* m_pTexture;//�e�N�X�`��
};

//���f���N���X
class CMODEL
{
public:
	int m_vertex_size;                //�ő咸�_��
	int m_index_size;                 //�ő�ʐ�
	CMATERIAL         m_Material;     //���f���̍ގ����
	ID3D11Buffer*     m_pVertexBuffer;//�o�[�e�b�N�X�o�b�t�@�i���f���̒��_���j
	ID3D11Buffer*     m_pIndexBuffer; //�C���f�b�N�X�o�b�t�@�i���f���̖ʏ��j
};

//3D�����_�����O�N���X
typedef class CRender3D
{
public:
	CRender3D(){}
	~CRender3D(){}
	static void Init();
	static void Delete();

private:
	static ID3D11VertexShader* m_pVertexShader;//�o�[�e�b�N�X�V�F�[�_�[
	static ID3D11PixelShader*  m_pPixelShader; //�s�N�Z���V�F�[�_�[
	static ID3D11InputLayout*  m_pVertexLayout;//���_���̓��C�A�E�g

}Render;