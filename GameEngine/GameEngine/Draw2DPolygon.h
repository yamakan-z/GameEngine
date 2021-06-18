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
	float color[4];    //R-G-B-A:�|���S���J���[
	float pos[4];      //�|���S���̈ʒu���
	float scale[4];    //�g��k����
	float rotation[4]; //��]���
	float texsize[4];  //�\������C���[�W�̃T�C�YHW
};


typedef class CDraw2DPolygon
{
public:
	CDraw2DPolygon(){}
	~CDraw2DPolygon(){}

	//�`��@�\�@Draw2D�Q
	static void Draw2D(int id,float x, float y) { Draw2D(id,x, y, 1.0f, 1.0f, 0.0f); }
	static void Draw2D(int id,float x, float y,float r) { Draw2D(id,x, y, 1.0f, 1.0f, r); }
	static void Draw2D(int id,float x, float y,float sx,float sy) { Draw2D(id,x, y, sx, sy, 0.0f); }
	static void Draw2D(int id,float x,float y,float sx,float sy,float r);//�`��

	static void Draw2DChar(ID3D11ShaderResourceView* resurec_view, float x, float y, float s, float rgba[4]);//�����`��p

	static HRESULT InitPolygonRender();    //�|���S���\�����̏�����
	static void    DeletePolygonRender();  //�|���S���\�����̔j��

	static void LoadImage(int id, const wchar_t* img_name); //�C���[�W���ǂݍ���

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
	static ID3D11ShaderResourceView* m_pTexture[32];       //�e�N�X�`���[���\�[�X
	static float                     m_width[32];      //�e�N�X�`���̉���
	static float                     m_height[32];     //�e�N�X�`���̏c��

}Draw;