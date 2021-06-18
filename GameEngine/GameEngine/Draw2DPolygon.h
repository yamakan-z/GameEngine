#pragma once
//2Dポリゴン描画に必要なヘッダー---------
#include <Windows.h>
#include <D3D11.h>
#include <d3dCompiler.h>

//メモリ開放マクロ-----------
#define SAFE_DELETE(p)      {if(p){delete(p);        (p)=nullptr;}}
#define SAFE_DELETE_ARRAY(p){if(p){delete[](p);      (p)=nullptr;}}
#define SAFE_RELEASE(p)     {if(p){(p)->Release();   (p)=nullptr;}}

//2Dポリゴン表示で使用する構造体----------------------
//頂点レイアウト構造体（頂点が持つ情報）
struct POINT_LAYOUT
{
	float pos[3];  //X-Y-Z  :頂点
	float color[4];//R-G-B-A:色
	float uv[2];   //U-V    :テクスチャ位置
};

//コンスタントバッファ構造体
struct POLYGON_BUFFER
{
	float color[4];    //R-G-B-A:ポリゴンカラー
	float pos[4];      //ポリゴンの位置情報
	float scale[4];    //拡大縮小率
	float rotation[4]; //回転情報
	float texsize[4];  //表示するイメージのサイズHW
};


typedef class CDraw2DPolygon
{
public:
	CDraw2DPolygon(){}
	~CDraw2DPolygon(){}

	//描画機能　Draw2D群
	static void Draw2D(int id,float x, float y) { Draw2D(id,x, y, 1.0f, 1.0f, 0.0f); }
	static void Draw2D(int id,float x, float y,float r) { Draw2D(id,x, y, 1.0f, 1.0f, r); }
	static void Draw2D(int id,float x, float y,float sx,float sy) { Draw2D(id,x, y, sx, sy, 0.0f); }
	static void Draw2D(int id,float x,float y,float sx,float sy,float r);//描画

	static void Draw2DChar(ID3D11ShaderResourceView* resurec_view, float x, float y, float s, float rgba[4]);//文字描画用

	static HRESULT InitPolygonRender();    //ポリゴン表示環境の初期化
	static void    DeletePolygonRender();  //ポリゴン表示環境の破棄

	static void LoadImage(int id, const wchar_t* img_name); //イメージ情報読み込み

private:
	//GPUで扱う用
	static ID3D11VertexShader*  m_pVertexShader;   //バーテックスシェーダー
	static ID3D11PixelShader*   m_pPixelShader;    //ピクセルシェーダー
	static ID3D11InputLayout*   m_pVertexLayout;   //頂点入力レイアウト
	static ID3D11Buffer*        m_pConstantBuffer; //コンスタントバッファ
	//ポリゴン情報登録用バッファ
	static ID3D11Buffer* m_pVertexBuffer;   //バーテックスバッファ
	static ID3D11Buffer* m_pIndexBuffer;    //インデックスバッファ

	//テクスチャに必要なもの
	static ID3D11SamplerState*       m_pSampleLinear;  //テクスチャーサンプラー
	static ID3D11ShaderResourceView* m_pTexture[32];       //テクスチャーリソース
	static float                     m_width[32];      //テクスチャの横幅
	static float                     m_height[32];     //テクスチャの縦幅

}Draw;