#include "Render3D.h"
#include "Draw2DPolygon.h"
#include "DeviceCreate.h"
#include "Math3D.h"

float CRender3D::m_light_vector[4];//���s�����i�����j
float CRender3D::m_light_pos[4];//�_�����i�ʒu�j

ID3D11VertexShader*   CRender3D::m_pVertexShader;   //�o�[�e�b�N�X�V�F�[�_�[
ID3D11PixelShader*    CRender3D::m_pPixelShader;    //�s�N�Z���V�F�[�_�[
ID3D11InputLayout*    CRender3D::m_pVertexLayout;   //���_���̓��C�A�E�g
ID3D11Buffer*         CRender3D::m_pConstantBuffer; //�R���X�^���g�o�b�t�@

ID3D11VertexShader*   CRender3D::m_pVertexShaderSkin;   //�X�L���p�o�[�e�b�N�X�V�F�[�_�[
ID3D11InputLayout*    CRender3D::m_pVertexLayoutSkin;   //�X�L���p���_���̓��C�A�E�g
ID3D11Buffer*         CRender3D::m_pConstantBufferSkin; //�X�L���p�R���X�^���g�o�b�t�@
 

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

	//CPU����擾����X�L���p���_���\����
	"struct vertex_skin_In          \n"
	"{                              \n"
	"  float4 pos : POSITION;       \n"
	"  float4 col : COLOR;          \n"
	"  float4 Nor : NORMAL;         \n"
	"  float2 uv  : UV;             \n"
	"  int4   bi  : BI;             \n"
	"  float4 we  : WE;             \n"
	"};                             \n"

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
	"cbuffer global  :  register( b0 ) \n"
	"{                                 \n"
	"  float4x4 mat;                   \n"//�r���[�p�C�v���C���g�����X�t�H�[���p
	"  float4x4 w_mat;                 \n"//�@���p�̃��[���h�g�����X�t�H�[���p
	"  float4 l_vec;                   \n"//���s���C�g�p�x�N�g��
	"  float4 l_pos;                   \n"//�_���C�g�p�|�W�V����
	"  float4 amb;                     \n"//�A���r�G���g
	"  float4 diff;                    \n"//�f�t�B�[�Y
	"  float4 emi;                     \n"//�G�~�b�V�u
	"  float4 sp;                      \n"//�X�y�L����
	"  float4 sp_p;                    \n"//�X�y�L�����p���[
	"  float4 eye;                     \n"//�������
	"};                                \n"

	//�{�[���p�R���X�^���g�o�b�t�@����
	"cbuffer global_bone : register( b1 )  \n"
	"{                                     \n"
	"  float4x4 b_mat[128];                \n"//�{�[���s��
	"};                                    \n"



	//�e�N�X�`�����
	"Texture2D      txDiffuse  : register( t0 );\n"//�e�N�X�`���̃O���t�B�b�N
	"SamplerState   samLinear  : register( s0 );\n"//�e�N�X�`���T���v��

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

	//���_�X�L���p�V�F�[�_
   "vertexOut vs_skin(vertex_skin_In IN)                    \n"
   "{                                                       \n"
   "  float4 pos=(float4)0.0f;                              \n"//�{�[���̉e���󂯂����_�̈ʒu
   "  float3 nor=(float3)0.0f;                              \n"//�{�[���̉e���󂯂����_�̖@��
   "                                                        \n"
   "  int   index[4] ={IN.bi.x,IN.bi.y,IN.bi.z,In.bi.w};    \n"//BoneID��int�^�Ȃ̂Ŕz��ɓ���Ȃ���
   "  float weight[4]={IN.we.x,IN.we.y,IN.we.z,IN.we.w};    \n"//Weight��float4�^�Ȃ̂Ŕz��ɓ���Ȃ���
   "                                                        \n"
   "  for(int i=0;i<4;i++)                                  \n"//�e�{�[���s��~�d�݂𒸓_�ɉ��Z���鏈��
   " {                                                      \n"
   "   float    w = weight[i];                              \n"//�E�F�C�g�l�擾
   "   float4x4 m = transpose(b_mat[index[i]]);             \n"//�{�[��ID����{�[���s����擾
   "   pos += w * mul(IN.pos,m);                            \n"//�X�L�������ʒu+���d�݁~�ʒu�~�{�[���s��
   "   nor += w * mul(IN.Nor,(float3x3)m);                  \n"//�X�L�������@��+���d�݁~�@���~�{�[���s��
   " }                                                      \n"
   "                                                        \n"//�r���[�C���O�p�C�v���C���n�̏���
   "  vertexOut OUT;                                        \n"
   "  OUT.pos   = mul(pos,transpose(mat));                  \n"//pos=wvp[4�~4]*�X�L�������ʒu(x,y,z)
   "  OUT.col   = IN.col;                                   \n"
   "  OUT.uv    = IN.uv;                                    \n"
   "  OUT.nor   = mul(nor,(float3x3)transpose(w_mat));      \n"//nor=w_mat[3�~3]*�@��(x,y,z)   
   "  OUT.pos_c = mul(pos,transpose(w_mat));                \n"//pos=w_mat[4�~4]*�ʒu(x,y,z)
   "  return OUT;                                           \n"
   "}                                                       \n"

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
   "                                                                   \n"
   "  float4 vec_sp=(float4)0.0f;vec_sp.a=1.0f;                        \n"//�X�y�L�������s�����̌��ʂ�����ϐ�
   "  float4 pos_sp=(float4)0.0f;pos_sp.a=1.0f;                        \n"//�X�y�L�����_�����̌��ʂ�����ϐ�
   "  if(any(sp)==true&&any(eye)==true)                                \n"//�X�y�L�����p���[�l�Ōv�Z�L�����s��
   "  {                                                                \n"
   "    float3 e=normalize(eye.rgb);                                   \n"//���������x�N�g���̐��K��
   "    float3 n=normalize(IN.nor.rgb);                                \n"//�@���̐��K��
   "    float3 l=-normalize(l_vec.rgb);                                \n"//���s���C�g�������t�ɂ��Ă̐��K��
   "    if(l_vec.w != 0.0f && any(IN.nor) == true)                     \n"//l_vec.w��0�ł���Ε��s���C�g�v�Z�͂��Ȃ�
   "    {                                                              \n"//�܂��A�@�����Ȃ��ꍇ���v�Z���Ȃ�
   "      if(dot(n,l)>=0.0f)                                           \n"//���̌����Ɩʂ̌��������΂��ǂ����𒲂ׂ�
   "      {                                                            \n"
   "        float3 rv=normalize(n*dot(n,l)*2.0-l);                     \n"//���˃x�N�g�������߂�
   "        vec_sp.rgb=pow(max(0,dot(rv,e)),sp_p.x);                   \n"//�X�y�L�����̋P�x���Z�o
   "        float3 no=(float3)((sp_p.x+2)/(2*3.14));                   \n"//�P�x�ɐ��K���W������Z
   "        vec_sp.rgb=sp.rgb*saturate(no*vec_sp.rgb);                 \n"//���K���W���E�X�y�L�����E�F����Z���ċ��߂�
   "      }                                                            \n"
   "    }                                                              \n"
   "                                                                   \n"
   "    if(l_pos.w>0.0f && any(IN.nor)==true)                          \n"//l_pos.w��0�ȉ��ł���Γ_���C�g�v�Z�͂��Ȃ�
   "    {                                                              \n"
   "      float3 lp_len=IN.pos_c-l_pos.xyz;                            \n"//�_�����ƒ��_�̊e�s�N�Z���̈ʒu����x�N�g�������߂�
   "      float  len=length(lp_len);                                   \n"//���߂��x�N�g���̒��������߂�
   "      float3 nor_lp=-normalize(lp_len);                            \n"//���߂��x�N�g���̐��K��
   "      float  w=saturate(len/l_pos.w);                              \n"//�x�N�g���̒����Əo�͕������ŏo��
   "      if(dot(n,nor_lp)>=0.0f)                                      \n"//�_�����̌����Ɩʂ̌��������΂��ǂ����𒲂ׂ�
   "      {                                                            \n"
   "        float3 rv=normalize(n*dot(n,nor_lp)*2.0-nor_lp);           \n"//�_�����ɑ΂��Ĕ��˃x�N�g�������߂�
   "        pos_sp.rgb=pow(max(0,dot(rv,e)),sp_p.x);                   \n"//�X�y�L�����̋P�x���Z�o
   "        float3 no=(float3)((sp_p.x+2) / (2*3.14));                 \n"//�P�x�ɐ��K���W������Z
   "        pos_sp.rgb=saturate(no*pos_sp.rgb)*((float3)1.0-w);        \n"//�X�y�L�����ƌ��̋�����_�Ώۂ̋����Ō���������
   "      }                                                            \n"
   "    }                                                              \n"
   "  }                                                                \n"
   "                                                                   \n"
   "  col=col*d+vec_sp+pos_sp+a;                                       \n"//�F=�|���S���F*�f�B�t�B�[�Y�F+�X�y�L����+�A���r�G���g�F
   "  col=saturate(col);                                               \n"//���߂��F��0.0�`1.0�𒴂��Ȃ��悤�ɂ���
   "                                                                   \n"
   "  if(any(emi)==true)                                               \n"//�G�~�b�V�u�̗L��
   "  {                                                                \n"
   "    col=max(col,emi);                                              \n"//���߂��F�ƃG�~�b�V�u�i���Ȕ����j�A���邢�F���ŏI�I�ȐF�Ƃ���
   "  }                                                                \n"
   "                                                                   \n"
   "  float4 tex=(float4)0.0f;                                         \n"//�e�N�X�`��
   "  tex=txDiffuse.Sample(samLinear,IN.uv);                            \n"//UV����e�N�X�`���̐F�̒l���擾
   "  float x,y;                                                       \n"//�e�N�X�`���̑傫��������ϐ�
   "  txDiffuse.GetDimensions(x,y);                                    \n"//�e�N�X�`���̑傫�����擾
   "  if(x != 0.0f && y != 0.0f)                                       \n"//�傫����0�̎��A�e�N�X�`���͖����Ɣ��f����
   "     col *= tex;                                                   \n"//�e�N�X�`���̐F����������
   "                                                                   \n"
   "  if(col.a<=0.0f)                                                  \n"//���S���߂ł���΁A
   "     discard;                                                      \n"//���̃s�N�Z����`�悵�Ȃ�
   "                                                                   \n"
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

	//�X�L�����f���p----------------
	//����������HLSL�� vs_skin�֐��������R���p�C��
	hr = D3DCompile(g_hlsl_sause_code, strlen(g_hlsl_sause_code), 0, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"vs_skin", "vs_4_0", 0, 0, &pCompiledShader, &pErrors);

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
		NULL, &m_pVertexShaderSkin);
	if (FAILED(hr))
	{
		SAFE_RELEASE(pCompiledShader);
		MessageBox(0, L"�o�[�e�b�N�X�V�F�[�_�[�X�L���p�쐬���s", NULL, MB_OK);
		return;
	}

	//���_�C���v�b�g���C�A�E�g���`
	D3D11_INPUT_ELEMENT_DESC layout_skin[] =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT   ,0,0, D3D11_INPUT_PER_VERTEX_DATA,0},
		{"NORMAL",  0,DXGI_FORMAT_R32G32B32_FLOAT,   0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"UV"   ,   0,DXGI_FORMAT_R32G32_FLOAT,      0,24,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"COLOR",   0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,32,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"BI"   ,   0,DXGI_FORMAT_R32G32B32A32_UINT ,0,48,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"WE"   ,   0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,64,D3D11_INPUT_PER_VERTEX_DATA,0},
	};
	UINT numElements_skin = sizeof(layout_skin) / sizeof(layout_skin[0]);

	//���_�C���v�b�g���C�A�E�g���쐬�E���C�A�E�g���Z�b�g
	hr = Dev::GetDevice()->CreateInputLayout(layout_skin, numElements_skin, pCompiledShader->GetBufferPointer(),
		pCompiledShader->GetBufferSize(), &m_pVertexLayout);
	if (FAILED(hr))
	{
		MessageBox(0, L"���C�A�E�g�쐬���s", NULL, MB_OK);
		return;
	}
	SAFE_RELEASE(pCompiledShader);

	//�X�L���p�̃R���X�^���g�o�b�t�@�쐬
	D3D11_BUFFER_DESC cb_skin;
	cb_skin.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
	cb_skin.ByteWidth           = sizeof(CMODEL_BONE_BUFFER);
	cb_skin.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
	cb_skin.MiscFlags           = 0;
	cb_skin.StructureByteStride = 0;
	cb_skin.Usage               = D3D11_USAGE_DYNAMIC;

	//�X�e�[�^�X�����ɃR���X�^���g�o�b�t�@
	hr = Dev::GetDevice()->CreateBuffer(&cb_skin, NULL, &m_pConstantBufferSkin);
	if (FAILED(hr))
	{
		MessageBox(0, L"�X�L���p�R���X�^���g�o�b�t�@�쐬���s", NULL, MB_OK);
		return;
	}

	//-------------------------------

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
		{"UV"   ,   0,DXGI_FORMAT_R32G32_FLOAT,      0,24,D3D11_INPUT_PER_VERTEX_DATA,0},
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
	//�ʏ탂�f���p�̔j��
	SAFE_RELEASE(m_pConstantBuffer);//�R���X�^���g�o�b�t�@�j��
	SAFE_RELEASE(m_pPixelShader);   //�s�N�Z���V�F�[�_�[�j��
	SAFE_RELEASE(m_pVertexLayout);  //���_���̓��C�A�E�g�j��
	SAFE_RELEASE(m_pVertexShader);  //�o�[�e�b�N�X�V�F�[�_�[
	//�X�L�����f���p�̔j��
	SAFE_RELEASE(m_pConstantBufferSkin);//�R���X�^���g�o�b�t�@�j��
	SAFE_RELEASE(m_pVertexLayoutSkin);  //���_���̓��C�A�E�g�j��
	SAFE_RELEASE(m_pVertexShaderSkin);  //�o�[�e�b�N�X�V�F�[�_�[�j��
}

