#pragma once
//3D���f���`��----
#include <Windows.h>
#include <D3D11.h>

#include <d3dCompiler.h>

//����������}�N��-------------
#define SAFE_DELETE(p)         {if(p){delete (p);     (p)=nullptr;}}
#define SAFE_DELETE_ARRAY(p)   {if(p){delete[](p);    (p)=nullptr;}}
#define SAFE_RELEASE(p)        {if(p){(p)->Release(); (p)=nullptr;}}

//�R���X�^���g�o�b�t�@�N���X
class CMODEL3D_BUFFER
{
public:
	float m_mat[16];//�g�����X�t�H�[���p�s��
	float m_mat_w[16];//�@���p�̃��[���h�s��
	float m_light_vec[4];//���s���C�g�̕���
	float m_light_pos[4];//�_���C�g�̈ʒu�Əo�͕�
	float m_ambient[4];//�A���r�G���g
	float m_diffuse[4];//�f�t�B�[�Y
	float m_emissive[4];//�G�~�b�V�u
	float m_specular[4];//�X�y�L����
	float m_specular_power[4];//�X�y�L�����p���[
	float m_eye[4];//�������
};

//�R���X�^���g�o�b�t�@�@�{�[���p
class CMODEL_BONE_BUFFER
{
public:
	float m_mat[128][16];//4�~4�̍s������f��������
};                       //�ő吔��128��A������

//���_���C�A�E�g�N���X�i���_�������j
class CPOINT3D_LAYOUT
{
public:
	float m_pos[3];     //X-Y-Z:���_
	float m_normal[3];  //X-Y-Z:�@��
	float m_uv[2];      //U-V�@:�e�N�X�`���ʒu
	float m_color[4];   //R-G-B-A:�F
};

//�X�L�����f���p�̒��_���C�A�E�g�N���X
class CPOINT3D_SKIN_LAYOUT :public CPOINT3D_LAYOUT
{
public:
	int   m_bone_index[4];    //���̒��_�ɉe����^����s��ID
	float m_bone_weight[4];   //�e����^����s��̏d�݁i�e���x�j
};

//�{�[���N���X
class BONE
{
public:
	char  m_name[128];          //�{�[����
	float m_bind_pos[16];       //�{�[�����W�ϊ��s��
	float m_inv_bind_pos[16];   //�t�{�[�����W�ϊ��i���_�Ɉړ�����s��j
	float m_local_transform[16];//���[�J���ϊ��s��
	BONE* m_p_bone_next[16];    //�ڑ��p�|�C���^�i�ő�16�{�[���ɐڑ��\�j
	//�R���X�g���N�^
	BONE()
	{
		memset(this, 0x00, sizeof(BONE));
	}
};

//�}�e���A���N���X(�ގ����)
class CMATERIAL
{
public:
	float m_diffuse[4];                  //�f�B���[�Y�i�g�U���ˁj
	float m_specular[4];                 //�X�y�L�����i���ʔ��ˁj
	float m_specular_power;              //�X�y�L�����p���[
	float m_ambient[4];                  //�A���r�G���g�i�����j
	float m_emissive[4];                 //�G�~�b�V�u�i���ː��i���Ȕ����j�j
	ID3D11ShaderResourceView* m_pTexture;//�e�N�X�`��

	//�R���X�g���N�^
	CMATERIAL()
	{
		memset(this, 0x00, sizeof(CMATERIAL));
		//m_pTexture->Release();
		m_pTexture = nullptr;
	}
	//�f�X�g���N�^
	~CMATERIAL()
	{
		if (m_pTexture != nullptr)
		{
			m_pTexture->Release();
		}
	}
};

//���f���N���X
class CMODEL
{
public:
	~CMODEL();
	int* m_pvertex_size;                 //�ő咸�_��
	int* m_pindex_size;                  //�ő�ʐ�
	CMATERIAL*         m_Material;      //���f���̍ގ����
	ID3D11Buffer**     m_ppVertexBuffer;//�o�[�e�b�N�X�o�b�t�@�i���f���̒��_���j
	ID3D11Buffer**     m_ppIndexBuffer; //�C���f�b�N�X�o�b�t�@�i���f���̖ʏ��j
	int m_material_max;                 //�}�e���A���̐�

	//�O�p��Model�쐬
	void CreateSampleTriangularpyramid();//�T���v���p�̎O�p�����쐬
	//CMO�t�@�C���ǂݍ���
	void LoadCmoModel(const wchar_t* name);
};

//�X�L�����f���N���X
class C_SKIN_MODEL :public CMODEL
{
public:
	BONE m_bone[128]; //�{�[�����i�ő�{�[����128�j
	int  m_bone_max;  //�g�p����{�[���̐�
	void LoadCmoModel(const wchar_t* name);//�X�L�����f����cmo�t�@�C���ǂݍ���
	void CreateSampleTriangularpyramid() {};//�p����̊֐������s�����Ȃ��_�~�[�֐�
};

//3D�����_�����O�N���X
typedef class CRender3D
{
public:
	CRender3D(){}
	~CRender3D(){}
	static void Init();
	static void Delete();

	static void Render(CMODEL* modle,float mat[16],float mat_w[16],float v_eye[3]); //���f���������_�����O����
	static void Render(C_SKIN_MODEL* modle, float mat[16], float mat_w[16], float v_eye[3]);//�X�L�����f���̃����_�����O
	static void SetLightVec(float x, float y, float z, bool light_on);//���s�����̌���������
	static void SetLightPos(float x, float y, float z, float max);//�_�����̈ʒu�Əo�͕�������

private:
	static void SkinAnimation(C_SKIN_MODEL* modle, CMODEL_BONE_BUFFER* bones, int bone_id, float mat_ani[16]);//�{�[���ɃA�j���[�V������������F

	static float m_light_vector[4];//���s�����i�����j
	static float m_light_pos[4];//�_�����i�ʒu�j
	static ID3D11VertexShader* m_pVertexShader;   //�o�[�e�b�N�X�V�F�[�_�[
	static ID3D11PixelShader*  m_pPixelShader;    //�s�N�Z���V�F�[�_�[
	static ID3D11InputLayout*  m_pVertexLayout;   //���_���̓��C�A�E�g
	static ID3D11Buffer*       m_pConstantBuffer; //�R���X�^���g�o�b�t�@

	static ID3D11VertexShader* m_pVertexShaderSkin;   //�X�L���p�o�[�e�b�N�X�V�F�[�_�[
	static ID3D11InputLayout*  m_pVertexLayoutSkin;   //�X�L���p���_���̓��C�A�E�g
	static ID3D11Buffer*       m_pConstantBufferSkin; //�X�L���p�R���X�^���g�o�b�t�@

}Render;