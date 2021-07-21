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


//三角錐Model作成
void CMODEL::CreateSampleTriangularpyramid()
{
	HRESULT hr = S_OK;

	//三角錐を形成するための頂点の情報
	CPOINT3D_LAYOUT vertices[] =
	{
		//    px    py   pz     nx    ny   nz      u    v       r    g    b    a
		   {{ 0.5f,0.0f,0.0f },{0.0f,0.0f,0.0f} ,{0.0f,0.0f} ,{1.0f,0.0f,0.0f,1.0f},},//頂点1
		   {{-0.5f,0.0f,0.0f },{0.0f,0.0f,0.0f} ,{0.0f,0.0f} ,{0.0f,1.0f,0.0f,1.0f},},//頂点2
		   {{ 0.0f,0.0f,0.5f },{0.0f,0.0f,0.0f} ,{0.0f,0.0f} ,{0.0f,0.0f,1.0f,1.0f},},//頂点3
		   {{ 0.0f,0.5f,0.25f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f} ,{1.0f,1.0f,1.0f,1.0f},},//頂点4

	};

    //ヴァーテックス・インデックスバッファ・材質量を決める（一つのみ）
    m_ppVertexBuffer = new ID3D11Buffer * [1];
    m_ppIndexBuffer  = new ID3D11Buffer * [1];
    m_Material       = new CMATERIAL[1];

    //各バッファ最大数を入れる配列の作成とマテリアルカウントの初期化
    this->m_material_max = 1; //三角錐は材質を使っていないが0だと0番目のバッファ描画されないので1を入れる
    m_pvertex_size = new int[1];
    m_pindex_size = new int[1];

    //バッファにバーテックスステータス設定
    D3D11_BUFFER_DESC bd;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;

    //バッファに入れるデータを設定
    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = vertices;

    //ステータスとバッファに入れるデータを元にバーテックスバッファ作成
    hr = Dev::GetDevice()->CreateBuffer(&bd, &InitData, &m_ppVertexBuffer[0]);
    if (FAILED(hr))
    {
        MessageBox(0, L"バーテックスバッファ作成失敗", NULL, MB_OK);
        return;
    }


    //三角錐の面を形成するための頂点インデックス情報
    unsigned short hIndexData[4][3] =
    {
        {3,1,0,},//1面
        {0,2,3,},//2面
        {3,2,1,},//3面
        {1,2,0,},//4面
    };

    //バッファにインデックスステータス設定
    D3D11_BUFFER_DESC hBufferDesc;
    hBufferDesc.ByteWidth = sizeof(hIndexData);
    hBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    hBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    hBufferDesc.CPUAccessFlags = 0;
    hBufferDesc.MiscFlags = 0;
    hBufferDesc.StructureByteStride = sizeof(unsigned short);

    //バッファに入れるデータを設定
    D3D11_SUBRESOURCE_DATA hSubResourceData;
    hSubResourceData.pSysMem = hIndexData;
    hSubResourceData.SysMemPitch = 0;
    hSubResourceData.SysMemSlicePitch = 0;

    //ステータスとバッファに入れるデータを元にインデックスバッファ作成
    hr = Dev::GetDevice()->CreateBuffer(&hBufferDesc, &hSubResourceData, &m_ppIndexBuffer[0]);
    if (FAILED(hr))
    {
        MessageBox(0, L"インデックスバッファ作成失敗", NULL, MB_OK);
        return;
    }

    //材質情報
    memset(m_Material->m_ambient,  0x00,  sizeof(m_Material->m_ambient ));
    memset(m_Material->m_diffuse,  0x00,  sizeof(m_Material->m_diffuse ));
    memset(m_Material->m_specular, 0x00,  sizeof(m_Material->m_specular));
    m_Material->m_pTexture = nullptr;
    m_Material->m_pTexture = nullptr;

    //頂点・インデックスの大きさ
    m_pindex_size[0]  = 4;
    m_pvertex_size[0] = 4;

   
}

//cmoファイルの読み込み
void CMODEL::LoadCmoModel(const wchar_t* name)
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
        m_ppVertexBuffer = new ID3D11Buffer * [m_material_max];
        m_ppIndexBuffer  = new ID3D11Buffer * [m_material_max];
        m_Material       = new CMATERIAL[m_material_max];
        m_pvertex_size   = new int[m_material_max];
        m_pindex_size    = new int[m_material_max];
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
        //Texture取得予定
        for (int i = 0; i < 15; i++)
        {
            //テクスチャがあるかどうかを判断 Texture:
            fscanf_s(fp, "%s", str, 256);
            if (strcmp(str, "}") == 0)//文字列に"{"でテクスチャ有無判断
            {
                m_Material[i].m_pTexture = nullptr;
                break;
            }
            //ここでテクスチャを読み込む予定
            m_Material[i].m_pTexture = nullptr;
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
            ,&a,&b,&c,&d,&e,&m_pindex_size[i]);
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

    //ヴァーテックス情報を取得
    for (int i = 0; i < sub_mesh_count; i++)
    {
        //ヴァーテックスサイズを取得 vertexbuffer_size
        fscanf_s(fp, "%s", str, 256);
        sscanf_s(str, "vertexbuffer_size:%d;", &m_pvertex_size[i]);
        //バッファ登録用のメモリ空間を作成
        CPOINT3D_LAYOUT* pvertex = new CPOINT3D_LAYOUT[m_pvertex_size[i]];

        //ヴァーテックス情報を取得
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

        //バッファにバーテックスステータス設定
        D3D11_BUFFER_DESC bd;
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(CPOINT3D_LAYOUT) * m_pvertex_size[i];
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = 0;
        bd.MiscFlags = 0;

        //バッファに入れるデータを設定
        D3D11_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = pvertex;

        //ステータスとバッファに入れるデータをもとにバーテックスバッファ作成
        HRESULT hr = Dev::GetDevice()->CreateBuffer(&bd, &InitData, &m_ppVertexBuffer[i]);
        if (FAILED(hr))
        {
            MessageBox(0, L"バーテックスバッファ作成失敗", NULL, MB_OK);
            return;
        }

        delete pvertex;//バッファ化したのでヴァーテックス情報破棄
    }


    fclose(fp);
}