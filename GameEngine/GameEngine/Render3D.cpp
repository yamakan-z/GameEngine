#include "Render3D.h"
#include "DeviceCreate.h"
#include "Math3D.h"

float CRender3D::m_light_vector[4];//���s�����i�����j
float CRender3D::m_light_pos[4];//�_�����i�ʒu�j

ID3D11VertexShader*   CRender3D::m_pVertexShader;   //�o�[�e�b�N�X�V�F�[�_�[
ID3D11PixelShader*    CRender3D::m_pPixelShader;    //�s�N�Z���V�F�[�_�[
ID3D11InputLayout*    CRender3D::m_pVertexLayout;   //���_���̓��C�A�E�g
ID3D11Buffer*         CRender3D::m_pConstantBuffer; //�R���X�^���g�o�b�t�@

//HLSL�\�[�X�R�[�h�i���������o�^�j
const char* g_hlsl_sause_code =
{
	//CPU����擾���钸�_���\����
	"struct vertexIn              \n"
	"{                            \n"
	"   float4 pos : POSITION;    \n"
	"   float4 col : COLOR;       \n"
	"   float4 Nor : NORMAL;      \n"
	"   float2 uv  : UV;          \n"
	"};                           \n"

	//VS����PS�ɑ�����
   "struct vertexOut                \n"
   "{                               \n"
   "  float4 pos  : SV_POSITION;    \n"
   "  float4 col  : COLOR;          \n"
   "  float2 uv   : UV;             \n"
   "  float3 nor  : NORMAL;         \n"//���[���h�s��~�@��
   "  float3 pos_c: POSITION_COLOR; \n"//���[���h�s��~�ʒu
   "};                              \n"

	//�R���X�^���g�o�b�t�@����
	"cbuffer global               \n"
	"{                            \n"
	"  float4x4 mat;              \n"//�r���[�p�C�v���C���g�����X�t�H�[���p
	"  float4x4 w_mat;            \n"//�@���p�̃��[���h�g�����X�t�H�[���p
	"  float4 l_vec;              \n"//���s���C�g�p�x�N�g��
	"  float4 l_pos;              \n"//�_���C�g�p�|�W�V����
	"  float4 amb;                \n"//�A���r�G���g
	"  float4 diff;               \n"//�f�t�B�[�Y
	"};                           \n"

	//���_�V�F�[�_
   "vertexOut vs(vertexIn IN)                              \n"
   "{                                                      \n" 
	"vertexOut OUT;                                        \n"
	"OUT.pos   = mul(IN.pos,transpose(mat));               \n"
	"OUT.col   = IN.col;                                   \n"
	"OUT.uv    = IN.uv;                                    \n"
	"OUT.nor   = mul(IN.Nor,(float3x3)transpose(w_mat));   \n"//nor=w_mat[3�~3]*�@��(x,y,z)   
	"OUT.pos_c = mul(IN.pos,transpose(w_mat));             \n"//pos=w_mat[4�~4]*�ʒu(x,y,z)
	"return OUT;                                           \n"
    "}                                                     \n"	

	//�s�N�Z���V�F�[�_
   "float4 ps(vertexOut IN) :SV_Target                                 \n"
   "{                                                                  \n"
   "   float4 col = IN.col;                                            \n"//���_��color�����擾
   "   float4 light_vec_sc=(float4)1.0f;                               \n"//���s���C�g�p�̉A�e���ʂ�����ϐ�
   "   float4 light_pos_sc=(float4)1.0f;                               \n"//�_���C�g�p�̉A�e���ʂ�����ϐ�
   "                                                                   \n"
   "   if(l_vec.w != 0.0f && any(IN.nor)==true)                        \n"//l_vec.w��0�ł���Ε��s�̃��C�g�v�Z�͂��Ȃ�
   "   {                                                               \n"//�܂��A�@�����Ȃ��ꍇ���A�v�Z���Ȃ�
   "     light_vec_sc.rgb = dot(normalize(IN.nor),normalize(-l_vec));  \n"//�@����-���������œ��ς����߉A�e��RGB�ɓ����
   "     light_vec_sc=saturate(light_vec_sc);                          \n"//light_vec_sc�̒l���i0�`1�j�܂łɂ���
   "   }                                                               \n"
   "                                                                   \n"
   "   if(l_pos.w > 0.0f && any(IN.nor)==true)                         \n"//l_pos.w��0�ȉ��ł���Γ_���C�g�v�Z�͂��Ȃ�
   "   {                                                               \n"
   "     float3 lp_len = l_pos.xyz - IN.pos_c;                         \n"//�_�����ƒ��_�̊e�s�N�Z���̈ʒu����x�N�g�������߂�
   "     float  len = length(lp_len);                                  \n"//���߂��x�N�g���̒��������߂�
   "     float  w = saturate(len/l_pos.w);                             \n"//�x�N�g���̒����Əo�͕������ŏo��
   "     light_pos_sc.rgb = dot(normalize(IN.nor),normalize(lp_len));  \n"//�@����lp_len����A�e�������߂�
   "     light_pos_sc.rgb = light_pos_sc.rgb*(float3)1.0-w;            \n"//�A�e�ƌ��̋������������ē_�����̉A�e�Ƃ���
   "   }                                                               \n"
   "  float4 d=(float4)1.0f;                                           \n"
   "  if(any(diff)==true)                                              \n"//�f�B�t�B�[�Y�v�Z�L��
   "  {                                                                \n"
   "    d=max(light_pos_sc,light_vec_sc)*diff;                         \n"//�_�E�x�N�g�������̒��Ŗ��邢�A�e��
   "  }                                                                \n"//�f�B�t�B�[�Y�F����������
   "                                                                   \n"
   "  float4 a=(float4)0.0f;                                           \n"
   "  if(any(amb)==true)                                               \n"//�A���r�G���g�̗L��
   "  {                                                                \n"
   "    a=amb;                                                         \n"//�A���r�G���g�l����
   "  }                                                                \n"
   "  col=col*d+a;                                                     \n"//�F=�|���S���F*�f�B�t�B�[�Y�F+�A���r�G���g�F
   "  col=saturate(col);                                               \n"//���߂��F��0.0�`1.0�𒴂��Ȃ��悤�ɂ���
   "  return col;                                                      \n"//�ŏI�I�ȏo��
   "}                                                                  \n"
};






