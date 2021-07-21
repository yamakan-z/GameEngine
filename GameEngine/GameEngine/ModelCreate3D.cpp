#include "DeviceCreate.h"
#include "Render3D.h"

#include<stdio.h>

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
        {3,1,0,},//1��
        {0,2,3,},//2��
        {3,2,1,},//3��
        {1,2,0,},//4��
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

//cmo�t�@�C���̓ǂݍ���
void CMODEL::LoadCmoModel(const wchar_t* name)
{
    //cmo�t�@�C�����J��
    FILE* fp;
    _wfopen_s(&fp, name, L"r");
    if (fp == nullptr)
    {
        MessageBox(0, L"cmo�t�@�C����������Ȃ�", NULL, MB_OK);
        return;
    }

    //�t�@�C���̈�s�ڂ��擾
    char str[256];
    fscanf_s(fp, "%s", str, 128);

    //�t�@�C�����̃��f�������擾
    int mesh_count = 0;
    fscanf_s(fp, "%s", str, 256);
    sscanf_s(str, "mesh_count:%d;", &mesh_count);

    if (mesh_count != 1)
    {
        MessageBox(0, L"File����Model��0�܂��́A2�ȏ�̂��ߒ��~���܂�", NULL, MB_OK);
        return;
    }

    //���f���Ŏg�p����}�e���A�����擾"material_count"
    fscanf_s(fp, "%s", str, 256);//mesh_name;�̍s�͔�΂�
    fscanf_s(fp, "%s", str, 256);
    sscanf_s(str, "material_count:%d;", &m_material_max);

    if (m_material_max == 0)
    {
        MessageBox(0, L"�ގ���񂪂Ȃ����ߒ��~���܂�", NULL, MB_OK);
        return;
    }
    else
    {
        //�}�e���A���̐������v�f�����
        //�}�e���A���E�C���f�b�N�X�E���@�[�e�b�N�X�������郁�����쐬
        m_ppVertexBuffer = new ID3D11Buffer * [m_material_max];
        m_ppIndexBuffer  = new ID3D11Buffer * [m_material_max];
        m_Material       = new CMATERIAL[m_material_max];
        m_pvertex_size   = new int[m_material_max];
        m_pindex_size    = new int[m_material_max];
    }

    //������Material���̎擾material A�ED�ES�ESP�EE
    for (int i = 0; i < m_material_max; i++)
    {
        fscanf_s(fp, "%s", str, 256);//Material���͔�΂�
        fscanf_s(fp, "%s", str, 256);//"{"�͔�΂�
        //A�擾
        fscanf_s(fp, "%s", str, 256);
        sscanf_s(str, " A:%f,%f,%f,%f",
            &m_Material[i].m_ambient[0], &m_Material[i].m_ambient[1], &m_Material[i].m_ambient[2], &m_Material[i].m_ambient[3]);
        //D�擾
        fscanf_s(fp, "%s", str, 256);
        sscanf_s(str, " D:%f,%f,%f,%f",
            &m_Material[i].m_diffuse[0], &m_Material[i].m_diffuse[1], &m_Material[i].m_diffuse[2], &m_Material[i].m_diffuse[3]);
        //S�擾
        fscanf_s(fp, "%s", str, 256);
        sscanf_s(str, " S:%f,%f,%f,%f",
            &m_Material[i].m_specular[0], &m_Material[i].m_specular[1], &m_Material[i].m_specular[2], &m_Material[i].m_specular[3]);
        //SP�擾
        fscanf_s(fp, "%s", str, 256);
        sscanf_s(str, " SP:%f", &m_Material[i].m_specular_power);
        //E�擾
        fscanf_s(fp, "%s", str, 256);
        sscanf_s(str, " E:%f,%f,%f,%f",
            &m_Material[i].m_emissive[0], &m_Material[i].m_emissive[1], &m_Material[i].m_emissive[2], &m_Material[i].m_emissive[3]);
        //Texture�擾�\��
        for (int i = 0; i < 15; i++)
        {
            //�e�N�X�`�������邩�ǂ����𔻒f Texture:
            fscanf_s(fp, "%s", str, 256);
            if (strcmp(str, "}") == 0)//�������"{"�Ńe�N�X�`���L�����f
            {
                m_Material[i].m_pTexture = nullptr;
                break;
            }
            //�����Ńe�N�X�`����ǂݍ��ޗ\��
            m_Material[i].m_pTexture = nullptr;
        }
    }

    //���f���̃��b�V�������l���@"sub_mesh_count:"
    int sub_mesh_count = 0;
    fscanf_s(fp, "%s", str, 256);
    sscanf_s(str, "sub_mesh_count:%d;", &sub_mesh_count);
    if (sub_mesh_count != m_material_max)
    {
        MessageBox(0, L"Material���Ɗe�o�b�t�@�̗v�f�����Ⴄ���ߒ��~���܂�", NULL, MB_OK);
        return;
    }
    else
    {
        //�e�o�b�t�@�̃T�C�Y���擾
        for (int i = 0; i < sub_mesh_count; i++)
        {
            //�T�u���b�V���̊e����Ȃ��������ϐ��i���Ȃ��ƕK�v�ȏ�񂪎��Ȃ��̂Łj
            int a, b, c, d, e;
            fscanf_s(fp, "%s", str, 256);
            sscanf_s(str, "{sub_mesh_id:%d,material:%d,IB:%d,VB:%d,start:%d,count:%d};"
            ,&a,&b,&c,&d,&e,&m_pindex_size[i]);
            m_pvertex_size[i] = m_pindex_size[i];
        }
    }

    //�C���f�b�N�X���擾
    fscanf_s(fp, "%s", str, 256);//ibs_count���΂�
    for (int i = 0; i < sub_mesh_count; i++)
    {
        fscanf_s(fp, "%s", str, 256);//indexbuffer_size���΂�
        //�o�b�t�@�o�^�p�̃�������Ԃ��쐬
        unsigned short* pindex = new unsigned short[m_pindex_size[i] * 3];

        //�C���f�b�N�X���擾
        for (int j = 0; j < m_pindex_size[i]; j++)
        {
            int w;
            fscanf_s(fp, "%s", str, 256);
            sscanf_s(str, "%d:{%hu,%hu,%hu};", &w, &pindex[j * 3 + 0], &pindex[j * 3 + 1], &pindex[j * 3 + 2]);
        }

        //�C���f�b�N�X�����o�b�t�@�[��
        //�o�b�t�@�ɃC���f�b�N�X�X�e�[�^�X�ݒ�
        D3D11_BUFFER_DESC hBufferDesc;
        hBufferDesc.ByteWidth = sizeof(unsigned short) * m_pindex_size[i] * 3;
        hBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        hBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        hBufferDesc.CPUAccessFlags = 0;
        hBufferDesc.MiscFlags = 0;
        hBufferDesc.StructureByteStride = sizeof(unsigned short);

        //�o�b�t�@�ɓ����f�[�^��ݒ�
        D3D11_SUBRESOURCE_DATA hSubResourceData;
        hSubResourceData.pSysMem = pindex;
        hSubResourceData.SysMemPitch = 0;
        hSubResourceData.SysMemSlicePitch = 0;

        //�X�e�[�^�X�ƃo�b�t�@�ɓ����f�[�^�����ɃC���f�b�N�X�o�b�t�@�쐬
        HRESULT hr = Dev::GetDevice()->CreateBuffer(&hBufferDesc, &hSubResourceData, &m_ppIndexBuffer[i]);
        if (FAILED(hr))
        {
            MessageBox(0, L"�C���f�b�N�X�o�b�t�@�쐬���s", NULL, MB_OK);
            return;
        }
        delete[] pindex;//�o�b�t�@�ɓo�^�����̂ŁA�ꎞ�I�ɓǂݍ��񂾏���j��
    }

    //���@�[�e�b�N�X�����擾
    for (int i = 0; i < sub_mesh_count; i++)
    {
        //���@�[�e�b�N�X�T�C�Y���擾 vertexbuffer_size
        fscanf_s(fp, "%s", str, 256);
        sscanf_s(str, "vertexbuffer_size:%d;", &m_pvertex_size[i]);
        //�o�b�t�@�o�^�p�̃�������Ԃ��쐬
        CPOINT3D_LAYOUT* pvertex = new CPOINT3D_LAYOUT[m_pvertex_size[i]];

        //���@�[�e�b�N�X�����擾
        for (int j = 0; j < m_pvertex_size[i]; j++)
        {
            fscanf_s(fp, "%s", str, 256);
            fscanf_s(fp, "%s", str, 256);
            sscanf_s(str, " P:%f,%f,%f,;",
                &pvertex[j].m_pos[0], &pvertex[j].m_pos[1], &pvertex[j].m_pos[2]);
            fscanf_s(fp, "%s", str, 256);
            sscanf_s(str, " N:%f,%f,%f,;",
                &pvertex[j].m_normal[0], &pvertex[j].m_normal[1], &pvertex[j].m_normal[2]);
            fscanf_s(fp, "%s", str, 256);
            sscanf_s(str, " UV:%f,%f,;",
                &pvertex[j].m_uv[0], &pvertex[j].m_uv[1]);
            fscanf_s(fp, "%s", str, 256);
            sscanf_s(str, " C:%f,%f,%f,%f,;",
                &pvertex[j].m_color[0], &pvertex[j].m_color[1], &pvertex[j].m_color[2], &pvertex[j].m_color[3]);
            fscanf_s(fp, "%s", str, 256);
        }

        //�o�b�t�@�Ƀo�[�e�b�N�X�X�e�[�^�X�ݒ�
        D3D11_BUFFER_DESC bd;
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(CPOINT3D_LAYOUT) * m_pvertex_size[i];
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = 0;
        bd.MiscFlags = 0;

        //�o�b�t�@�ɓ����f�[�^��ݒ�
        D3D11_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = pvertex;

        //�X�e�[�^�X�ƃo�b�t�@�ɓ����f�[�^�����ƂɃo�[�e�b�N�X�o�b�t�@�쐬
        HRESULT hr = Dev::GetDevice()->CreateBuffer(&bd, &InitData, &m_ppVertexBuffer[i]);
        if (FAILED(hr))
        {
            MessageBox(0, L"�o�[�e�b�N�X�o�b�t�@�쐬���s", NULL, MB_OK);
            return;
        }

        delete pvertex;//�o�b�t�@�������̂Ń��@�[�e�b�N�X���j��
    }


    fclose(fp);
}