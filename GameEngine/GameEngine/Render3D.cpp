#include "Render3D.h"
#include "DeviceCreate.h"


ID3D11VertexShader*   CRender3D::m_pVertexShader;//�o�[�e�b�N�X�V�F�[�_�[
ID3D11PixelShader*    CRender3D::m_pPixelShader; //�s�N�Z���V�F�[�_�[
ID3D11InputLayout*    CRender3D::m_pVertexLayout;//���_���̓��C�A�E�g

//HLSL�\�[�X�R�[�h�i���������o�^�j
const char* g_hlsl_sause_code =
{
	//CPU����擾���钸�_���\����
	"struct vertexIn           \n"
    "{                            \n"
    "	float4 pos : POSITION;    \n"
    "	float4 col : COLOR;       \n"
    "	float4 Nor : NORMAL;      \n"
    "   float2 uv  : UV;\n        \n"
    "};                           \n"

     //VS����PS�ɑ�����
    "struct vertexOut             \n"
    "{                            \n"              
	"  float4 pos : SV_POSITION;  \n"
	"  float4 col : COLOR;        \n"
	"  float2 uv  : UV;           \n"
	"};                           \n"

	//���_�V�F�[�_
   "vertexOut vs(vertexIn IN)      \n"
   "{                              \n" 
	"vertexOut OUT;                \n"
	"OUT.pos = IN.pos;             \n"
	"OUT.col = IN.col;             \n"
	"OUT.uv  = IN.uv;              \n"
	"return OUT;                   \n"
    "}                             \n"	

	//�s�N�Z���V�F�[�_
   "float4 ps(vertexOut IN) :SV_Target   \n"
   "{                                    \n"
   " float4 col = IN.col;                \n"
   " return col;                         \n"
   "}                                    \n"
};






void CRender3D::Init()
{
	HRESULT hr = S_OK;
	//hlsl�t�@�C���ǂݍ��݁@�u���u�쐬�@�u���u�Ƃ̓V�F�[�_�[�̉�݂����Ȃ���
	//XX�V�F�[�_�[�Ƃ��ē����������Ȃ��B��Ŋe��V�F�[�_�[�ƂȂ�
	ID3DBlob* pCompiledShader = NULL;
	ID3DBlob* pErrors = NULL;

	//����������HLSL��vs�֐������̃R���p�C��
	hr = D3DCompile(g_hlsl_sause_code, strlen(g_hlsl_sause_code), 0, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"vs", "vs_4_0", 0, 0, &pCompiledShader, &pErrors);

	if (FAILED(hr))
	{
		//�G���[������ꍇ�A�����f�o�b�O��������
		char* c = (char*)pErrors->GetBufferPointer();
		MessageBox(0, L"3Dhlsl�ǂݍ��ݎ��s1", NULL, MB_OK);
		SAFE_RELEASE(pErrors);
		return;
	}

	//�R���p�C�������o�[�e�b�N�X�V�F�[�_�[�����ɃC���^�[�t�F�[�X���쐬
	hr = Dev::GetDevice()->CreateVertexShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(),
		NULL, &m_pVertexShader);
	if (FAILED(hr))
	{
		SAFE_RELEASE(pCompiledShader);
		MessageBox(0, L"�o�[�e�b�N�X�V�F�[�_�[�쐬���s", NULL, MB_OK);
		return;
	}
	//���_�C���v�b�g���C�A�E�g���`
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT   ,0,0, D3D11_INPUT_PER_VERTEX_DATA,0},
		{"NORMAL",  0,DXGI_FORMAT_R32G32B32_FLOAT,   0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"UV"   ,   0,DXGI_FORMAT_R32G32_FLOAT,      0,28,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"COLOR",   0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,32,D3D11_INPUT_PER_VERTEX_DATA,0},
	};
	UINT numElements = sizeof(layout) / sizeof(layout[0]);

	//���_�C���v�b�g���C�A�E�g���쐬�E���C�A�E�g���Z�b�g
	hr = Dev::GetDevice()->CreateInputLayout(layout, numElements, pCompiledShader->GetBufferPointer(),
		pCompiledShader->GetBufferSize(), &m_pVertexLayout);
	if (FAILED(hr))
	{
		MessageBox(0, L"���C�A�E�g�쐬���s", NULL, MB_OK);
		return;
	}
	SAFE_RELEASE(pCompiledShader);

	//����������HLSL��ps�֐��������R���p�C��
	hr = D3DCompile(g_hlsl_sause_code,strlen(g_hlsl_sause_code),0,0,D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"ps", "ps_4_0", 0, 0, &pCompiledShader, &pErrors);
	if (FAILED(hr))
	{
		//�G���[������ꍇ�A�����f�o�b�N��������
		char* c = (char*)pErrors->GetBufferPointer();
		MessageBox(0, L"hlsl�ǂݍ��ݎ��s2", NULL, MB_OK);
		SAFE_RELEASE(pErrors);
		return;
	}

	//�R���p�C�������s�N�Z���V�F�[�_�[�ŃC���^�[�t�F�[�X���쐬
	hr = Dev::GetDevice()->CreatePixelShader(pCompiledShader->GetBufferPointer(),
		pCompiledShader->GetBufferSize(), NULL, &m_pPixelShader);
	if (FAILED(hr))
	{
		SAFE_RELEASE(pCompiledShader);
		MessageBox(0, L"�s�N�Z���V�F�[�_�[�쐬���s", NULL, MB_OK);
		return;
	}
	SAFE_RELEASE(pCompiledShader);
}

void CRender3D::Delete()
{
	SAFE_RELEASE(m_pPixelShader);   //�s�N�Z���V�F�[�_�[�j��
	SAFE_RELEASE(m_pVertexLayout);  //���_���̓��C�A�E�g�j��
	SAFE_RELEASE(m_pVertexShader);  //�o�[�e�b�N�X�V�F�[�_�[
}

//���f���������_�����O����
void CRender3D::Render(CMODEL* modle)
{
	//���_���C�A�E�g
	Dev::GetDeviceContext()->IASetInputLayout(m_pVertexLayout);

	//�g�p����V�F�[�_�[�̓o�^
	Dev::GetDeviceContext()->VSSetShader(m_pVertexShader, NULL, 0);
	Dev::GetDeviceContext()->PSSetShader(m_pPixelShader,  NULL, 0);

	//�v���~�e�B�u�E�g�|���W�[���Z�b�g
	Dev::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//�o�[�e�b�N�X�o�b�t�@�o�^
	UINT stride = sizeof(CPOINT3D_LAYOUT);
	UINT offset = 0;
	Dev::GetDeviceContext()->IASetVertexBuffers(0, 1, &modle->m_pVertexBuffer, &stride, &offset);
	//�C���f�b�N�X�o�b�t�@�o�^
	Dev::GetDeviceContext()->IASetIndexBuffer(modle->m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	//�o�^�����������Ƀ|���S���`��
	Dev::GetDeviceContext()->DrawIndexed(modle->m_index_size, 0, 0);
}