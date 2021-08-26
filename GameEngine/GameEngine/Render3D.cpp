#include "Render3D.h"
#include "DeviceCreate.h"
#include "Math3D.h"

float CRender3D::m_light_vector[4];//平行光源（方向）
float CRender3D::m_light_pos[4];//点光源（位置）

ID3D11VertexShader*   CRender3D::m_pVertexShader;   //バーテックスシェーダー
ID3D11PixelShader*    CRender3D::m_pPixelShader;    //ピクセルシェーダー
ID3D11InputLayout*    CRender3D::m_pVertexLayout;   //頂点入力レイアウト
ID3D11Buffer*         CRender3D::m_pConstantBuffer; //コンスタントバッファ

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
	"cbuffer global               \n"
	"{                            \n"
	"  float4x4 mat;              \n"//ビューパイプライントランスフォーム用
	"  float4x4 w_mat;            \n"//法線用のワールドトランスフォーム用
	"  float4 l_vec;              \n"//平行ライト用ベクトル
	"  float4 l_pos;              \n"//点ライト用ポジション
	"  float4 amb;                \n"//アンビエント
	"  float4 diff;               \n"//デフィーズ
	"};                           \n"

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
   "  col=col*d+a;                                                     \n"//色=ポリゴン色*ディフィーズ色+アンビエント色
   "  col=saturate(col);                                               \n"//求めた色を0.0～1.0を超えないようにする
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
		{"UV"   ,   0,DXGI_FORMAT_R32G32_FLOAT,      0,28,D3D11_INPUT_PER_VERTEX_DATA,0},
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
	SAFE_RELEASE(m_pConstantBuffer);//コンスタントバッファ破棄
	SAFE_RELEASE(m_pPixelShader);   //ピクセルシェーダー破棄
	SAFE_RELEASE(m_pVertexLayout);  //頂点入力レイアウト破棄
	SAFE_RELEASE(m_pVertexShader);  //バーテックスシェーダー
}

//モデルをレンダリングする
void CRender3D::Render(CMODEL* modle,float mat[16],float mat_w[16])
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

			//平行ライトの値を渡す
			memcpy(data.m_light_vec, m_light_vector, sizeof(m_light_vector));
			//点ライトの値を渡す
			memcpy(data.m_light_pos, m_light_pos, sizeof(m_light_pos));

			//材質　アンビエントを渡す
			memcpy(data.m_ambient, modle->m_Material[i].m_ambient, sizeof(data.m_ambient));
			//材質　デフィーズを渡す
			memcpy(data.m_diffuse, modle->m_Material[i].m_diffuse, sizeof(data.m_diffuse));

			memcpy_s(pData.pData, pData.RowPitch, (void*)&data, sizeof(CMODEL3D_BUFFER));
			//コンスタントバッファをシェーダに輸送
			Dev::GetDeviceContext()->Unmap(m_pConstantBuffer, 0);
		}

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