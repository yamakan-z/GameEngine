#pragma once
//Device�쐬�ɕK�v�ȃw�b�_�[
#include <Windows.h>
#include <D3D11.h>

#include <d3dCompiler.h>


typedef class CDeviceCreate
{
public:
	CDeviceCreate(){}
	~CDeviceCreate(){}

	static HRESULT APIENTRY InitDevice(HWND hWnd, int w, int h);//�f�o�C�X�̏�����
	static void ShutDown();                                     //�I���֐�

	//�O���Ŏg�p����ϐ�
	static ID3D11Device*            GetDevice()         { return m_pDevice; }
	static ID3D11DeviceContext*     GetDeviceContext()  { return m_pDeviceContext; }
	static ID3D11RasterizerState*   GetRS()             { return m_pRS; }
	static ID3D11RenderTargetView*  GetRTV()            { return m_pRTV; }
	static ID3D11RenderTargetView** GetppRTV()          { return &m_pRTV; }
	static IDXGISwapChain*          GetSwapChain()      { return m_pDXGISwapChain; }


private:
	static ID3D11Device*                 m_pDevice;              //   D3D11�f�o�C�X
	static ID3D11DeviceContext*          m_pDeviceContext;       //   D3D11�f�o�C�X�R���e�L�X�g
	static ID3D11RasterizerState*        m_pRS;                  //   D3D11���X�^�[���C�U�[
	static ID3D11RenderTargetView*       m_pRTV;                 //   D3D11�����_�[�^�[�Q�b�g
	static ID3D11BlendState*             m_pBlendState;          //   D3D11�u�����h�X�e�[�^�X
	static IDXGIAdapter*                 m_pDXGIAdapter;         //   DXGI�A�_�v�^�[
	static IDXGIFactory*                 m_pDXGIFactory;         //   DXGI�t�@�N�g���[
	static IDXGISwapChain*               m_pDXGISwapChain;       //   DXGI�X���b�v�`�F�[��
	static IDXGIOutput**                 m_ppDXGIOutputArray;    //   DXGI�o�͌Q
	static UINT                          m_nDXGIOutputArraySize; //   DXGI�o�͌Q�T�C�Y
	static IDXGIDevice1*                 m_pDXGIDevice;          //   DXGI�f�o�C�X
	static D3D_FEATURE_LEVEL             m_FeatureLevel;         //   D3D�@�\���x��
}Dev;