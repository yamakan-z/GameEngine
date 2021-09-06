#include "Render3D.h"
#include "Draw2DPolygon.h"
#include "DeviceCreate.h"
#include "Math3D.h"

float CRender3D::m_light_vector[4];//平行光源（方向）
float CRender3D::m_light_pos[4];//点光源（位置）

ID3D11VertexShader*   CRender3D::m_pVertexShader;   //バーテックスシェーダー
ID3D11PixelShader*    CRender3D::m_pPixelShader;    //ピクセルシェーダー
ID3D11InputLayout*    CRender3D::m_pVertexLayout;   //頂点入力レイアウト
ID3D11Buffer*         CRender3D::m_pConstantBuffer; //コンスタントバッファ

ID3D11VertexShader*   CRender3D::m_pVertexShaderSkin;   //スキン用バーテックスシェーダー
ID3D11InputLayout*    CRender3D::m_pVertexLayoutSkin;   //スキン用頂点入力レイアウト
ID3D11Buffer*         CRender3D::m_pConstantBufferSkin; //スキン用コンスタントバッファ
 

//HLSLソースコード（メモリ内登録）
const char* g_hlsl_sause_code =
{
	//CPUから取得する頂点情報構造体
	"struct vertexIn              \n"
	"{                            \n"
	"   float4 pos : POSITION;    \n"
	"   float4 col : COLOR;       \n"
	"   float4 Nor : NORMAL;      \n"
	"   float2 uv  : UV;          \n"
	"};                           \n"

	//CPUから取得するスキン用頂点情報構造体
	"struct vertex_skin_In          \n"
	"{                              \n"
	"  float4 pos : POSITION;       \n"
	"  float4 col : COLOR;          \n"
	"  float4 Nor : NORMAL;         \n"
	"  float2 uv  : UV;             \n"
	"  int4   bi  : BI;             \n"
	"  float4 we  : WE;             \n"
	"};                             \n"

	//VSからPSに送る情報
   "struct vertexOut                \n"
   "{                               \n"
   "  float4 pos  : SV_POSITION;    \n"
   "  float4 col  : COLOR;          \n"
   "  float2 uv   : UV;             \n"
   "  float3 nor  : NORMAL;         \n"//ワールド行列×法線
   "  float3 pos_c: POSITION_COLOR; \n"//ワールド行列×位置
   "};                              \n"

	//コンスタントバッファ受取先
	"cbuffer global  :  register( b0 ) \n"
	"{                                 \n"
	"  float4x4 mat;                   \n"//ビューパイプライントランスフォーム用
	"  float4x4 w_mat;                 \n"//法線用のワールドトランスフォーム用
	"  float4 l_vec;                   \n"//平行ライト用ベクトル
	"  float4 l_pos;                   \n"//点ライト用ポジション
	"  float4 amb;                     \n"//アンビエント
	"  float4 diff;                    \n"//デフィーズ
	"  float4 emi;                     \n"//エミッシブ
	"  float4 sp;                      \n"//スペキュラ
	"  float4 sp_p;                    \n"//スペキュラパワー
	"  float4 eye;                     \n"//視野方向
	"};                                \n"

	//ボーン用コンスタントバッファ受取先
	"cbuffer global_bone : register( b1 )  \n"
	"{                                     \n"
	"  float4x4 b_mat[128];                \n"//ボーン行列
	"};                                    \n"



	//テクスチャ情報
	"Texture2D      txDiffuse  : register( t0 );\n"//テクスチャのグラフィック
	"SamplerState   samLinear  : register( s0 );\n"//テクスチャサンプラ

	//頂点シェーダ
   "vertexOut vs(vertexIn IN)                              \n"
   "{                                                      \n" 
	"vertexOut OUT;                                        \n"
	"OUT.pos   = mul(IN.pos,transpose(mat));               \n"
	"OUT.col   = IN.col;                                   \n"
	"OUT.uv    = IN.uv;                                    \n"
	"OUT.nor   = mul(IN.Nor,(float3x3)transpose(w_mat));   \n"//nor=w_mat[3×3]*法線(x,y,z)   
	"OUT.pos_c = mul(IN.pos,transpose(w_mat));             \n"//pos=w_mat[4×4]*位置(x,y,z)
	"return OUT;                                           \n"
    "}                                                     \n"	

	//頂点スキン用シェーダ
   "vertexOut vs_skin(vertex_skin_In IN)                    \n"
   "{                                                       \n"
   "  float4 pos=(float4)0.0f;                              \n"//ボーンの影響受けた頂点の位置
   "  float3 nor=(float3)0.0f;                              \n"//ボーンの影響受けた頂点の法線
   "                                                        \n"
   "  int   index[4] ={IN.bi.x,IN.bi.y,IN.bi.z,In.bi.w};    \n"//BoneIDがint型なので配列に入れなおす
   "  float weight[4]={IN.we.x,IN.we.y,IN.we.z,IN.we.w};    \n"//Weightがfloat4型なので配列に入れなおす
   "                                                        \n"
   "  for(int i=0;i<4;i++)                                  \n"//各ボーン行列×重みを頂点に加算する処理
   " {                                                      \n"
   "   float    w = weight[i];                              \n"//ウェイト値取得
   "   float4x4 m = transpose(b_mat[index[i]]);             \n"//ボーンIDからボーン行列を取得
   "   pos += w * mul(IN.pos,m);                            \n"//スキンした位置+＝重み×位置×ボーン行列
   "   nor += w * mul(IN.Nor,(float3x3)m);                  \n"//スキンした法線+＝重み×法線×ボーン行列
   " }                                                      \n"
   "                                                        \n"//ビューイングパイプライン系の処理
   "  vertexOut OUT;                                        \n"
   "  OUT.pos   = mul(pos,transpose(mat));                  \n"//pos=wvp[4×4]*スキンした位置(x,y,z)
   "  OUT.col   = IN.col;                                   \n"
   "  OUT.uv    = IN.uv;                                    \n"
   "  OUT.nor   = mul(nor,(float3x3)transpose(w_mat));      \n"//nor=w_mat[3×3]*法線(x,y,z)   
   "  OUT.pos_c = mul(pos,transpose(w_mat));                \n"//pos=w_mat[4×4]*位置(x,y,z)
   "  return OUT;                                           \n"
   "}                                                       \n"

	//ピクセルシェーダ
   "float4 ps(vertexOut IN) :SV_Target                                 \n"
   "{                                                                  \n"
   "   float4 col = IN.col;                                            \n"//頂点のcolor情報を取得
   "   float4 light_vec_sc=(float4)1.0f;                               \n"//平行ライト用の陰影結果を入れる変数
   "   float4 light_pos_sc=(float4)1.0f;                               \n"//点ライト用の陰影結果を入れる変数
   "                                                                   \n"
   "   if(l_vec.w != 0.0f && any(IN.nor)==true)                        \n"//l_vec.wが0であれば平行のライト計算はしない
   "   {                                                               \n"//また、法線がない場合も、計算しない
   "     light_vec_sc.rgb = dot(normalize(IN.nor),normalize(-l_vec));  \n"//法線と-光源向きで内積を求め陰影のRGBに入れる
   "     light_vec_sc=saturate(light_vec_sc);                          \n"//light_vec_scの値を（0～1）までにする
   "   }                                                               \n"
   "                                                                   \n"
   "   if(l_pos.w > 0.0f && any(IN.nor)==true)                         \n"//l_pos.wが0以下であれば点ライト計算はしない
   "   {                                                               \n"
   "     float3 lp_len = l_pos.xyz - IN.pos_c;                         \n"//点光源と頂点の各ピクセルの位置からベクトルを求める
   "     float  len = length(lp_len);                                  \n"//求めたベクトルの長さを求める
   "     float  w = saturate(len/l_pos.w);                             \n"//ベクトルの長さと出力幅を％で出す
   "     light_pos_sc.rgb = dot(normalize(IN.nor),normalize(lp_len));  \n"//法線とlp_lenから陰影部を求める
   "     light_pos_sc.rgb = light_pos_sc.rgb*(float3)1.0-w;            \n"//陰影と光の強さを合成して点光源の陰影とする
   "   }                                                               \n"
   "  float4 d=(float4)1.0f;                                           \n"
   "  if(any(diff)==true)                                              \n"//ディフィーズ計算有無
   "  {                                                                \n"
   "    d=max(light_pos_sc,light_vec_sc)*diff;                         \n"//点・ベクトル光源の中で明るい陰影と
   "  }                                                                \n"//ディフィーズ色を合成する
   "                                                                   \n"
   "  float4 a=(float4)0.0f;                                           \n"
   "  if(any(amb)==true)                                               \n"//アンビエントの有無
   "  {                                                                \n"
   "    a=amb;                                                         \n"//アンビエント値を代入
   "  }                                                                \n"
   "                                                                   \n"
   "  float4 vec_sp=(float4)0.0f;vec_sp.a=1.0f;                        \n"//スペキュラ平行光源の結果を入れる変数
   "  float4 pos_sp=(float4)0.0f;pos_sp.a=1.0f;                        \n"//スペキュラ点光源の結果を入れる変数
   "  if(any(sp)==true&&any(eye)==true)                                \n"//スペキュラパワー値で計算有無を行う
   "  {                                                                \n"
   "    float3 e=normalize(eye.rgb);                                   \n"//視線方向ベクトルの正規化
   "    float3 n=normalize(IN.nor.rgb);                                \n"//法線の正規化
   "    float3 l=-normalize(l_vec.rgb);                                \n"//平行ライト向きを逆にしての正規化
   "    if(l_vec.w != 0.0f && any(IN.nor) == true)                     \n"//l_vec.wが0であれば平行ライト計算はしない
   "    {                                                              \n"//また、法線がない場合も計算しない
   "      if(dot(n,l)>=0.0f)                                           \n"//光の向きと面の向きが反対かどうかを調べる
   "      {                                                            \n"
   "        float3 rv=normalize(n*dot(n,l)*2.0-l);                     \n"//反射ベクトルを求める
   "        vec_sp.rgb=pow(max(0,dot(rv,e)),sp_p.x);                   \n"//スペキュラの輝度を算出
   "        float3 no=(float3)((sp_p.x+2)/(2*3.14));                   \n"//輝度に正規化係数を乗算
   "        vec_sp.rgb=sp.rgb*saturate(no*vec_sp.rgb);                 \n"//正規化係数・スペキュラ・色を乗算して求める
   "      }                                                            \n"
   "    }                                                              \n"
   "                                                                   \n"
   "    if(l_pos.w>0.0f && any(IN.nor)==true)                          \n"//l_pos.wが0以下であれば点ライト計算はしない
   "    {                                                              \n"
   "      float3 lp_len=IN.pos_c-l_pos.xyz;                            \n"//点光源と頂点の各ピクセルの位置からベクトルを求める
   "      float  len=length(lp_len);                                   \n"//求めたベクトルの長さを求める
   "      float3 nor_lp=-normalize(lp_len);                            \n"//求めたベクトルの正規化
   "      float  w=saturate(len/l_pos.w);                              \n"//ベクトルの長さと出力幅を％で出す
   "      if(dot(n,nor_lp)>=0.0f)                                      \n"//点光源の向きと面の向きが反対かどうかを調べる
   "      {                                                            \n"
   "        float3 rv=normalize(n*dot(n,nor_lp)*2.0-nor_lp);           \n"//点光源に対して反射ベクトルを求める
   "        pos_sp.rgb=pow(max(0,dot(rv,e)),sp_p.x);                   \n"//スペキュラの輝度を算出
   "        float3 no=(float3)((sp_p.x+2) / (2*3.14));                 \n"//輝度に正規化係数を乗算
   "        pos_sp.rgb=saturate(no*pos_sp.rgb)*((float3)1.0-w);        \n"//スペキュラと光の強さを点対象の距離で減衰させる
   "      }                                                            \n"
   "    }                                                              \n"
   "  }                                                                \n"
   "                                                                   \n"
   "  col=col*d+vec_sp+pos_sp+a;                                       \n"//色=ポリゴン色*ディフィーズ色+スペキュラ+アンビエント色
   "  col=saturate(col);                                               \n"//求めた色を0.0～1.0を超えないようにする
   "                                                                   \n"
   "  if(any(emi)==true)                                               \n"//エミッシブの有無
   "  {                                                                \n"
   "    col=max(col,emi);                                              \n"//求めた色とエミッシブ（自己発光）、明るい色を最終的な色とする
   "  }                                                                \n"
   "                                                                   \n"
   "  float4 tex=(float4)0.0f;                                         \n"//テクスチャ
   "  tex=txDiffuse.Sample(samLinear,IN.uv);                            \n"//UVからテクスチャの色の値を取得
   "  float x,y;                                                       \n"//テクスチャの大きさを入れる変数
   "  txDiffuse.GetDimensions(x,y);                                    \n"//テクスチャの大きさを取得
   "  if(x != 0.0f && y != 0.0f)                                       \n"//大きさが0の時、テクスチャは無いと判断する
   "     col *= tex;                                                   \n"//テクスチャの色を合成する
   "                                                                   \n"
   "  if(col.a<=0.0f)                                                  \n"//完全透過であれば、
   "     discard;                                                      \n"//そのピクセルを描画しない
   "                                                                   \n"
   "  return col;                                                      \n"//最終的な出力
   "}                                                                  \n"
};