//���f���������_�����O����
void CRender3D::Render(CMODEL* modle,float mat[16],float mat_w[16],float v_eye[3])
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

			//��������x�N�g����n��
			if (v_eye == nullptr)
			{
				memset(data.m_eye, 0x00, sizeof(data.m_eye));
			}
			else
			{
				data.m_eye[0] = v_eye[0];
				data.m_eye[1] = v_eye[1];
				data.m_eye[2] = v_eye[2];
				data.m_eye[3] = 1.0f;
			}

			//���s���C�g�̒l��n��
			memcpy(data.m_light_vec, m_light_vector, sizeof(m_light_vector));
			//�_���C�g�̒l��n��
			memcpy(data.m_light_pos, m_light_pos, sizeof(m_light_pos));

			//�ގ��@�A���r�G���g��n��
			memcpy(data.m_ambient, modle->m_Material[i].m_ambient, sizeof(data.m_ambient));
			//�ގ��@�f�t�B�[�Y��n��
			memcpy(data.m_diffuse, modle->m_Material[i].m_diffuse, sizeof(data.m_diffuse));
			//�ގ��@�G�~�b�V�u��n��
			memcpy(data.m_emissive, modle->m_Material[i].m_emissive, sizeof(data.m_emissive));
			//�ގ��@�X�y�L������n��
			memcpy(data.m_specular, modle->m_Material[i].m_specular, sizeof(data.m_specular));
			for (int j = 0; j < 4; j++)
			{
				data.m_specular_power[j] = modle->m_Material[i].m_specular_power;
			}

			memcpy_s(pData.pData, pData.RowPitch, (void*)&data, sizeof(CMODEL3D_BUFFER));
			//�R���X�^���g�o�b�t�@���V�F�[�_�ɗA��
			Dev::GetDeviceContext()->Unmap(m_pConstantBuffer, 0);
		}

		//�e�N�X�`���[�T���v����o�^
		Dev::GetDeviceContext()->PSSetSamplers(0, 1, Draw::GetSamplerState());
		//�e�N�X�`����o�^
		Dev::GetDeviceContext()->PSSetShaderResources(0, 1, &modle->m_Material[i].m_pTexture);

		//�o�^�����������Ƀ|���S���`��
		Dev::GetDeviceContext()->DrawIndexed(modle->m_pindex_size[i] * 3, 0, 0);

	}
	

	
}

