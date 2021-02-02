//CPUから取得する頂点情報構造体
struct vertexIn
{
	float4 pos : POSITION;
	float4 col : COLOR;
	float2  uv  : UV;
};

//VSからPSに送る情報
struct vertexOut
{
	//頂点情報はラスタライズで使用
	float4 pos : SV_POSITION;
	//以下PSで使用する
	float4 col : COLOR;
	float2 uv  : UV;
};

//グローバル
cbuffer global
{
	float4 color;
	float4 pos;
	float4 scale;
};

//テクスチャ情報
Texture2D      txDiffuse    :register(t0);//テクスチャのグラフィック
SamplerState   samLinear    :register(s0);//テクスチャサンプラ

//頂点シェーダ
//引数 vertexIn   :CPUから受け取る頂点情報
//戻り値 vertexOut:PSに送る情報
//頂点情報を座標変換させるが今回は変換させていない。
//CPUから受け取ったデータをそのまま流している
vertexOut vs(vertexIn IN)
{
	vertexOut OUT;

	//拡大率
	OUT.pos.x = IN.pos.x * scale.x;
	OUT.pos.y = IN.pos.y * scale.y;

	//2D座標への変換
	OUT.pos.x = +(OUT.pos.x * (256.0f / 400.0f)) - 1.0f;
	OUT.pos.y = -(OUT.pos.y * (256.0f / 300.0f)) + 1.0f;

	//平行移動
	OUT.pos.x += (pos.x / 400.0f);
	OUT.pos.y += (-pos.y / 300.0f);

	//残りはINからOUTへそのまま流す
	OUT.pos.zw = IN.pos.zw; //頂点
	OUT.col = IN.col; //色
	OUT.uv  = IN.uv;  //UV

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
	//UVからテクスチャの色の値を取得
	float4 tex = txDiffuse.Sample(samLinear, IN.uv);

	//colにテクスチャの色合成
	col *= tex;
	//col.a = 1.0f;

return col;
}