void CRender3D::Init()
{

	//���s���C�g�l�̏�����
	memset(m_light_vector, 0x00, sizeof(m_light_vector));
	//�_���C�g
	memset(m_light_pos, 0x00, sizeof(m_light_pos));

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

	//�o�b�t�@�ɃR���X�^���g�o�b�t�@�i�V�F�[�_�Ƀf�[�^�󂯓n���p�j�X�e�[�^�X��ݒ�
	D3D11_BUFFER_DESC cb;
	cb.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
	cb.ByteWidth           = sizeof(CMODEL3D_BUFFER);
	cb.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags           = 0;
	cb.StructureByteStride = 0;
	cb.Usage               = D3D11_USAGE_DYNAMIC;

	//�X�e�[�^�X�����ɃR���X�^���g�o�b�t�@���쐬
	hr = Dev::GetDevice()->CreateBuffer(&cb, NULL, &m_pConstantBuffer);
	if (FAILED(hr))
	{
		MessageBox(0, L"�R���X�^���g�o�b�t�@�쐬���s", NULL, MB_OK);
		return;
	}

}

//�j�����\�b�h
void CRender3D::Delete()
{
	SAFE_RELEASE(m_pConstantBuffer);//�R���X�^���g�o�b�t�@�j��
	SAFE_RELEASE(m_pPixelShader);   //�s�N�Z���V�F�[�_�[�j��
	SAFE_RELEASE(m_pVertexLayout);  //���_���̓��C�A�E�g�j��
	SAFE_RELEASE(m_pVertexShader);  //�o�[�e�b�N�X�V�F�[�_�[
}

