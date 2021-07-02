#include "Render3D.h"
#include "DeviceCreate.h"

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
}

void CRender3D::Delete()
{

}