//�X�L�����f���̃����_�����O
void CRender3D::Render(C_SKIN_MODEL* modle, float mat[16], float mat_w[16], float v_eye[3])
{
	//���_���C�A�E�g
	Dev::GetDeviceContext()->IASetInputLayout(m_pVertexLayoutSkin);

	//�g�p����V�F�[�_�[�̓o�^
	Dev::GetDeviceContext()->VSSetShader(m_pVertexShaderSkin, NULL, 0);
	Dev::GetDeviceContext()->PSSetShader(m_pPixelShader, NULL, 0);

	//�R���X�^���g�o�b�t�@���g�p����V�F�[�_�ɓo�^
	Dev::GetDeviceContext()->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	Dev::GetDeviceContext()->VSSetConstantBuffers(1, 1, &m_pConstantBufferSkin);
	Dev::GetDeviceContext()->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	Dev::GetDeviceContext()->PSSetConstantBuffers(1, 1, &m_pConstantBufferSkin);


	//�v���~�e�B�u�E�g�|���W�[���Z�b�g  D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
	Dev::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//�{�[���R���X�^���g�o�b�t�@�ɓ����֐�
	CMODEL_BONE_BUFFER bones;
	//bones�̏�����
	for (int i = 0; i < modle->m_bone_max; i++)
	{
		Math3D::IdentityMatrix(bones.m_mat[i]);
	}
	//bones�Ɋe�{�[���̎p���s��ƃA�j���[�V�����s�񂩂猻�݂̎p�������߂�

	//�{�[���ɐe�q�֌W�����߂�

	//�{�[���p�̃R���X�^���g�o�b�t�@��GPU�ɑ���
	D3D11_MAPPED_SUBRESOURCE pData;
	if (SUCCEEDED(Dev::GetDeviceContext()->Map(m_pConstantBufferSkin, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
	{
		memcpy_s(pData.pData, pData.RowPitch, (void*)&bones, sizeof(CMODEL_BONE_BUFFER));
		Dev::GetDeviceContext()->Unmap(m_pConstantBufferSkin, 0);
	}

	for (int i = 0; i < modle->m_material_max; i++)
	{
		//�o�[�e�b�N�X�o�b�t�@�o�^
		UINT stride = sizeof(CPOINT3D_SKIN_LAYOUT);
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

			//��������x�N�g����n��
			if (v_eye == nullptr)
			{
				memset(data.m_eye, 0x00, sizeof(data.m_eye));
			}
			else
			{
				data.m_eye[0] = v_eye[0];
				data.m_eye[1] = v_eye[1];
				data.m_eye[2] = v_eye[2];
				data.m_eye[3] = 1.0f;
			}

			//���s���C�g�̒l��n��
			memcpy(data.m_light_vec, m_light_vector, sizeof(m_light_vector));
			//�_���C�g�̒l��n��
			memcpy(data.m_light_pos, m_light_pos, sizeof(m_light_pos));

			//�ގ��@�A���r�G���g��n��
			memcpy(data.m_ambient, modle->m_Material[i].m_ambient, sizeof(data.m_ambient));
			//�ގ��@�f�t�B�[�Y��n��
			memcpy(data.m_diffuse, modle->m_Material[i].m_diffuse, sizeof(data.m_diffuse));
			//�ގ��@�G�~�b�V�u��n��
			memcpy(data.m_emissive, modle->m_Material[i].m_emissive, sizeof(data.m_emissive));
			//�ގ��@�X�y�L������n��
			memcpy(data.m_specular, modle->m_Material[i].m_specular, sizeof(data.m_specular));
			for (int j = 0; j < 4; j++)
			{
				data.m_specular_power[j] = modle->m_Material[i].m_specular_power;
			}

			memcpy_s(pData.pData, pData.RowPitch, (void*)&data, sizeof(CMODEL3D_BUFFER));
			//�R���X�^���g�o�b�t�@���V�F�[�_�ɗA��
			Dev::GetDeviceContext()->Unmap(m_pConstantBuffer, 0);
		}

		//�e�N�X�`���[�T���v����o�^
		Dev::GetDeviceContext()->PSSetSamplers(0, 1, Draw::GetSamplerState());
		//�e�N�X�`����o�^
		Dev::GetDeviceContext()->PSSetShaderResources(0, 1, &modle->m_Material[i].m_pTexture);

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