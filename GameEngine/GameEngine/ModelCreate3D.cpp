#include "DeviceCreate.h"
#include "Render3D.h"

CMODEL::~CMODEL()
{
    delete[] m_pvertex_size;
    delete[] m_pindex_size;
    delete[] m_Material;

    for (int i = 0; i < m_material_max; i++)
    {
        m_ppVertexBuffer[i]->Release();
        m_ppIndexBuffer[i]->Release();
    }

    delete[] m_ppVertexBuffer;
    delete[] m_ppIndexBuffer;
}


//�O�p��Model�쐬
void CMODEL::CreateSampleTriangularpyramid()
{
	HRESULT hr = S_OK;

	//�O�p�����`�����邽�߂̒��_�̏��
	CPOINT3D_LAYOUT vertices[] =
	{
		//    px    py   pz     nx    ny   nz      u    v       r    g    b    a
		   {{ 0.5f,0.0f,0.0f },{0.0f,0.0f,0.0f} ,{0.0f,0.0f} ,{1.0f,0.0f,0.0f,1.0f},},//���_1
		   {{-0.5f,0.0f,0.0f },{0.0f,0.0f,0.0f} ,{0.0f,0.0f} ,{0.0f,1.0f,0.0f,1.0f},},//���_2
		   {{ 0.0f,0.0f,0.5f },{0.0f,0.0f,0.0f} ,{0.0f,0.0f} ,{0.0f,0.0f,1.0f,1.0f},},//���_3
		   {{ 0.0f,0.5f,0.25f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f} ,{1.0f,1.0f,1.0f,1.0f},},//���_4

	};

    //���@�[�e�b�N�X�E�C���f�b�N�X�o�b�t�@�E�ގ��ʂ����߂�i��̂݁j
    m_ppVertexBuffer = new ID3D11Buffer * [1];
    m_ppIndexBuffer  = new ID3D11Buffer * [1];
    m_Material       = new CMATERIAL[1];

    //�e�o�b�t�@�ő吔������z��̍쐬�ƃ}�e���A���J�E���g�̏�����
    this->m_material_max = 1; //�O�p���͍ގ����g���Ă��Ȃ���0����0�Ԗڂ̃o�b�t�@�`�悳��Ȃ��̂�1������
    m_pvertex_size = new int[1];
    m_pindex_size = new int[1];

    //�o�b�t�@�Ƀo�[�e�b�N�X�X�e�[�^�X�ݒ�
    D3D11_BUFFER_DESC bd;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;

    //�o�b�t�@�ɓ����f�[�^��ݒ�
    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = vertices;

    //�X�e�[�^�X�ƃo�b�t�@�ɓ����f�[�^�����Ƀo�[�e�b�N�X�o�b�t�@�쐬
    hr = Dev::GetDevice()->CreateBuffer(&bd, &InitData, &m_ppVertexBuffer[0]);
    if (FAILED(hr))
    {
        MessageBox(0, L"�o�[�e�b�N�X�o�b�t�@�쐬���s", NULL, MB_OK);
        return;
    }


    //�O�p���̖ʂ��`�����邽�߂̒��_�C���f�b�N�X���
    unsigned short hIndexData[4][3] =
    {
        {0,3,1,},//1��
        {0,2,3,},//2��
        {2,3,1,},//3��
        {0,2,1,},//4��
    };

    //�o�b�t�@�ɃC���f�b�N�X�X�e�[�^�X�ݒ�
    D3D11_BUFFER_DESC hBufferDesc;
    hBufferDesc.ByteWidth = sizeof(hIndexData);
    hBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    hBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    hBufferDesc.CPUAccessFlags = 0;
    hBufferDesc.MiscFlags = 0;
    hBufferDesc.StructureByteStride = sizeof(unsigned short);

    //�o�b�t�@�ɓ����f�[�^��ݒ�
    D3D11_SUBRESOURCE_DATA hSubResourceData;
    hSubResourceData.pSysMem = hIndexData;
    hSubResourceData.SysMemPitch = 0;
    hSubResourceData.SysMemSlicePitch = 0;

    //�X�e�[�^�X�ƃo�b�t�@�ɓ����f�[�^�����ɃC���f�b�N�X�o�b�t�@�쐬
    hr = Dev::GetDevice()->CreateBuffer(&hBufferDesc, &hSubResourceData, &m_ppIndexBuffer[0]);
    if (FAILED(hr))
    {
        MessageBox(0, L"�C���f�b�N�X�o�b�t�@�쐬���s", NULL, MB_OK);
        return;
    }

    //�ގ����
    memset(m_Material->m_ambient,  0x00,  sizeof(m_Material->m_ambient ));
    memset(m_Material->m_diffuse,  0x00,  sizeof(m_Material->m_diffuse ));
    memset(m_Material->m_specular, 0x00,  sizeof(m_Material->m_specular));
    m_Material->m_pTexture = nullptr;
    m_Material->m_pTexture = nullptr;

    //���_�E�C���f�b�N�X�̑傫��
    m_pindex_size[0]  = 4;
    m_pvertex_size[0] = 4;
}