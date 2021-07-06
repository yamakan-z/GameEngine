#include "DeviceCreate.h"
#include "Render3D.h"

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
    hr = Dev::GetDevice()->CreateBuffer(&bd, &InitData, &m_pVertexBuffer);
    if (FAILED(hr))
    {
        MessageBox(0, L"バーテックスバッファ作成失敗", NULL, MB_OK);
        return;
    }


    //三角錐の面を形成するための頂点インデックス情報
    unsigned short hIndexData[4][3] =
    {
        {0,1,2,},//1面
        {0,3,2,},//2面
        {2,3,1,},//3面
        {0,2,1,},//4面
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
    hr = Dev::GetDevice()->CreateBuffer(&hBufferDesc, &hSubResourceData, &m_pIndexBuffer);
    if (FAILED(hr))
    {
        MessageBox(0, L"インデックスバッファ作成失敗", NULL, MB_OK);
        return;
    }

    //材質情報
    memset(m_Material.m_ambient,  0x00,  sizeof(m_Material.m_ambient ));
    memset(m_Material.m_diffuse,  0x00,  sizeof(m_Material.m_diffuse ));
    memset(m_Material.m_specular, 0x00,  sizeof(m_Material.m_specular));
    m_Material.m_pTexture = nullptr;

    //頂点・インデックスの大きさ
    m_index_size = 4;
    m_vertex_size = 4;
}