#include "DeviceCreate.h"
#include "Render3D.h"
#include "Draw2DPolygon.h"
#include<stdio.h>

//cmoファイルを読み込み
void C_SKIN_MODEL::LoadCmoModel(const wchar_t* name)
{
    //cmoファイルを開く
    FILE* fp;
    _wfopen_s(&fp, name, L"r");
    if (fp == nullptr)
    {
        MessageBox(0, L"cmoファイルが見つからない", NULL, MB_OK);
        return;
    }

    //ファイルの一行目を取得
    char str[256];
    fscanf_s(fp, "%s", str, 128);

    //ファイル内のモデル数を取得
    int mesh_count = 0;
    fscanf_s(fp, "%s", str, 256);
    sscanf_s(str, "mesh_count:%d;", &mesh_count);

    if (mesh_count != 1)
    {
        MessageBox(0, L"File内のModelが0または、2以上のため中止します", NULL, MB_OK);
        return;
    }

    //モデルで使用するマテリアル数取得"material_count"
    fscanf_s(fp, "%s", str, 256);//mesh_name;の行は飛ばす
    fscanf_s(fp, "%s", str, 256);
    sscanf_s(str, "material_count:%d;", &m_material_max);

    if (m_material_max == 0)
    {
        MessageBox(0, L"材質情報がないため中止します", NULL, MB_OK);
        return;
    }
    else
    {
        //マテリアルの数だけ要素を作る
        //マテリアル・インデックス・ヴァーテックス情報を入れるメモリ作成
        m_ppVertexBuffer    = new ID3D11Buffer * [m_material_max];
        m_ppIndexBuffer     = new ID3D11Buffer * [m_material_max];
        m_Material          = new CMATERIAL[m_material_max];
        m_pvertex_size      = new int[m_material_max];
        m_pindex_size       = new int[m_material_max];
    }

    //複数のMaterial情報の取得material A・D・S・SP・E
    for (int i = 0; i < m_material_max; i++)
    {
        fscanf_s(fp, "%s", str, 256);//Material名は飛ばす
        fscanf_s(fp, "%s", str, 256);//"{"は飛ばす
        //A取得
        fscanf_s(fp, "%s", str, 256);
        sscanf_s(str, " A:%f,%f,%f,%f",
            &m_Material[i].m_ambient[0], &m_Material[i].m_ambient[1], &m_Material[i].m_ambient[2], &m_Material[i].m_ambient[3]);
        //D取得
        fscanf_s(fp, "%s", str, 256);
        sscanf_s(str, " D:%f,%f,%f,%f",
            &m_Material[i].m_diffuse[0], &m_Material[i].m_diffuse[1], &m_Material[i].m_diffuse[2], &m_Material[i].m_diffuse[3]);
        //S取得
        fscanf_s(fp, "%s", str, 256);
        sscanf_s(str, " S:%f,%f,%f,%f",
            &m_Material[i].m_specular[0], &m_Material[i].m_specular[1], &m_Material[i].m_specular[2], &m_Material[i].m_specular[3]);
        //SP取得
        fscanf_s(fp, "%s", str, 256);
        sscanf_s(str, " SP:%f", &m_Material[i].m_specular_power);
        //E取得
        fscanf_s(fp, "%s", str, 256);
        sscanf_s(str, " E:%f,%f,%f,%f",
            &m_Material[i].m_emissive[0], &m_Material[i].m_emissive[1], &m_Material[i].m_emissive[2], &m_Material[i].m_emissive[3]);
        //Texture取得
        m_Material[i].m_pTexture = nullptr;//テクスチャの初期化

        for (int i = 0; i < 15; i++)
        {
            //テクスチャがあるかどうかを判断 Texture:
            fscanf_s(fp, "%s", str, 256);
            if (strcmp(str, "}") == 0)//文字列に"{"でテクスチャ有無判断
            {
                break;
            }
            //フルパスからファイル名を取り出す
            char* p = str;//文字列操作用
            char* p_str = str;
            if (strstr(p, "__") != nullptr)
            {
                MessageBox(0, L"テクスチャ名にアンダーバーが含まれているため中止", NULL, MB_OK);
                return;
            }

            //アンダーバーない場所まで移動
            do
            {
                p_str = strstr(p, "_");//文字列の中にあるアンダーバーのあるアドレス先を取得
                if (p_str == nullptr)  //文字列にアンダーバーがなければファイル名の場所
                    break;
                p = p_str + 1;//アンダーバーの位置に移動
            } while (1);

            int len;                       //識別コードまでの長さ
            char file_name[128] = { "\0" };//ファイル名を入れる配列
            len = strcspn(p, ".");         //文字列の.の位置を取得

            strncpy_s(file_name, p, len + 4);
            file_name[len + 5] = '\0';//取得したファイル名の末端に\0を付ける

            //ここでテクスチャを読み込む
            wchar_t file_name_w[128];//ユニコード用文字列を入れる配列
            size_t ret;
            mbstowcs_s(&ret, file_name_w, 128, file_name, strlen(file_name) + 1);//マルチバイト文字をユニコードに変換
            m_Material[i].m_pTexture = Draw::LoadImage(file_name_w);//テクスチャ作成
        }
       
    }

    //モデルのメッシュ情報を獲得　"sub_mesh_count:"
    int sub_mesh_count = 0;
    fscanf_s(fp, "%s", str, 256);
    sscanf_s(str, "sub_mesh_count:%d;", &sub_mesh_count);
    if (sub_mesh_count != m_material_max)
    {
        MessageBox(0, L"Material数と各バッファの要素数が違うため中止します", NULL, MB_OK);
        return;
    }
    else
    {
        //各バッファのサイズを取得
        for (int i = 0; i < sub_mesh_count; i++)
        {
            //サブメッシュの各いらない情報入れる変数（取らないと必要な情報が取れないので）
            int a, b, c, d, e;
            fscanf_s(fp, "%s", str, 256);
            sscanf_s(str, "{sub_mesh_id:%d,material:%d,IB:%d,VB:%d,start:%d,count:%d};"
                , &a, &b, &c, &d, &e, &m_pindex_size[i]);
            m_pvertex_size[i] = m_pindex_size[i];
        }
    }

    //インデックス情報取得
    fscanf_s(fp, "%s", str, 256);//ibs_countを飛ばす
    for (int i = 0; i < sub_mesh_count; i++)
    {
        fscanf_s(fp, "%s", str, 256);//indexbuffer_sizeを飛ばす
        //バッファ登録用のメモリ空間を作成
        unsigned short* pindex = new unsigned short[m_pindex_size[i] * 3];

        //インデックス情報取得
        for (int j = 0; j < m_pindex_size[i]; j++)
        {
            int w;
            fscanf_s(fp, "%s", str, 256);
            sscanf_s(str, "%d:{%hu,%hu,%hu};", &w, &pindex[j * 3 + 0], &pindex[j * 3 + 1], &pindex[j * 3 + 2]);
        }

        //インデックス情報をバッファー化
        //バッファにインデックスステータス設定
        D3D11_BUFFER_DESC hBufferDesc;
        hBufferDesc.ByteWidth = sizeof(unsigned short) * m_pindex_size[i] * 3;
        hBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        hBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        hBufferDesc.CPUAccessFlags = 0;
        hBufferDesc.MiscFlags = 0;
        hBufferDesc.StructureByteStride = sizeof(unsigned short);

        //バッファに入れるデータを設定
        D3D11_SUBRESOURCE_DATA hSubResourceData;
        hSubResourceData.pSysMem = pindex;
        hSubResourceData.SysMemPitch = 0;
        hSubResourceData.SysMemSlicePitch = 0;

        //ステータスとバッファに入れるデータを元にインデックスバッファ作成
        HRESULT hr = Dev::GetDevice()->CreateBuffer(&hBufferDesc, &hSubResourceData, &m_ppIndexBuffer[i]);
        if (FAILED(hr))
        {
            MessageBox(0, L"インデックスバッファ作成失敗", NULL, MB_OK);
            return;
        }
        delete[] pindex;//バッファに登録したので、一時的に読み込んだ情報を破棄
    }

    //バッファ登録用のメモリ空間を作成
    CPOINT3D_SKIN_LAYOUT** ppvertex = new CPOINT3D_SKIN_LAYOUT * [sub_mesh_count];

    //ヴァーテックス情報を取得
    for (int i = 0; i < sub_mesh_count; i++)
    {
        //ヴァーテックスサイズを取得 vertexbuffer_size
        fscanf_s(fp, "%s", str, 256);
        sscanf_s(str, "vertexbuffer_size:%d;", &m_pvertex_size[i]);

        //バッファ登録用のメモリ空間を作成
        ppvertex[i] = new CPOINT3D_SKIN_LAYOUT[m_pvertex_size[i]];

        //ヴァーテックス情報を取得
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

    //ヴァーテックスバッファの有無チェック
    fscanf_s(fp, "%s", str, 256);
    if (strcmp(str, "extents:") == 0)
    {
        //行列IDをweight情報無し
        MessageBox(0, L"スキン情報が無いため中止します", NULL, MB_OK);
        return;
    }

    //ヴァーテックスバッファにスキン情報を登録
    for (int i = 0; i < sub_mesh_count; i++)
    {
        //スキンの頂点数獲得 skin_max;
        int skin_max = 0;
        sscanf_s(str, "skin_max:%d;", &skin_max);
        if (skin_max != m_pvertex_size[i])
        {
            MessageBox(0, L"頂点数とスキン追加する頂点数に違いがあるので中止します", NULL, MB_OK);
            return;
        }
        else
        {
            //個々の追加頂点情報取得
            for (int j = 0; j < skin_max; j++)
            {
                fscanf_s(fp, "%s", str, 256);
                fscanf_s(fp, "%s", str, 256);
                sscanf_s(str, "BI:%d,%d,%d,%d,;",   //ボーン行列IDの取得
                    &ppvertex[i][j].m_bone_index[0],
                    &ppvertex[i][j].m_bone_index[1],
                    &ppvertex[i][j].m_bone_index[2],
                    &ppvertex[i][j].m_bone_index[3]
                );
                fscanf_s(fp, "%s", str, 256);
                sscanf_s(str, "BW:%f,%f,%f,%f,;",   //ボーン行列に対しての重みの取得
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


    //ヴァーテックスバッファ登録
    for (int i = 0; i < sub_mesh_count; i++)
    {
        //バッファにバーテックスステータス設定
        D3D11_BUFFER_DESC bd;
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(CPOINT3D_SKIN_LAYOUT) * m_pvertex_size[i];
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.CPUAccessFlags = 0;
        bd.MiscFlags = 0;
       

        //バッファに入れるデータを設定
        D3D11_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = ppvertex[i];
       

        //ステータスとバッファに入れるデータを元にバーテックスバッファ作成
        HRESULT hr = Dev::GetDevice()->CreateBuffer(&bd, &InitData, &m_ppVertexBuffer[i]);
        if (FAILED(hr))
        {
            MessageBox(0, L"バーテックスバッファ作成失敗", NULL, MB_OK);
            return;
        }
        //バッファに登録したppvertexは廃棄
        delete[] ppvertex[i];
    }
    delete[] ppvertex;

    //extents:を飛ばす
    fscanf_s(fp, "%s", str, 256);
    fscanf_s(fp, "%s", str, 256);
    fscanf_s(fp, "%s", str, 256);
    //ボーン情報の取得
    fscanf_s(fp, "%s", str, 256);
    sscanf_s(str, "bone_count:%d;", &m_bone_max);//ボーンの最大取得数

    for (int i = 0; i < m_bone_max; i++)
    {
        fscanf_s(fp, "%s", str, 256);
        sscanf_s(str, "bone_name:%s;", m_bone[i].m_name, 128);//ボーン名取得
        m_bone[i].m_name[strlen(m_bone[i].m_name) - 1] = '\0';//末端の';'を取得
        fscanf_s(fp, "%s", str, 256);
        fscanf_s(fp, "%s", str, 256);
        int index = -1;
        sscanf_s(str, "ParentIndex:%d;", &index);//ボーンの接続元情報取得
        if (index == -1)
        {
            ;//このボーンは始祖のため何もしない
        }
        else
        {
            //接続元のボーンアクセス用ポインタから空いてるポインタを探す
            for (int j = 0; j < 16; j++)
            {
                //空いてるポインタからこのボーンにアクセス
                if (m_bone[index].m_p_bone_next[j] == nullptr)
                {
                    m_bone[index].m_p_bone_next[j] = &m_bone[i];
                    break;
                }
            }
        }
        fscanf_s(fp, "%s", str, 256);

        //各行列情報取得
        fscanf_s(fp, "%s", str, 256);
        //BindPos情報取得
        float* p = m_bone[i].m_bind_pos;//受けとり名が長いので代わりにｐから受けとる
        sscanf_s(str, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,",
            &p[ 0], &p[ 1], &p[ 2], &p[ 3],
            &p[ 4], &p[ 5], &p[ 6], &p[ 7],
            &p[ 8], &p[ 9], &p[10], &p[11],
            &p[12], &p[13], &p[14], &p[15]);
        fscanf_s(fp, "%s", str, 256);

        fscanf_s(fp, "%s", str, 256);
        fscanf_s(fp, "%s", str, 256);
        //InvBindPos情報取得
        p = m_bone[i].m_inv_bind_pos;
        sscanf_s(str, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,",
            &p[ 0], &p[ 1], &p[ 2], &p[ 3],
            &p[ 4], &p[ 5], &p[ 6], &p[ 7],
            &p[ 8], &p[ 9], &p[10], &p[11],
            &p[12], &p[13], &p[14], &p[15]);
        fscanf_s(fp, "%s", str, 256);

        fscanf_s(fp, "%s", str, 256);
        fscanf_s(fp, "%s", str, 256);
        //LocalTransform情報取得
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