//STLデバッグ機能をOFFにする
#define _SECURE_SCL (0)
#define _HAS_ITERATOR_DEBUGGING (0)

#include "DeviceCreate.h"
#include "Draw2DPolygon.h"
#include "DirectXTex.h"
#include "WICTextureLoader11.h"

//GPUで扱う用
ID3D11VertexShader* Draw::m_pVertexShader;   //バーテックスシェーダー
ID3D11PixelShader*  Draw::m_pPixelShader;    //ピクセルシェーダー
ID3D11InputLayout*  Draw::m_pVertexLayout;   //頂点入力レイアウト
ID3D11Buffer*       Draw::m_pConstantBuffer; //コンスタントバッファ
//ポリゴン情報登録用バッファ
ID3D11Buffer* Draw::m_pVertexBuffer;   //バーテックスバッファ
ID3D11Buffer* Draw::m_pIndexBuffer;    //インデックスバッファ

//テクスチャに必要なもの
ID3D11SamplerState*       Draw::m_pSampleLinear;      //テクスチャーサンプラー
ID3D11ShaderResourceView* Draw::m_pTexture[32];       //テクスチャーリソース
float Draw::m_width[32];   //テクスチャの横幅
float Draw::m_height[32];  //テクスチャの縦幅

//3Dモデル用のイメージ情報読み込み
ID3D11ShaderResourceView* CDraw2DPolygon::LoadImage(const wchar_t* img_name)
{
    HRESULT hr = S_OK;
    ID3D11ShaderResourceView* view;
    //設定を元にテクスチャ作成
    hr = DirectX::CreateWICTextureFromFile(Dev::GetDevice(), Dev::GetDeviceContext(), img_name, nullptr, &view, 0U);

    if (FAILED(hr))
    {
        MessageBox(0, L"テクスチャの読み込みが出来ませんでした。", NULL, MB_OK);
        return nullptr;
    }

    return view;
}


//イメージ情報読み込み
void CDraw2DPolygon::LoadImage(int id, const wchar_t* img_name)
{
    //読み込むテクスチャの大きさを測る
    GetLoadImageFileSizeHW(img_name, &m_width[id], &m_height[id]);

    //設定を元にテクスチャー作成
    DirectX::CreateWICTextureFromFile(Dev::GetDevice(), Dev::GetDeviceContext(),img_name, nullptr, &m_pTexture[id], 0U);
}

