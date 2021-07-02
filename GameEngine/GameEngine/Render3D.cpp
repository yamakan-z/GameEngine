#include "Render3D.h"
#include "DeviceCreate.h"

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

	//頂点シェーダ
   "vertexOut vs(vertexIn IN)      \n"
   "{                              \n" 
	"vertexOut OUT;                \n"
	"OUT.pos = IN.pos;             \n"
	"OUT.col = IN.col;             \n"
	"OUT.uv  = IN.uv;              \n"
	"return OUT;                   \n"
    "}                             \n"	

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
}

void CRender3D::Delete()
{

}