void CRender3D::Init()
{

	//平行ライト値の初期化
	memset(m_light_vector, 0x00, sizeof(m_light_vector));
	//点ライト
	memset(m_light_pos, 0x00, sizeof(m_light_pos));

	HRESULT hr = S_OK;
	//hlslファイル読み込み　ブロブ作成　ブロブとはシェーダーの塊みたいなもの
	//XXシェーダーとして特徴を持たない。後で各種シェーダーとなる
	ID3DBlob* pCompiledShader = NULL;
	ID3DBlob* pErrors = NULL;

	//スキンモデル用----------------
	//メモリ内のHLSLの vs_skin関数部分をコンパイル
	hr = D3DCompile(g_hlsl_sause_code, strlen(g_hlsl_sause_code), 0, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"vs_skin", "vs_4_0", 0, 0, &pCompiledShader, &pErrors);

	if (FAILED(hr))
	{
		//エラーがある場合、ｃがデバッグ情報を持つ
		char* c = (char*)pErrors->GetBufferPointer();
		MessageBox(0, L"3Dhlsl読み込み失敗1", NULL, MB_OK);
		SAFE_RELEASE(pErrors);
		return;
	}

	//コンパイルしたバーテックスシェーダーを元にインターフェースを作成
	hr = Dev::GetDevice()->CreateVertexShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(),
		NULL, &m_pVertexShaderSkin);
	if (FAILED(hr))
	{
		SAFE_RELEASE(pCompiledShader);
		MessageBox(0, L"バーテックスシェーダースキン用作成失敗", NULL, MB_OK);
		return;
	}

	//頂点インプットレイアウトを定義
	D3D11_INPUT_ELEMENT_DESC layout_skin[] =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT   ,0,0, D3D11_INPUT_PER_VERTEX_DATA,0},
		{"NORMAL",  0,DXGI_FORMAT_R32G32B32_FLOAT,   0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"UV"   ,   0,DXGI_FORMAT_R32G32_FLOAT,      0,24,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"COLOR",   0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,32,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"BI"   ,   0,DXGI_FORMAT_R32G32B32A32_UINT ,0,48,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"WE"   ,   0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,64,D3D11_INPUT_PER_VERTEX_DATA,0},
	};
	UINT numElements_skin = sizeof(layout_skin) / sizeof(layout_skin[0]);

	//頂点インプットレイアウトを作成・レイアウトをセット
	hr = Dev::GetDevice()->CreateInputLayout(layout_skin, numElements_skin, pCompiledShader->GetBufferPointer(),
		pCompiledShader->GetBufferSize(), &m_pVertexLayout);
	if (FAILED(hr))
	{
		MessageBox(0, L"レイアウト作成失敗", NULL, MB_OK);
		return;
	}
	SAFE_RELEASE(pCompiledShader);

	//スキン用のコンスタントバッファ作成
	D3D11_BUFFER_DESC cb_skin;
	cb_skin.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
	cb_skin.ByteWidth           = sizeof(CMODEL_BONE_BUFFER);
	cb_skin.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
	cb_skin.MiscFlags           = 0;
	cb_skin.StructureByteStride = 0;
	cb_skin.Usage               = D3D11_USAGE_DYNAMIC;

	//ステータスを元にコンスタントバッファ
	hr = Dev::GetDevice()->CreateBuffer(&cb_skin, NULL, &m_pConstantBufferSkin);
	if (FAILED(hr))
	{
		MessageBox(0, L"スキン用コンスタントバッファ作成失敗", NULL, MB_OK);
		return;
	}

	//-------------------------------

	//メモリ内のHLSLのvs関数部分のコンパイル
	hr = D3DCompile(g_hlsl_sause_code, strlen(g_hlsl_sause_code), 0, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"vs", "vs_4_0", 0, 0, &pCompiledShader, &pErrors);

	if (FAILED(hr))
	{
		//エラーがある場合、ｃがデバッグ情報を持つ
		char* c = (char*)pErrors->GetBufferPointer();
		MessageBox(0, L"3Dhlsl読み込み失敗1", NULL, MB_OK);
		SAFE_RELEASE(pErrors);
		return;
	}

	//コンパイルしたバーテックスシェーダーを元にインターフェースを作成
	hr = Dev::GetDevice()->CreateVertexShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(),
		NULL, &m_pVertexShader);
	if (FAILED(hr))
	{
		SAFE_RELEASE(pCompiledShader);
		MessageBox(0, L"バーテックスシェーダー作成失敗", NULL, MB_OK);
		return;
	}
	//頂点インプットレイアウトを定義
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT   ,0,0, D3D11_INPUT_PER_VERTEX_DATA,0},
		{"NORMAL",  0,DXGI_FORMAT_R32G32B32_FLOAT,   0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"UV"   ,   0,DXGI_FORMAT_R32G32_FLOAT,      0,24,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"COLOR",   0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,32,D3D11_INPUT_PER_VERTEX_DATA,0},
	};
	UINT numElements = sizeof(layout) / sizeof(layout[0]);

	//頂点インプットレイアウトを作成・レイアウトをセット
	hr = Dev::GetDevice()->CreateInputLayout(layout, numElements, pCompiledShader->GetBufferPointer(),
		pCompiledShader->GetBufferSize(), &m_pVertexLayout);
	if (FAILED(hr))
	{
		MessageBox(0, L"レイアウト作成失敗", NULL, MB_OK);
		return;
	}
	SAFE_RELEASE(pCompiledShader);

	//メモリ内のHLSLのps関数部分をコンパイル
	hr = D3DCompile(g_hlsl_sause_code,strlen(g_hlsl_sause_code),0,0,D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"ps", "ps_4_0", 0, 0, &pCompiledShader, &pErrors);
	if (FAILED(hr))
	{
		//エラーがある場合、ｃがデバック情報を持つ
		char* c = (char*)pErrors->GetBufferPointer();
		MessageBox(0, L"hlsl読み込み失敗2", NULL, MB_OK);
		SAFE_RELEASE(pErrors);
		return;
	}

	//コンパイルしたピクセルシェーダーでインターフェースを作成
	hr = Dev::GetDevice()->CreatePixelShader(pCompiledShader->GetBufferPointer(),
		pCompiledShader->GetBufferSize(), NULL, &m_pPixelShader);
	if (FAILED(hr))
	{
		SAFE_RELEASE(pCompiledShader);
		MessageBox(0, L"ピクセルシェーダー作成失敗", NULL, MB_OK);
		return;
	}
	SAFE_RELEASE(pCompiledShader);

	//バッファにコンスタントバッファ（シェーダにデータ受け渡し用）ステータスを設定
	D3D11_BUFFER_DESC cb;
	cb.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
	cb.ByteWidth           = sizeof(CMODEL3D_BUFFER);
	cb.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags           = 0;
	cb.StructureByteStride = 0;
	cb.Usage               = D3D11_USAGE_DYNAMIC;

	//ステータスを元にコンスタントバッファを作成
	hr = Dev::GetDevice()->CreateBuffer(&cb, NULL, &m_pConstantBuffer);
	if (FAILED(hr))
	{
		MessageBox(0, L"コンスタントバッファ作成失敗", NULL, MB_OK);
		return;
	}

}