//文字描画用
void CDraw2DPolygon::Draw2DChar(ID3D11ShaderResourceView* resurec_view, float x, float y, float s, float rgba[4])
{
    //頂点レイアウト
    Dev::GetDeviceContext()->IASetInputLayout(m_pVertexLayout);

    //使用するシェーダーの登録
    Dev::GetDeviceContext()->VSSetShader(m_pVertexShader, NULL, 0);
    Dev::GetDeviceContext()->PSSetShader(m_pPixelShader, NULL, 0);

    //コンスタントバッファを使用するシェーダーに登録
    Dev::GetDeviceContext()->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
    Dev::GetDeviceContext()->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

    //プリミティブ・トポロジーをセット
    Dev::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    //バーテックスバッファ登録
    UINT stride = sizeof(POINT_LAYOUT);
    UINT offset = 0;
    Dev::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

    //インデックスバッファ登録
    Dev::GetDeviceContext()->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    //コンスタントバッファのデータ登録
    D3D11_MAPPED_SUBRESOURCE pData;
    if (SUCCEEDED(Dev::GetDeviceContext()->Map(m_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
    {
        POLYGON_BUFFER data;
        //色
        data.color[0] = rgba[0];
        data.color[1] = rgba[1];
        data.color[2] = rgba[2];
        data.color[3] = rgba[3];

        //位置情報
        data.pos[0] = x;
        data.pos[1] = y;

        //拡大率
        data.scale[0] = s;
        data.scale[1] = s;

        //回転
        data.rotation[0] = 0.0f;

        //イメージサイズ
        data.texsize[0] = 32;
        data.texsize[1] = 32;
        data.texsize[2] = 1.0f;//テクスチャ有り

        memcpy_s(pData.pData, pData.RowPitch, (void*)&data, sizeof(POLYGON_BUFFER));
        //コンスタントバッファをシェーダに輸送
        Dev::GetDeviceContext()->Unmap(m_pConstantBuffer, 0);
    }

    //テクスチャーサンプラを使用
    Dev::GetDeviceContext()->PSSetSamplers(0, 1, &m_pSampleLinear);

    //テクスチャを登録
    Dev::GetDeviceContext()->PSSetShaderResources(0, 1, &resurec_view);


    //登録した情報を元にポリゴンを描画
    Dev::GetDeviceContext()->DrawIndexed(6, 0, 0);
}


//デバッグ当たり判定用描画
void CDraw2DPolygon::Draw2DHitBox(float x, float y, float w, float h, float rgba[4])
{
    //頂点レイアウト
    Dev::GetDeviceContext()->IASetInputLayout(m_pVertexLayout);

    //使用するシェーダーの登録
    Dev::GetDeviceContext()->VSSetShader(m_pVertexShader, NULL, 0);
    Dev::GetDeviceContext()->PSSetShader(m_pPixelShader, NULL, 0);

    //コンスタントバッファを使用するシェーダーに登録
    Dev::GetDeviceContext()->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
    Dev::GetDeviceContext()->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

    //プリミティブ・トポロジーをセット
    Dev::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    //バーテックスバッファ登録
    UINT stride = sizeof(POINT_LAYOUT);
    UINT offset = 0;
    Dev::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

    //インデックスバッファ登録
    Dev::GetDeviceContext()->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    //コンスタントバッファのデータ登録
    D3D11_MAPPED_SUBRESOURCE pData;
    if (SUCCEEDED(Dev::GetDeviceContext()->Map(m_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
    {
        POLYGON_BUFFER data;
        data.color[0] = rgba[0];
        data.color[1] = rgba[1];
        data.color[2] = rgba[2];
        data.color[3] = rgba[3];

        //位置情報
        data.pos[0] = x;
        data.pos[1] = y;

        //拡大率
        data.scale[0] = 1.0f;
        data.scale[1] = 1.0f;

        //回転
        data.rotation[0] = 0.0f;

        //イメージサイズ
        data.texsize[0] = w;
        data.texsize[1] = h;
        //テクスチャなし
        data.texsize[2] = 0.0f;

        memcpy_s(pData.pData, pData.RowPitch, (void*)&data, sizeof(POLYGON_BUFFER));
        //コンスタントバッファをシェーダに輸送
        Dev::GetDeviceContext()->Unmap(m_pConstantBuffer, 0);
    }

    //テクスチャーサンプラを使用
    Dev::GetDeviceContext()->PSSetSamplers(0, 1, &m_pSampleLinear);

    //登録した情報を元にポリゴンを描画
    Dev::GetDeviceContext()->DrawIndexed(6, 0, 0);

}


//描画
void CDraw2DPolygon::Draw2D(int id,float x, float y, float sx, float sy,float r)
{
    //頂点レイアウト
    Dev::GetDeviceContext()->IASetInputLayout(m_pVertexLayout);

    //使用するシェーダーの登録
    Dev::GetDeviceContext()->VSSetShader(m_pVertexShader, NULL, 0);
    Dev::GetDeviceContext()->PSSetShader(m_pPixelShader, NULL, 0);

    //コンスタントバッファを使用するシェーダーに登録
    Dev::GetDeviceContext()->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
    Dev::GetDeviceContext()->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

    //プリミティブ・トポロジーをセット
    Dev::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    //バーテックスバッファ登録
    UINT stride = sizeof(POINT_LAYOUT);
    UINT offset = 0;
    Dev::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

    //インデックスバッファ登録
    Dev::GetDeviceContext()->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    //コンスタントバッファのデータ登録
    D3D11_MAPPED_SUBRESOURCE pData;
    if (SUCCEEDED(Dev::GetDeviceContext()->Map(m_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
    {
        POLYGON_BUFFER data;
        data.color[0] = 1.0f;
        data.color[1] = 1.0f;
        data.color[2] = 1.0f;
        data.color[3] = 1.0f;

        //位置情報
        data.pos[0] = x;
        data.pos[1] = y;

        //拡大率
        data.scale[0] = sx;
        data.scale[1] = sy;

        //回転
        data.rotation[0] = r;

        //イメージサイズ
        data.texsize[0] = m_width[id];
        data.texsize[1] = m_height[id];
        data.texsize[2] = 1.0f;//テクスチャ有り

        memcpy_s(pData.pData, pData.RowPitch, (void*)&data, sizeof(POLYGON_BUFFER));
        //コンスタントバッファをシェーダに輸送
        Dev::GetDeviceContext()->Unmap(m_pConstantBuffer, 0);
    }

    //テクスチャーサンプラを使用
    Dev::GetDeviceContext()->PSSetSamplers(0, 1, &m_pSampleLinear);
    
    //テクスチャを登録
    Dev::GetDeviceContext()->PSSetShaderResources(0, 1, &m_pTexture[id]);


    //登録した情報を元にポリゴンを描画
    Dev::GetDeviceContext()->DrawIndexed(6, 0, 0);

}

//ポリゴン表示環境の初期化
HRESULT CDraw2DPolygon::InitPolygonRender()
{
    //テクスチャの配列の初期化
    for (int i = 0; i < 32; i++)
    {
        m_pTexture[i] = nullptr;
    }

    HRESULT hr = S_OK;

    //hlslファイル名
    const wchar_t* hlsl_name = L"PolygonDraw.hlsl";

    //hlslファイルを読み込み　ブロブ作成　ブロブとはシェーダーの塊みたいなもの
    //XXシェーダーとして特徴を持たない。後で各種シェーダーとなる
    ID3DBlob* pCompiledShader = NULL;
    ID3DBlob* pErrors = NULL;

    //ブロブからバーテックスシェーダーコンパイル
    hr = D3DCompileFromFile(hlsl_name, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "vs", "vs_4_0", 0, NULL, &pCompiledShader, &pErrors);

    if (FAILED(hr))
    {
        //エラーがある場合、ｃがデバッグ情報を持つ
        char* c = (char*)pErrors->GetBufferPointer();
        MessageBox(0, L"hlsl読み込み失敗1", NULL, MB_OK);
        SAFE_RELEASE(pErrors);
        return hr;
    }
    //コンパイルしたバーデックスシェーダーを元にインターフェースを作成
    hr = Dev::GetDevice()->CreateVertexShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(),
        NULL, &m_pVertexShader);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pCompiledShader);
        MessageBox(0, L"バーテックスシェーダー作成失敗", NULL, MB_OK);
        return hr;
    }
    //頂点インプットレイアウトを定義
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"UV"   ,0,DXGI_FORMAT_R32G32_FLOAT,      0,28,D3D11_INPUT_PER_VERTEX_DATA,0},
    };
    UINT numElements = sizeof(layout) / sizeof(layout[0]);

    //頂点インプットレイアウトを作成・レイアウトをセット
    hr = Dev::GetDevice()->CreateInputLayout(layout, numElements, pCompiledShader->GetBufferPointer(),
        pCompiledShader->GetBufferSize(), &m_pVertexLayout);
    if (FAILED(hr))
    {
        MessageBox(0, L"レイアウト作成失敗", NULL, MB_OK);
        return hr;
    }
    SAFE_RELEASE(pCompiledShader);

    //ブロブからピクセルシェーダーコンパイル
    hr = D3DCompileFromFile(hlsl_name, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "ps", "ps_4_0", 0, NULL, &pCompiledShader, &pErrors);
    if (FAILED(hr))
    {
        //エラーがある場合、ｃがデバック情報を持つ
        char* c = (char*)pErrors->GetBufferPointer();
        MessageBox(0, L"hlsl読み込み失敗2", NULL, MB_OK);
        SAFE_RELEASE(pErrors);
        return hr;
    }
    //コンパイルしたピクセルシェーダーでインターフェースを作成
    hr = Dev::GetDevice()->CreatePixelShader(pCompiledShader->GetBufferPointer(),
        pCompiledShader->GetBufferSize(), NULL, &m_pPixelShader);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pCompiledShader);
        MessageBox(0, L"ピクセルシェーダー作成失敗", NULL, MB_OK);
    }
    SAFE_RELEASE(pCompiledShader);

    //三角ポリゴンの各頂点の情報
    POINT_LAYOUT vertices[] =
    {  //  x     y    z     r     g   b    a      u     v
        {{1.0f,0.0f,0.0f},{0.5f,0.5f,0.5f,1.0f},{1.0f,0.0f},},//頂点1
        {{0.0f,0.0f,0.0f},{0.5f,0.5f,0.5f,1.0f},{0.0f,0.0f},},//頂点2
        {{0.0f,1.0f,0.0f},{0.5f,0.5f,0.5f,1.0f},{0.0f,1.0f},},//頂点3
        {{1.0f,1.0f,0.0f},{0.5f,0.5f,0.5f,1.0f},{1.0f,1.0f},},//頂点4
    };
    //バッファにバーテックスステータス設定
    D3D11_BUFFER_DESC bd;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(POINT_LAYOUT) * 4;
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
        return hr;
    }
    //ポリゴンのインデックス情報
    unsigned short hIndexData[2][3] =
    {
        {0,1,2,},//1面
        {0,3,2,},//2面
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
        return hr;
    }

    //バッファにコンスタントバッファ（シェーダにデータ受け渡し用）ステータス作成
    D3D11_BUFFER_DESC cb;
    cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cb.ByteWidth = sizeof(POLYGON_BUFFER);
    cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cb.MiscFlags = 0;
    cb.StructureByteStride = 0;
    cb.Usage = D3D11_USAGE_DYNAMIC;

    //ステータスを元にコンスタントバッファを作成
    hr = Dev::GetDevice()->CreateBuffer(&cb, NULL, &m_pConstantBuffer);
    if (FAILED(hr))
    {
        MessageBox(0, L"コンスタントバッファ作成失敗", NULL, MB_OK);
        return hr;
    }

    //テクスチャー用サンプラー作成
    D3D11_SAMPLER_DESC SamDesc;
    ZeroMemory(&SamDesc, sizeof(D3D11_SAMPLER_DESC));

    SamDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    SamDesc.BorderColor[0] = 0.0f;
    SamDesc.BorderColor[1] = 0.0f;
    SamDesc.BorderColor[2] = 0.0f;
    SamDesc.BorderColor[3] = 0.0f;
    SamDesc.MipLODBias = 0.0f;
    SamDesc.MaxAnisotropy = 2;
    SamDesc.MinLOD = 0.0f;
    SamDesc.MaxLOD = D3D11_FLOAT32_MAX;
    SamDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    Dev::GetDevice()->CreateSamplerState(&SamDesc, &m_pSampleLinear);

   

    return hr;
}

//ポリゴン表示環境の破棄
void CDraw2DPolygon::DeletePolygonRender()
{
    //テクスチャ情報の破棄
    SAFE_RELEASE(m_pSampleLinear);

    for(int i=0;i<32;i++)
        SAFE_RELEASE(m_pTexture[i]);

    //GPU側で扱う用
    SAFE_RELEASE(m_pVertexShader);
    SAFE_RELEASE(m_pPixelShader);
    SAFE_RELEASE(m_pVertexLayout);

    //ポリゴン情報登録用バッファ
    SAFE_RELEASE(m_pConstantBuffer);
    SAFE_RELEASE(m_pVertexBuffer);
    SAFE_RELEASE(m_pIndexBuffer);
}
