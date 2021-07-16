#include "Render3D.h"
#include "DeviceCreate.h"
#include "Math3D.h"

ID3D11VertexShader*   CRender3D::m_pVertexShader;   //バーテックスシェーダー
ID3D11PixelShader*    CRender3D::m_pPixelShader;    //ピクセルシェーダー
ID3D11InputLayout*    CRender3D::m_pVertexLayout;   //頂点入力レイアウト
ID3D11Buffer*         CRender3D::m_pConstantBuffer; //コンスタントバッファ

//HLSLソースコード（メモリ内登録）
const char* g_hlsl_sause_code =
{
	//CPUから取得する頂点情報構造体
	"struct vertexIn           \n"
    "{                            \n"
    "	float4 pos : POSITION;    \n"
    "	float4 col : COLOR;       \n"
    "	float4 Nor : NORMAL;      \n"
    "   float2 uv  : UV;\n        \n"
    "};                           \n"

     //VSからPSに送る情報
    "struct vertexOut             \n"
    "{                            \n"              
	"  float4 pos : SV_POSITION;  \n"
	"  float4 col : COLOR;        \n"
	"  float2 uv  : UV;           \n"
	"};                           \n"

	//コンスタントバッファ受取先
	"cbuffer global               \n"
	"{                            \n"
	"  float4x4 mat;              \n"
	"};                           \n"

	//頂点シェーダ
   "vertexOut vs(vertexIn IN)               \n"
   "{                                       \n" 
	"vertexOut OUT;                         \n"
	"OUT.pos = mul(IN.pos,transpose(mat));  \n"
	"OUT.col = IN.col;                      \n"
	"OUT.uv  = IN.uv;                       \n"
	"return OUT;                            \n"
    "}                                      \n"	

	//ピクセルシェーダ
   "float4 ps(vertexOut IN) :SV_Target   \n"
   "{                                    \n"
   " float4 col = IN.col;                \n"
   " return col;                         \n"
   "}                                    \n"
};






void CRender3D::Init()
{
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
void CRender3D::Render(CMODEL* modle,float mat[16])
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
	//バーテックスバッファ登録
	UINT stride = sizeof(CPOINT3D_LAYOUT);
	UINT offset = 0;
	Dev::GetDeviceContext()->IASetVertexBuffers(0, 1, &modle->m_pVertexBuffer, &stride, &offset);
	//インデックスバッファ登録
	Dev::GetDeviceContext()->IASetIndexBuffer(modle->m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

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
		memcpy_s(pData.pData, pData.RowPitch, (void*)&data, sizeof(CMODEL3D_BUFFER));
		//コンスタントバッファをシェーダに輸送
		Dev::GetDeviceContext()->Unmap(m_pConstantBuffer, 0);
	}

	//登録した情報を元にポリゴン描画
	Dev::GetDeviceContext()->DrawIndexed(modle->m_index_size*3, 0, 0);
}