//���f���������_�����O����
void CRender3D::Render(CMODEL* modle,float mat[16],float mat_w[16])
{
	//���_���C�A�E�g
	Dev::GetDeviceContext()->IASetInputLayout(m_pVertexLayout);

	//�g�p����V�F�[�_�[�̓o�^
	Dev::GetDeviceContext()->VSSetShader(m_pVertexShader, NULL, 0);
	Dev::GetDeviceContext()->PSSetShader(m_pPixelShader,  NULL, 0);

	//�R���X�^���g�o�b�t�@���g�p����V�F�[�_�ɓo�^
	Dev::GetDeviceContext()->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	Dev::GetDeviceContext()->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);


	//�v���~�e�B�u�E�g�|���W�[���Z�b�g
	Dev::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	for (int i = 0; i < modle->m_material_max; i++)
	{
		//�o�[�e�b�N�X�o�b�t�@�o�^
		UINT stride = sizeof(CPOINT3D_LAYOUT);
		UINT offset = 0;
		Dev::GetDeviceContext()->IASetVertexBuffers(0, 1, &modle->m_ppVertexBuffer[i], &stride, &offset);
		//�C���f�b�N�X�o�b�t�@�o�^
		Dev::GetDeviceContext()->IASetIndexBuffer(modle->m_ppIndexBuffer[i], DXGI_FORMAT_R16_UINT, 0);

		//�R���X�^���g�o�b�t�@�̃f�[�^�o�^
		D3D11_MAPPED_SUBRESOURCE pData;
		if (SUCCEEDED(Dev::GetDeviceContext()->Map(m_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
		{
			CMODEL3D_BUFFER data;

			//�g�����X�t�H�[���s����R���X�^���g�o�b�t�@�ɓn��
			if (mat == nullptr)
			{
				Math3D::IdentityMatrix(data.m_mat);//�Ȃ��ꍇ�͒P�ʍs��
			}
			else
			{
				memcpy(data.m_mat, mat, sizeof(float) * 16);
			}

			//�@���p�̃��[���h�s���n��
			if (mat_w == nullptr)
			{
				Math3D::IdentityMatrix(data.m_mat_w);//�Ȃ��ꍇ�͒P�ʍs���n��
			}
			else
			{
				memcpy(data.m_mat_w, mat_w, sizeof(float) * 16);
			}

			//���s���C�g�̒l��n��
			memcpy(data.m_light_vec, m_light_vector, sizeof(m_light_vector));
			//�_���C�g�̒l��n��
			memcpy(data.m_light_pos, m_light_pos, sizeof(m_light_pos));

			//�ގ��@�A���r�G���g��n��
			memcpy(data.m_ambient, modle->m_Material[i].m_ambient, sizeof(data.m_ambient));
			//�ގ��@�f�t�B�[�Y��n��
			memcpy(data.m_diffuse, modle->m_Material[i].m_diffuse, sizeof(data.m_diffuse));

			memcpy_s(pData.pData, pData.RowPitch, (void*)&data, sizeof(CMODEL3D_BUFFER));
			//�R���X�^���g�o�b�t�@���V�F�[�_�ɗA��
			Dev::GetDeviceContext()->Unmap(m_pConstantBuffer, 0);
		}

		//�o�^�����������Ƀ|���S���`��
		Dev::GetDeviceContext()->DrawIndexed(modle->m_pindex_size[i] * 3, 0, 0);

	}
	

	
}

//���s�����̌���������
void CRender3D::SetLightVec(float x, float y, float z, bool light_on)
{
	//���C�g�̌�������
	m_light_vector[0] = x;
	m_light_vector[1] = y;
	m_light_vector[2] = z;

	//light_on�Ń��C�g�̗L�������߂�悤�ɂ���
	if (light_on == true)
	{
		m_light_vector[3] = 1.0f;//���C�g�L
	}
	else
	{
		m_light_vector[3] = 0.0f;//���C�g��
	}
}

//�_�����̈ʒu�Əo�͕�������
void CRender3D::SetLightPos(float x, float y, float z, float max)
{
	m_light_pos[0] = x;
	m_light_pos[1] = y;
	m_light_pos[2] = z;
	m_light_pos[3] = max;//���ɓ͂��ő勗��
}