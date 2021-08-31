#include "DeviceCreate.h"
#include "Render3D.h"
#include "Draw2DPolygon.h"
#include<stdio.h>

//cmo�t�@�C����ǂݍ���
void C_SKIN_MODEL::LoadCmoModel(const wchar_t* name)
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
        m_ppVertexBuffer    = new ID3D11Buffer * [m_material_max];
        m_ppIndexBuffer     = new ID3D11Buffer * [m_material_max];
        m_Material          = new CMATERIAL[m_material_max];
        m_pvertex_size      = new int[m_material_max];
        m_pindex_size       = new int[m_material_max];
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
        //Texture�擾
        m_Material[i].m_pTexture = nullptr;//�e�N�X�`���̏�����

        for (int i = 0; i < 15; i++)
        {
            //�e�N�X�`�������邩�ǂ����𔻒f Texture:
            fscanf_s(fp, "%s", str, 256);
            if (strcmp(str, "}") == 0)//�������"{"�Ńe�N�X�`���L�����f
            {
                break;
            }
            //�t���p�X����t�@�C���������o��
            char* p = str;//�����񑀍�p
            char* p_str = str;
            if (strstr(p, "__") != nullptr)
            {
                MessageBox(0, L"�e�N�X�`�����ɃA���_�[�o�[���܂܂�Ă��邽�ߒ��~", NULL, MB_OK);
                return;
            }

            //�A���_�[�o�[�Ȃ��ꏊ�܂ňړ�
            do
            {
                p_str = strstr(p, "_");//������̒��ɂ���A���_�[�o�[�̂���A�h���X����擾
                if (p_str == nullptr)  //������ɃA���_�[�o�[���Ȃ���΃t�@�C�����̏ꏊ
                    break;
                p = p_str + 1;//�A���_�[�o�[�̈ʒu�Ɉړ�
            } while (1);

            int len;                       //���ʃR�[�h�܂ł̒���
            char file_name[128] = { "\0" };//�t�@�C����������z��
            len = strcspn(p, ".");         //�������.�̈ʒu���擾

            strncpy_s(file_name, p, len + 4);
            file_name[len + 5] = '\0';//�擾�����t�@�C�����̖��[��\0��t����

            //�����Ńe�N�X�`����ǂݍ���
            wchar_t file_name_w[128];//���j�R�[�h�p�����������z��
            size_t ret;
            mbstowcs_s(&ret, file_name_w, 128, file_name, strlen(file_name) + 1);//�}���`�o�C�g���������j�R�[�h�ɕϊ�
            m_Material[i].m_pTexture = Draw::LoadImage(file_name_w);//�e�N�X�`���쐬
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
                , &a, &b, &c, &d, &e, &m_pindex_size[i]);
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

    //�o�b�t�@�o�^�p�̃�������Ԃ��쐬
    CPOINT3D_SKIN_LAYOUT** ppvertex = new CPOINT3D_SKIN_LAYOUT * [sub_mesh_count];

    //���@�[�e�b�N�X�����擾
    for (int i = 0; i < sub_mesh_count; i++)
    {
        //���@�[�e�b�N�X�T�C�Y���擾 vertexbuffer_size
        fscanf_s(fp, "%s", str, 256);
        sscanf_s(str, "vertexbuffer_size:%d;", &m_pvertex_size[i]);

        //�o�b�t�@�o�^�p�̃�������Ԃ��쐬
        ppvertex[i] = new CPOINT3D_SKIN_LAYOUT[m_pvertex_size[i]];

        //���@�[�e�b�N�X�����擾
        for (int j = 0; j < m_pvertex_size[i]; j++)
        {
            fscanf_s(fp, "%s", str, 256);
            fscanf_s(fp, "%s", str, 256);
            sscanf_s(str, " P:%f,%f,%f,;",
                &ppvertex[i][j].m_pos[0], &ppvertex[i][j].m_pos[1], &ppvertex[i][j].m_pos[2]);
            fscanf_s(fp, "%s", str, 256);
            sscanf_s(str, " N:%f,%f,%f,;",
                &ppvertex[i][j].m_normal[0], &ppvertex[i][j].m_normal[1], &ppvertex[i][j].m_normal[2]);
            fscanf_s(fp, "%s", str, 256);
            sscanf_s(str, " UV:%f,%f,;",
                &ppvertex[i][j].m_uv[0], &ppvertex[i][j].m_uv[1]);
            fscanf_s(fp, "%s", str, 256);
            sscanf_s(str, " C:%f,%f,%f,%f,;",
                &ppvertex[i][j].m_color[0], &ppvertex[i][j].m_color[1], &ppvertex[i][j].m_color[2], &ppvertex[i][j].m_color[3]);
            fscanf_s(fp, "%s", str, 256);
        }

    }

    //���@�[�e�b�N�X�o�b�t�@�̗L���`�F�b�N
    fscanf_s(fp, "%s", str, 256);
    if (strcmp(str, "extents:") == 0)
    {
        //�s��ID��weight��񖳂�
        MessageBox(0, L"�X�L����񂪖������ߒ��~���܂�", NULL, MB_OK);
        return;
    }

    //���@�[�e�b�N�X�o�b�t�@�ɃX�L������o�^
    for (int i = 0; i < sub_mesh_count; i++)
    {
        //�X�L���̒��_���l�� skin_max;
        int skin_max = 0;
        sscanf_s(str, "skin_max:%d;", &skin_max);
        if (skin_max != m_pvertex_size[i])
        {
            MessageBox(0, L"���_���ƃX�L���ǉ����钸�_���ɈႢ������̂Œ��~���܂�", NULL, MB_OK);
            return;
        }
        else
        {
            //�X�̒ǉ����_���擾
            for (int j = 0; j < skin_max; j++)
            {
                fscanf_s(fp, "%s", str, 256);
                fscanf_s(fp, "%s", str, 256);
                sscanf_s(str, "BI:%d,%d,%d,%d,;",   //�{�[���s��ID�̎擾
                    &ppvertex[i][j].m_bone_index[0],
                    &ppvertex[i][j].m_bone_index[1],
                    &ppvertex[i][j].m_bone_index[2],
                    &ppvertex[i][j].m_bone_index[3]
                );
                fscanf_s(fp, "%s", str, 256);
                sscanf_s(str, "BW:%f,%f,%f,%f,;",   //�{�[���s��ɑ΂��Ă̏d�݂̎擾
                    &ppvertex[i][j].m_bone_weight[0],
                    &ppvertex[i][j].m_bone_weight[1],
                    &ppvertex[i][j].m_bone_weight[2],
                    &ppvertex[i][j].m_bone_weight[3]
                );

                fscanf_s(fp, "%s", str, 256);
            }
            fscanf_s(fp, "%s", str, 256);
        }
    }


    //���@�[�e�b�N�X�o�b�t�@�o�^
    for (int i = 0; i < sub_mesh_count; i++)
    {
        //�o�b�t�@�Ƀo�[�e�b�N�X�X�e�[�^�X�ݒ�
        D3D11_BUFFER_DESC bd;
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(CPOINT3D_SKIN_LAYOUT) * m_pvertex_size[i];
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.CPUAccessFlags = 0;
        bd.MiscFlags = 0;
       

        //�o�b�t�@�ɓ����f�[�^��ݒ�
        D3D11_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = ppvertex[i];
       

        //�X�e�[�^�X�ƃo�b�t�@�ɓ����f�[�^�����Ƀo�[�e�b�N�X�o�b�t�@�쐬
        HRESULT hr = Dev::GetDevice()->CreateBuffer(&bd, &InitData, &m_ppVertexBuffer[i]);
        if (FAILED(hr))
        {
            MessageBox(0, L"�o�[�e�b�N�X�o�b�t�@�쐬���s", NULL, MB_OK);
            return;
        }
        //�o�b�t�@�ɓo�^����ppvertex�͔p��
        delete[] ppvertex[i];
    }
    delete[] ppvertex;

    //extents:���΂�
    fscanf_s(fp, "%s", str, 256);
    fscanf_s(fp, "%s", str, 256);
    fscanf_s(fp, "%s", str, 256);
    //�{�[�����̎擾
    fscanf_s(fp, "%s", str, 256);
    sscanf_s(str, "bone_count:%d;", &m_bone_max);//�{�[���̍ő�擾��

    for (int i = 0; i < m_bone_max; i++)
    {
        fscanf_s(fp, "%s", str, 256);
        sscanf_s(str, "bone_name:%s;", m_bone[i].m_name, 128);//�{�[�����擾
        m_bone[i].m_name[strlen(m_bone[i].m_name) - 1] = '\0';//���[��';'���擾
        fscanf_s(fp, "%s", str, 256);
        fscanf_s(fp, "%s", str, 256);
        int index = -1;
        sscanf_s(str, "ParentIndex:%d;", &index);//�{�[���̐ڑ������擾
        if (index == -1)
        {
            ;//���̃{�[���͎n�c�̂��߉������Ȃ�
        }
        else
        {
            //�ڑ����̃{�[���A�N�Z�X�p�|�C���^����󂢂Ă�|�C���^��T��
            for (int j = 0; j < 16; j++)
            {
                //�󂢂Ă�|�C���^���炱�̃{�[���ɃA�N�Z�X
                if (m_bone[index].m_p_bone_next[j] == nullptr)
                {
                    m_bone[index].m_p_bone_next[j] = &m_bone[i];
                    break;
                }
            }
        }
        fscanf_s(fp, "%s", str, 256);

        //�e�s����擾
        fscanf_s(fp, "%s", str, 256);
        //BindPos���擾
        float* p = m_bone[i].m_bind_pos;//�󂯂Ƃ薼�������̂ő���ɂ�����󂯂Ƃ�
        sscanf_s(str, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,",
            &p[ 0], &p[ 1], &p[ 2], &p[ 3],
            &p[ 4], &p[ 5], &p[ 6], &p[ 7],
            &p[ 8], &p[ 9], &p[10], &p[11],
            &p[12], &p[13], &p[14], &p[15]);
        fscanf_s(fp, "%s", str, 256);

        fscanf_s(fp, "%s", str, 256);
        fscanf_s(fp, "%s", str, 256);
        //InvBindPos���擾
        p = m_bone[i].m_inv_bind_pos;
        sscanf_s(str, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,",
            &p[ 0], &p[ 1], &p[ 2], &p[ 3],
            &p[ 4], &p[ 5], &p[ 6], &p[ 7],
            &p[ 8], &p[ 9], &p[10], &p[11],
            &p[12], &p[13], &p[14], &p[15]);
        fscanf_s(fp, "%s", str, 256);

        fscanf_s(fp, "%s", str, 256);
        fscanf_s(fp, "%s", str, 256);
        //LocalTransform���擾
        p = m_bone[i].m_local_transform;
        sscanf_s(str, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,",
            &p[ 0], &p[ 1], &p[ 2], &p[ 3],
            &p[ 4], &p[ 5], &p[ 6], &p[ 7],
            &p[ 8], &p[ 9], &p[10], &p[11],
            &p[12], &p[13], &p[14], &p[15]);
        fscanf_s(fp, "%s", str, 256);
        fscanf_s(fp, "%s", str, 256);
    }
}