//破棄メソッド
void CRender3D::Delete()
{
	//通常モデル用の破棄
	SAFE_RELEASE(m_pConstantBuffer);//コンスタントバッファ破棄
	SAFE_RELEASE(m_pPixelShader);   //ピクセルシェーダー破棄
	SAFE_RELEASE(m_pVertexLayout);  //頂点入力レイアウト破棄
	SAFE_RELEASE(m_pVertexShader);  //バーテックスシェーダー
	//スキンモデル用の破棄
	SAFE_RELEASE(m_pConstantBufferSkin);//コンスタントバッファ破棄
	SAFE_RELEASE(m_pVertexLayoutSkin);  //頂点入力レイアウト破棄
	SAFE_RELEASE(m_pVertexShaderSkin);  //バーテックスシェーダー破棄
}

//モデルをレンダリングする
void CRender3D::Render(CMODEL* modle,float mat[16],float mat_w[16],float v_eye[3])
{
	//頂点レイアウト
	Dev::GetDeviceContext()->IASetInputLayout(m_pVertexLayout);

	//使用するシェーダーの登録
	Dev::GetDeviceContext()->VSSetShader(m_pVertexShader, NULL, 0);
	Dev::GetDeviceContext()->PSSetShader(m_pPixelShader,  NULL, 0);

	//コンスタントバッファを使用するシェーダに登録
	Dev::GetDeviceContext()->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	Dev::GetDeviceContext()->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);


	//プリミティブ・トポロジーをセット
	Dev::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	for (int i = 0; i < modle->m_material_max; i++)
	{
		//バーテックスバッファ登録
		UINT stride = sizeof(CPOINT3D_LAYOUT);
		UINT offset = 0;
		Dev::GetDeviceContext()->IASetVertexBuffers(0, 1, &modle->m_ppVertexBuffer[i], &stride, &offset);
		//インデックスバッファ登録
		Dev::GetDeviceContext()->IASetIndexBuffer(modle->m_ppIndexBuffer[i], DXGI_FORMAT_R16_UINT, 0);

		//コンスタントバッファのデータ登録
		D3D11_MAPPED_SUBRESOURCE pData;
		if (SUCCEEDED(Dev::GetDeviceContext()->Map(m_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
		{
			CMODEL3D_BUFFER data;

			//トランスフォーム行列情報コンスタントバッファに渡す
			if (mat == nullptr)
			{
				Math3D::IdentityMatrix(data.m_mat);//ない場合は単位行列
			}
			else
			{
				memcpy(data.m_mat, mat, sizeof(float) * 16);
			}

			//法線用のワールド行列を渡す
			if (mat_w == nullptr)
			{
				Math3D::IdentityMatrix(data.m_mat_w);//ない場合は単位行列を渡す
			}
			else
			{
				memcpy(data.m_mat_w, mat_w, sizeof(float) * 16);
			}

			//視野方向ベクトルを渡す
			if (v_eye == nullptr)
			{
				memset(data.m_eye, 0x00, sizeof(data.m_eye));
			}
			else
			{
				data.m_eye[0] = v_eye[0];
				data.m_eye[1] = v_eye[1];
				data.m_eye[2] = v_eye[2];
				data.m_eye[3] = 1.0f;
			}

			//平行ライトの値を渡す
			memcpy(data.m_light_vec, m_light_vector, sizeof(m_light_vector));
			//点ライトの値を渡す
			memcpy(data.m_light_pos, m_light_pos, sizeof(m_light_pos));

			//材質　アンビエントを渡す
			memcpy(data.m_ambient, modle->m_Material[i].m_ambient, sizeof(data.m_ambient));
			//材質　デフィーズを渡す
			memcpy(data.m_diffuse, modle->m_Material[i].m_diffuse, sizeof(data.m_diffuse));
			//材質　エミッシブを渡す
			memcpy(data.m_emissive, modle->m_Material[i].m_emissive, sizeof(data.m_emissive));
			//材質　スペキュラを渡す
			memcpy(data.m_specular, modle->m_Material[i].m_specular, sizeof(data.m_specular));
			for (int j = 0; j < 4; j++)
			{
				data.m_specular_power[j] = modle->m_Material[i].m_specular_power;
			}

			memcpy_s(pData.pData, pData.RowPitch, (void*)&data, sizeof(CMODEL3D_BUFFER));
			//コンスタントバッファをシェーダに輸送
			Dev::GetDeviceContext()->Unmap(m_pConstantBuffer, 0);
		}

		//テクスチャーサンプラを登録
		Dev::GetDeviceContext()->PSSetSamplers(0, 1, Draw::GetSamplerState());
		//テクスチャを登録
		Dev::GetDeviceContext()->PSSetShaderResources(0, 1, &modle->m_Material[i].m_pTexture);

		//登録した情報を元にポリゴン描画
		Dev::GetDeviceContext()->DrawIndexed(modle->m_pindex_size[i] * 3, 0, 0);

	}
	

	
}

//スキンモデルのレンダリング
void CRender3D::Render(C_SKIN_MODEL* modle, float mat[16], float mat_w[16], float v_eye[3])
{
	//頂点レイアウト
	Dev::GetDeviceContext()->IASetInputLayout(m_pVertexLayoutSkin);

	//使用するシェーダーの登録
	Dev::GetDeviceContext()->VSSetShader(m_pVertexShaderSkin, NULL, 0);
	Dev::GetDeviceContext()->PSSetShader(m_pPixelShader, NULL, 0);

	//コンスタントバッファを使用するシェーダに登録
	Dev::GetDeviceContext()->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	Dev::GetDeviceContext()->VSSetConstantBuffers(1, 1, &m_pConstantBufferSkin);
	Dev::GetDeviceContext()->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	Dev::GetDeviceContext()->PSSetConstantBuffers(1, 1, &m_pConstantBufferSkin);


	//プリミティブ・トポロジーをセット  D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
	Dev::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//ボーンコンスタントバッファに入れる関数
	CMODEL_BONE_BUFFER bones;
	//bonesの初期化
	for (int i = 0; i < modle->m_bone_max; i++)
	{
		Math3D::IdentityMatrix(bones.m_mat[i]);
	}
	//bonesに各ボーンの姿勢行列とアニメーション行列から現在の姿勢を求める

	//ボーンに親子関係を求める

	//ボーン用のコンスタントバッファをGPUに送る
	D3D11_MAPPED_SUBRESOURCE pData;
	if (SUCCEEDED(Dev::GetDeviceContext()->Map(m_pConstantBufferSkin, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
	{
		memcpy_s(pData.pData, pData.RowPitch, (void*)&bones, sizeof(CMODEL_BONE_BUFFER));
		Dev::GetDeviceContext()->Unmap(m_pConstantBufferSkin, 0);
	}

	for (int i = 0; i < modle->m_material_max; i++)
	{
		//バーテックスバッファ登録
		UINT stride = sizeof(CPOINT3D_SKIN_LAYOUT);
		UINT offset = 0;
		Dev::GetDeviceContext()->IASetVertexBuffers(0, 1, &modle->m_ppVertexBuffer[i], &stride, &offset);
		//インデックスバッファ登録
		Dev::GetDeviceContext()->IASetIndexBuffer(modle->m_ppIndexBuffer[i], DXGI_FORMAT_R16_UINT, 0);

		//コンスタントバッファのデータ登録
		D3D11_MAPPED_SUBRESOURCE pData;
		if (SUCCEEDED(Dev::GetDeviceContext()->Map(m_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
		{
			CMODEL3D_BUFFER data;

			//トランスフォーム行列情報コンスタントバッファに渡す
			if (mat == nullptr)
			{
				Math3D::IdentityMatrix(data.m_mat);//ない場合は単位行列
			}
			else
			{
				memcpy(data.m_mat, mat, sizeof(float) * 16);
			}

			//法線用のワールド行列を渡す
			if (mat_w == nullptr)
			{
				Math3D::IdentityMatrix(data.m_mat_w);//ない場合は単位行列を渡す
			}
			else
			{
				memcpy(data.m_mat_w, mat_w, sizeof(float) * 16);
			}

			//視野方向ベクトルを渡す
			if (v_eye == nullptr)
			{
				memset(data.m_eye, 0x00, sizeof(data.m_eye));
			}
			else
			{
				data.m_eye[0] = v_eye[0];
				data.m_eye[1] = v_eye[1];
				data.m_eye[2] = v_eye[2];
				data.m_eye[3] = 1.0f;
			}

			//平行ライトの値を渡す
			memcpy(data.m_light_vec, m_light_vector, sizeof(m_light_vector));
			//点ライトの値を渡す
			memcpy(data.m_light_pos, m_light_pos, sizeof(m_light_pos));

			//材質　アンビエントを渡す
			memcpy(data.m_ambient, modle->m_Material[i].m_ambient, sizeof(data.m_ambient));
			//材質　デフィーズを渡す
			memcpy(data.m_diffuse, modle->m_Material[i].m_diffuse, sizeof(data.m_diffuse));
			//材質　エミッシブを渡す
			memcpy(data.m_emissive, modle->m_Material[i].m_emissive, sizeof(data.m_emissive));
			//材質　スペキュラを渡す
			memcpy(data.m_specular, modle->m_Material[i].m_specular, sizeof(data.m_specular));
			for (int j = 0; j < 4; j++)
			{
				data.m_specular_power[j] = modle->m_Material[i].m_specular_power;
			}

			memcpy_s(pData.pData, pData.RowPitch, (void*)&data, sizeof(CMODEL3D_BUFFER));
			//コンスタントバッファをシェーダに輸送
			Dev::GetDeviceContext()->Unmap(m_pConstantBuffer, 0);
		}

		//テクスチャーサンプラを登録
		Dev::GetDeviceContext()->PSSetSamplers(0, 1, Draw::GetSamplerState());
		//テクスチャを登録
		Dev::GetDeviceContext()->PSSetShaderResources(0, 1, &modle->m_Material[i].m_pTexture);

		//登録した情報を元にポリゴン描画
		Dev::GetDeviceContext()->DrawIndexed(modle->m_pindex_size[i] * 3, 0, 0);

	}

}


//平行光源の向きを入れる
void CRender3D::SetLightVec(float x, float y, float z, bool light_on)
{
	//ライトの光源方向
	m_light_vector[0] = x;
	m_light_vector[1] = y;
	m_light_vector[2] = z;

	//light_onでライトの有無を決めるようにする
	if (light_on == true)
	{
		m_light_vector[3] = 1.0f;//ライト有
	}
	else
	{
		m_light_vector[3] = 0.0f;//ライト無
	}
}

//点光源の位置と出力幅を入れる
void CRender3D::SetLightPos(float x, float y, float z, float max)
{
	m_light_pos[0] = x;
	m_light_pos[1] = y;
	m_light_pos[2] = z;
	m_light_pos[3] = max;//光に届く最大距離
}