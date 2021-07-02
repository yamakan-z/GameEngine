#pragma once
//Device作成に必要なヘッダー
#include <Windows.h>
#include <D3D11.h>

#include <d3dCompiler.h>


typedef class CDeviceCreate
{
public:
	CDeviceCreate(){}
	~CDeviceCreate(){}

	static HRESULT APIENTRY InitDevice(HWND hWnd, int w, int h);//デバイスの初期化
	static void ShutDown();                                     //終了関数
	static void Init3DEnvironment();  //3D環境の構築
	static void Delete3DEnvironment();//3D環境破棄

	//外部で使用する変数
	static ID3D11Device*            GetDevice()         { return m_pDevice; }
	static ID3D11DeviceContext*     GetDeviceContext()  { return m_pDeviceContext; }
	static ID3D11RasterizerState*   GetRS()             { return m_pRS; }
	static ID3D11RenderTargetView*  GetRTV()            { return m_pRTV; }
	static ID3D11RenderTargetView** GetppRTV()          { return &m_pRTV; }
	static IDXGISwapChain*          GetSwapChain()      { return m_pDXGISwapChain; }
	static ID3D11RasterizerState*   GetRS3D()           { return m_pRS3D; }
	static ID3D11DepthStencilView*  GetDSV()            { return m_pDSV; }


private:
	static ID3D11Device*                 m_pDevice;              //   D3D11デバイス
	static ID3D11DeviceContext*          m_pDeviceContext;       //   D3D11デバイスコンテキスト
	static ID3D11RasterizerState*        m_pRS;                  //   D3D11ラスターライザー
	static ID3D11RenderTargetView*       m_pRTV;                 //   D3D11レンダーターゲット
	static ID3D11BlendState*             m_pBlendState;          //   D3D11ブレンドステータス
	static IDXGIAdapter*                 m_pDXGIAdapter;         //   DXGIアダプター
	static IDXGIFactory*                 m_pDXGIFactory;         //   DXGIファクトリー
	static IDXGISwapChain*               m_pDXGISwapChain;       //   DXGIスワップチェーン
	static IDXGIOutput**                 m_ppDXGIOutputArray;    //   DXGI出力群
	static UINT                          m_nDXGIOutputArraySize; //   DXGI出力群サイズ
	static IDXGIDevice1*                 m_pDXGIDevice;          //   DXGIデバイス
	static D3D_FEATURE_LEVEL             m_FeatureLevel;         //   D3D機能レベル

	//3D環境に必要なもの
	static ID3D11RasterizerState*    m_pRS3D;//D3D11 3D用のラスタライザー
	static ID3D11DepthStencilView*   m_pDSV; //D3D11 深度ステンシルビュー
}Dev;