//CPUから取得する頂点情報構造体
struct vertexIn
{
	float4 pos : POSITION;
	float4 col : COLOR;
};

//VSからPSに送る情報
struct vertexOut
{
	float4 pos : SV_POSITION;
	//以下PSで使用する
	float4 col : COLOR;
};

//グローバル
cbuffer global
{
	float4 color;
};

//頂点シェーダ
//引数 vertexIn   :CPUから受け取る頂点情報
//戻り値 vertexOut:PSに送る情報
//頂点情報を座標変換させるが今回は変換させていない。
vertexOut vs(vertexIn IN)
{
	vertexOut OUT;

	OUT.pos = IN.pos;
	OUT.col = IN.col;

	return OUT;
}

//ピクセルシェーダ
//引数　　vertexOut:VSから送られてきた情報
//戻り値　float4:Color値
//引数の情報元に色を決める。今回は頂点が持つ色とグローバル
//から持ってきた色を乗算してる
float4 ps(vertexOut IN) :SV_Target
{
	float4 col = IN.col * color;
return col;
}