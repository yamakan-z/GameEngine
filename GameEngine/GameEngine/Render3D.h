#pragma once
//3Dモデル描画----
#include <Windows.h>
#include <D3D11.h>

#include <d3dCompiler.h>

//メモリ解放マクロ-------------
#define SAFE_DELETE(p)         {if(p){delete (p);     (p)=nullptr;}}
#define SAFE_DELETE_ARRAY(p)   {if(p){delete[](p);    (p)=nullptr;}}
#define SAFE_RELEASE(p)        {if(p){(p)->Release(); (p)=nullptr;}}

//頂点レイアウトクラス（頂点が持つ情報）
class CPOINT3D_LAYOUT
{
public:
	float m_pos[3];     //X-Y-Z:頂点
	float m_normal[3];  //X-Y-Z:法線
	float m_uv[2];      //U-V　:テクスチャ位置
	float m_color[4];   //R-G-B-A:色
};

//マテリアルクラス(材質情報)
class CMATERIAL
{
public:
	float m_diffuse[3];                  //ディユーズ（拡散反射）
	float m_specular[3];                 //スペキュラ（鏡面反射）
	float m_ambient[3];                  //アンビエント（環境光）
	ID3D11ShaderResourceView* m_pTexture;//テクスチャ
};

//モデルクラス
class CMODEL
{
public:
	int m_vertex_size;                //最大頂点数
	int m_index_size;                 //最大面数
	CMATERIAL         m_Material;     //モデルの材質情報
	ID3D11Buffer*     m_pVertexBuffer;//バーテックスバッファ（モデルの頂点情報）
	ID3D11Buffer*     m_pIndexBuffer; //インデックスバッファ（モデルの面情報）
};

//3Dレンダリングクラス
typedef class CRender3D
{
public:
	CRender3D(){}
	~CRender3D(){}
	static void Init();
	static void Delete();

private:
	static ID3D11VertexShader* m_pVertexShader;//バーテックスシェーダー
	static ID3D11PixelShader*  m_pPixelShader; //ピクセルシェーダー
	static ID3D11InputLayout*  m_pVertexLayout;//頂点入力レイアウト

}Render;