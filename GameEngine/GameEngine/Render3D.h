#pragma once
//3Dモデル描画----
#include <Windows.h>
#include <D3D11.h>

#include <d3dCompiler.h>

//メモリ解放マクロ-------------
#define SAFE_DELETE(p)         {if(p){delete (p);     (p)=nullptr;}}
#define SAFE_DELETE_ARRAY(p)   {if(p){delete[](p);    (p)=nullptr;}}
#define SAFE_RELEASE(p)        {if(p){(p)->Release(); (p)=nullptr;}}

//コンスタントバッファクラス
class CMODEL3D_BUFFER
{
public:
	float m_mat[16];//トランスフォーム用行列
	float m_mat_w[16];//法線用のワールド行列
	float m_light_vec[4];//平行ライトの方向
	float m_light_pos[4];//点ライトの位置と出力幅
	float m_ambient[4];//アンビエント
	float m_diffuse[4];//デフィーズ
	float m_emissive[4];//エミッシブ
	float m_specular[4];//スペキュラ
	float m_specular_power[4];//スペキュラパワー
	float m_eye[4];//視野方向
};

//コンスタントバッファ　ボーン用
class CMODEL_BONE_BUFFER
{
public:
	float m_mat[128][16];//4×4の行列をモデルが持つ
};                       //最大数の128個を輸送する

//頂点レイアウトクラス（頂点が持つ情報）
class CPOINT3D_LAYOUT
{
public:
	float m_pos[3];     //X-Y-Z:頂点
	float m_normal[3];  //X-Y-Z:法線
	float m_uv[2];      //U-V　:テクスチャ位置
	float m_color[4];   //R-G-B-A:色
};

//スキンモデル用の頂点レイアウトクラス
class CPOINT3D_SKIN_LAYOUT :public CPOINT3D_LAYOUT
{
public:
	int   m_bone_index[4];    //この頂点に影響を与える行列ID
	float m_bone_weight[4];   //影響を与える行列の重み（影響度）
};

//ボーンクラス
class BONE
{
public:
	char  m_name[128];          //ボーン名
	float m_bind_pos[16];       //ボーン座標変換行列
	float m_inv_bind_pos[16];   //逆ボーン座標変換（原点に移動する行列）
	float m_local_transform[16];//ローカル変換行列
	BONE* m_p_bone_next[16];    //接続用ポインタ（最大16つボーンに接続可能）
	//コンストラクタ
	BONE()
	{
		memset(this, 0x00, sizeof(BONE));
	}
};

//マテリアルクラス(材質情報)
class CMATERIAL
{
public:
	float m_diffuse[4];                  //ディユーズ（拡散反射）
	float m_specular[4];                 //スペキュラ（鏡面反射）
	float m_specular_power;              //スペキュラパワー
	float m_ambient[4];                  //アンビエント（環境光）
	float m_emissive[4];                 //エミッシブ（放射線（自己発光））
	ID3D11ShaderResourceView* m_pTexture;//テクスチャ

	//コンストラクタ
	CMATERIAL()
	{
		memset(this, 0x00, sizeof(CMATERIAL));
		//m_pTexture->Release();
		m_pTexture = nullptr;
	}
	//デストラクタ
	~CMATERIAL()
	{
		if (m_pTexture != nullptr)
		{
			m_pTexture->Release();
		}
	}
};

//モデルクラス
class CMODEL
{
public:
	~CMODEL();
	int* m_pvertex_size;                 //最大頂点数
	int* m_pindex_size;                  //最大面数
	CMATERIAL*         m_Material;      //モデルの材質情報
	ID3D11Buffer**     m_ppVertexBuffer;//バーテックスバッファ（モデルの頂点情報）
	ID3D11Buffer**     m_ppIndexBuffer; //インデックスバッファ（モデルの面情報）
	int m_material_max;                 //マテリアルの数

	//三角錐Model作成
	void CreateSampleTriangularpyramid();//サンプル用の三角錐を作成
	//CMOファイル読み込み
	void LoadCmoModel(const wchar_t* name);
};

//スキンモデルクラス
class C_SKIN_MODEL :public CMODEL
{
public:
	BONE m_bone[128]; //ボーン情報（最大ボーン数128）
	int  m_bone_max;  //使用するボーンの数
	void LoadCmoModel(const wchar_t* name);//スキンモデルのcmoファイル読み込み
	void CreateSampleTriangularpyramid() {};//継承先の関数を実行させないダミー関数
};

//3Dレンダリングクラス
typedef class CRender3D
{
public:
	CRender3D(){}
	~CRender3D(){}
	static void Init();
	static void Delete();

	static void Render(CMODEL* modle,float mat[16],float mat_w[16],float v_eye[3]); //モデルをレンダリングする
	static void Render(C_SKIN_MODEL* modle, float mat[16], float mat_w[16], float v_eye[3]);//スキンモデルのレンダリング
	static void SetLightVec(float x, float y, float z, bool light_on);//平行光源の向きを入れる
	static void SetLightPos(float x, float y, float z, float max);//点光源の位置と出力幅を入れる

private:
	static void SkinAnimation(C_SKIN_MODEL* modle, CMODEL_BONE_BUFFER* bones, int bone_id, float mat_ani[16]);//ボーンにアニメーション情報を入れるF

	static float m_light_vector[4];//平行光源（方向）
	static float m_light_pos[4];//点光源（位置）
	static ID3D11VertexShader* m_pVertexShader;   //バーテックスシェーダー
	static ID3D11PixelShader*  m_pPixelShader;    //ピクセルシェーダー
	static ID3D11InputLayout*  m_pVertexLayout;   //頂点入力レイアウト
	static ID3D11Buffer*       m_pConstantBuffer; //コンスタントバッファ

	static ID3D11VertexShader* m_pVertexShaderSkin;   //スキン用バーテックスシェーダー
	static ID3D11InputLayout*  m_pVertexLayoutSkin;   //スキン用頂点入力レイアウト
	static ID3D11Buffer*       m_pConstantBufferSkin; //スキン用コンスタントバッファ

}Render;