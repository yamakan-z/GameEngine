#include "FontTex.h"


HFONT           CFontTex::m_hFont;     //フォントハンドル：論理フォント（GDIオブジェクト）
HDC             CFontTex::m_hdc;       //ディスプレイデバイスコンテキストのハンドル
HFONT           CFontTex::m_oldFont;   //フォントハンドル：物理フォント（GDIオブジェクト）
TEXTMETRIC      CFontTex::m_TM;        //フォント情報格納用
list<unique_ptr<CCherClass>> CFontTex::list_char_tex;//文字リスト

//-----CCherClass---------

//文字テクスチャ作成メソッド
void CCherClass::CreateCharTex(wchar_t c,HDC hdc,TEXTMETRIC TM)
{
	//識別文字用
	UINT code = 0;//作成する文字コード

	//文字フォント描画用
	BYTE* ptr;      //文字のグラフィック（ビットマップ）を入れる場所
	DWORD size;     //文字を表現するのに必要なメモリの大きさ
	GLYPHMETRICS GM;//象形文字の情報が格納
	const MAT2 Mat = { {0,1},{0,0},{0,0},{0,1} };//フォント書き込み向き

	//テクスチャ描きこみ用ポインタ
	D3D11_MAPPED_SUBRESOURCE mapped;//リソースにアクセスするポインタ
	BYTE* pBits;                    //テクスチャのピクセル情報入れるポインタ

	//識別文字コード登録
	m_pc.reset(new wchar_t(c));
	code = (UINT)*m_pc.get();

	//フォント情報から文字のビットマップ取得
	//文字のビットマップの大きさを取得
	size = GetGlyphOutline(hdc, code, GGO_GRAY4_BITMAP, &GM, 0, NULL, &Mat);
	ptr = new BYTE[size];
	//文字のビットマップ情報をptrに入れる
	GetGlyphOutline(hdc, code, GGO_GRAY4_BITMAP, &GM, size, ptr, &Mat);

	//空テクスチャの設定
	D3D11_TEXTURE2D_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;             //テクスチャフォーマットR8G8B8の24bit
	desc.SampleDesc.Count = 1;                            //サンプリングは1ピクセルのみ
	desc.Usage = D3D11_USAGE_DYNAMIC;                     //CPU書き込み可能
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;          //シェーダリソース
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;         //CPUから書き込みアクセス可
	desc.Height = 32;                                     //縦のサイズ
	desc.Width = 32;                                      //横のサイズ

	//設定を元に空テクスチャを作成
	Dev::GetDevice()->CreateTexture2D(&desc, 0, &m_pTexture);

	//テクスチャ情報を取得する
	D3D11_TEXTURE2D_DESC texDesc;
	m_pTexture->GetDesc(&texDesc);
}

//-----CFontTex-----------

//初期化メソッド
void CFontTex::InitFontTex()
{

	//リスト初期化
	list_char_tex.clear();


	//第一引数のカテゴリに、第二引数の国別コードに影響を与える
	//この場合、Unicodeの文字を日本コードにするとなる
	setlocale(LC_CTYPE, "jpn");

	//論理フォント設定
	HFONT hFont = CreateFont(
		32,
		0,0,0,0,
		FALSE,FALSE,FALSE,
		SHIFTJIS_CHARSET,
		OUT_TT_ONLY_PRECIS,
		CLIP_DEFAULT_PRECIS,
		PROOF_QUALITY,
		FIXED_PITCH|FF_MODERN,
		L"MS ゴシック"//使用フォント
	);

	//指定されたウィンドウのクライアント領域または画面全体を表す
	//ディスプレイデバイスコンテキストのハンドルを取得
	m_hdc = GetDC(NULL);

	//Windowsは、SelectObjectでデバイスコンテキストにする。
	//これを設定すると論理フォントに最も近い物理フォントをセットします
	m_oldFont = (HFONT)SelectObject(m_hdc, hFont);

	//現在選択されているフォントの情報を指定されたバッファに格納
	GetTextMetrics(m_hdc, &m_TM);

	//論理フォントをメンバに渡す
	m_hFont = hFont;
}

//削除メソッド
void CFontTex::DeleteFontTex()
{
	//リスト破棄
	list_char_tex.clear();

	//これらGDIオブジェクトを破棄
	DeleteObject(m_oldFont);
	DeleteObject(m_hFont);
	//ディスプレイデバイスコンテキストハンドル解放
	ReleaseDC(NULL, m_hdc);
}