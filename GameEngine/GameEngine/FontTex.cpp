#include "FontTex.h"


HFONT           CFontTex::m_hFont;     //フォントハンドル：論理フォント（GDIオブジェクト）
HDC             CFontTex::m_hdc;       //ディスプレイデバイスコンテキストのハンドル
HFONT           CFontTex::m_oldFont;   //フォントハンドル：物理フォント（GDIオブジェクト）
TEXTMETRIC      CFontTex::m_TM;        //フォント情報格納用
list<unique_ptr<CCherClass>> CFontTex::list_char_tex;//文字リスト

//-----CCherClass---------

//文字テクスチャ作成メソッド
void CCherClass::CreateCharTex(wchar_t c)
{

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