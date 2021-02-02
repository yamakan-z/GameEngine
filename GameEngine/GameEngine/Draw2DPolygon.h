#pragma once
//2D�|���S���`��ɕK�v�ȃw�b�_�[---------
#include <Windows.h>
#include <D3D11.h>
#include <d3dCompiler.h>

//�������J���}�N��-----------
#define SAFE_DELETE(p)      {if(p){delete(p);        (p)=nullptr;}}
#define SAFE_DELETE_ARRAY(p){if(p){delete[](p);      (p)=nullptr;}}
#define SAFE_RELEASE(p)     {if(p){(p)->Release();   (p)=nullptr;}}

//2D�|���S���\���Ŏg�p����\����----------------------
//���_���C�A�E�g�\���́i���_�������j
struct POINT_LAYOUT
{
	float pos[3];  //X-Y-Z  :���_
	float color[4];//R-G-B-A:�F
	float uv[2];   //U-V    :�e�N�X�`���ʒu
};

//�R���X�^���g�o�b�t�@�\����
struct POLYGON_BUFFER
{
	float color[4]; //R-G-B-A:�|���S���J���[
	float pos[4];   //�|���S���̈ʒu���
	float scale[4]; //�g��k����
};


typedef class CDraw2DPolygon
{
public:
	CDraw2DPolygon(){}
	~CDraw2DPolygon(){}

	static void Draw2D(float x,float y,float sx,float sy);//�`��

	static HRESULT InitPolygonRender();    //�|���S���\�����̏�����
	static void    DeletePolygonRender();  //�|���S���\�����̔j��

private:
	//GPU�ň����p
	static ID3D11VertexShader*  m_pVertexShader;   //�o�[�e�b�N�X�V�F�[�_�[
	static ID3D11PixelShader*   m_pPixelShader;    //�s�N�Z���V�F�[�_�[
	static ID3D11InputLayout*   m_pVertexLayout;   //���_���̓��C�A�E�g
	static ID3D11Buffer*        m_pConstantBuffer; //�R���X�^���g�o�b�t�@
	//�|���S�����o�^�p�o�b�t�@
	static ID3D11Buffer* m_pVertexBuffer;   //�o�[�e�b�N�X�o�b�t�@
	static ID3D11Buffer* m_pIndexBuffer;    //�C���f�b�N�X�o�b�t�@

	//�e�N�X�`���ɕK�v�Ȃ���
	static ID3D11SamplerState*       m_pSampleLinear;  //�e�N�X�`���[�T���v���[
	static ID3D11ShaderResourceView* m_pTexture;       //�e�N�X�`���[���\�[�X

}Draw;