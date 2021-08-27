//STL�f�o�b�O�@�\��OFF�ɂ���
#define _SECURE_SCL (0)
#define _HAS_ITERATOR_DEBUGGING (0)

#include "DeviceCreate.h"
#include "Draw2DPolygon.h"
#include "DirectXTex.h"
#include "WICTextureLoader11.h"

//GPU�ň����p
ID3D11VertexShader* Draw::m_pVertexShader;   //�o�[�e�b�N�X�V�F�[�_�[
ID3D11PixelShader*  Draw::m_pPixelShader;    //�s�N�Z���V�F�[�_�[
ID3D11InputLayout*  Draw::m_pVertexLayout;   //���_���̓��C�A�E�g
ID3D11Buffer*       Draw::m_pConstantBuffer; //�R���X�^���g�o�b�t�@
//�|���S�����o�^�p�o�b�t�@
ID3D11Buffer* Draw::m_pVertexBuffer;   //�o�[�e�b�N�X�o�b�t�@
ID3D11Buffer* Draw::m_pIndexBuffer;    //�C���f�b�N�X�o�b�t�@

//�e�N�X�`���ɕK�v�Ȃ���
ID3D11SamplerState*       Draw::m_pSampleLinear;      //�e�N�X�`���[�T���v���[
ID3D11ShaderResourceView* Draw::m_pTexture[32];       //�e�N�X�`���[���\�[�X
float Draw::m_width[32];   //�e�N�X�`���̉���
float Draw::m_height[32];  //�e�N�X�`���̏c��

//3D���f���p�̃C���[�W���ǂݍ���
ID3D11ShaderResourceView* CDraw2DPolygon::LoadImage(const wchar_t* img_name)
{
    HRESULT hr = S_OK;
    ID3D11ShaderResourceView* view;
    //�ݒ�����Ƀe�N�X�`���쐬
    hr = DirectX::CreateWICTextureFromFile(Dev::GetDevice(), Dev::GetDeviceContext(), img_name, nullptr, &view, 0U);

    if (FAILED(hr))
    {
        MessageBox(0, L"�e�N�X�`���̓ǂݍ��݂��o���܂���ł����B", NULL, MB_OK);
        return nullptr;
    }

    return view;
}


//�C���[�W���ǂݍ���
void CDraw2DPolygon::LoadImage(int id, const wchar_t* img_name)
{
    //�ǂݍ��ރe�N�X�`���̑傫���𑪂�
    GetLoadImageFileSizeHW(img_name, &m_width[id], &m_height[id]);

    //�ݒ�����Ƀe�N�X�`���[�쐬
    DirectX::CreateWICTextureFromFile(Dev::GetDevice(), Dev::GetDeviceContext(),img_name, nullptr, &m_pTexture[id], 0U);
}

//�����`��p
void CDraw2DPolygon::Draw2DChar(ID3D11ShaderResourceView* resurec_view, float x, float y, float s, float rgba[4])
{
    //���_���C�A�E�g
    Dev::GetDeviceContext()->IASetInputLayout(m_pVertexLayout);

    //�g�p����V�F�[�_�[�̓o�^
    Dev::GetDeviceContext()->VSSetShader(m_pVertexShader, NULL, 0);
    Dev::GetDeviceContext()->PSSetShader(m_pPixelShader, NULL, 0);

    //�R���X�^���g�o�b�t�@���g�p����V�F�[�_�[�ɓo�^
    Dev::GetDeviceContext()->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
    Dev::GetDeviceContext()->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

    //�v���~�e�B�u�E�g�|���W�[���Z�b�g
    Dev::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    //�o�[�e�b�N�X�o�b�t�@�o�^
    UINT stride = sizeof(POINT_LAYOUT);
    UINT offset = 0;
    Dev::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

    //�C���f�b�N�X�o�b�t�@�o�^
    Dev::GetDeviceContext()->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    //�R���X�^���g�o�b�t�@�̃f�[�^�o�^
    D3D11_MAPPED_SUBRESOURCE pData;
    if (SUCCEEDED(Dev::GetDeviceContext()->Map(m_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
    {
        POLYGON_BUFFER data;
        //�F
        data.color[0] = rgba[0];
        data.color[1] = rgba[1];
        data.color[2] = rgba[2];
        data.color[3] = rgba[3];

        //�ʒu���
        data.pos[0] = x;
        data.pos[1] = y;

        //�g�嗦
        data.scale[0] = s;
        data.scale[1] = s;

        //��]
        data.rotation[0] = 0.0f;

        //�C���[�W�T�C�Y
        data.texsize[0] = 32;
        data.texsize[1] = 32;
        data.texsize[2] = 1.0f;//�e�N�X�`���L��

        memcpy_s(pData.pData, pData.RowPitch, (void*)&data, sizeof(POLYGON_BUFFER));
        //�R���X�^���g�o�b�t�@���V�F�[�_�ɗA��
        Dev::GetDeviceContext()->Unmap(m_pConstantBuffer, 0);
    }

    //�e�N�X�`���[�T���v�����g�p
    Dev::GetDeviceContext()->PSSetSamplers(0, 1, &m_pSampleLinear);

    //�e�N�X�`����o�^
    Dev::GetDeviceContext()->PSSetShaderResources(0, 1, &resurec_view);


    //�o�^�����������Ƀ|���S����`��
    Dev::GetDeviceContext()->DrawIndexed(6, 0, 0);
}


//�f�o�b�O�����蔻��p�`��
void CDraw2DPolygon::Draw2DHitBox(float x, float y, float w, float h, float rgba[4])
{
    //���_���C�A�E�g
    Dev::GetDeviceContext()->IASetInputLayout(m_pVertexLayout);

    //�g�p����V�F�[�_�[�̓o�^
    Dev::GetDeviceContext()->VSSetShader(m_pVertexShader, NULL, 0);
    Dev::GetDeviceContext()->PSSetShader(m_pPixelShader, NULL, 0);

    //�R���X�^���g�o�b�t�@���g�p����V�F�[�_�[�ɓo�^
    Dev::GetDeviceContext()->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
    Dev::GetDeviceContext()->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

    //�v���~�e�B�u�E�g�|���W�[���Z�b�g
    Dev::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    //�o�[�e�b�N�X�o�b�t�@�o�^
    UINT stride = sizeof(POINT_LAYOUT);
    UINT offset = 0;
    Dev::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

    //�C���f�b�N�X�o�b�t�@�o�^
    Dev::GetDeviceContext()->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    //�R���X�^���g�o�b�t�@�̃f�[�^�o�^
    D3D11_MAPPED_SUBRESOURCE pData;
    if (SUCCEEDED(Dev::GetDeviceContext()->Map(m_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
    {
        POLYGON_BUFFER data;
        data.color[0] = rgba[0];
        data.color[1] = rgba[1];
        data.color[2] = rgba[2];
        data.color[3] = rgba[3];

        //�ʒu���
        data.pos[0] = x;
        data.pos[1] = y;

        //�g�嗦
        data.scale[0] = 1.0f;
        data.scale[1] = 1.0f;

        //��]
        data.rotation[0] = 0.0f;

        //�C���[�W�T�C�Y
        data.texsize[0] = w;
        data.texsize[1] = h;
        //�e�N�X�`���Ȃ�
        data.texsize[2] = 0.0f;

        memcpy_s(pData.pData, pData.RowPitch, (void*)&data, sizeof(POLYGON_BUFFER));
        //�R���X�^���g�o�b�t�@���V�F�[�_�ɗA��
        Dev::GetDeviceContext()->Unmap(m_pConstantBuffer, 0);
    }

    //�e�N�X�`���[�T���v�����g�p
    Dev::GetDeviceContext()->PSSetSamplers(0, 1, &m_pSampleLinear);

    //�o�^�����������Ƀ|���S����`��
    Dev::GetDeviceContext()->DrawIndexed(6, 0, 0);

}


//�`��
void CDraw2DPolygon::Draw2D(int id,float x, float y, float sx, float sy,float r)
{
    //���_���C�A�E�g
    Dev::GetDeviceContext()->IASetInputLayout(m_pVertexLayout);

    //�g�p����V�F�[�_�[�̓o�^
    Dev::GetDeviceContext()->VSSetShader(m_pVertexShader, NULL, 0);
    Dev::GetDeviceContext()->PSSetShader(m_pPixelShader, NULL, 0);

    //�R���X�^���g�o�b�t�@���g�p����V�F�[�_�[�ɓo�^
    Dev::GetDeviceContext()->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
    Dev::GetDeviceContext()->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

    //�v���~�e�B�u�E�g�|���W�[���Z�b�g
    Dev::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    //�o�[�e�b�N�X�o�b�t�@�o�^
    UINT stride = sizeof(POINT_LAYOUT);
    UINT offset = 0;
    Dev::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

    //�C���f�b�N�X�o�b�t�@�o�^
    Dev::GetDeviceContext()->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    //�R���X�^���g�o�b�t�@�̃f�[�^�o�^
    D3D11_MAPPED_SUBRESOURCE pData;
    if (SUCCEEDED(Dev::GetDeviceContext()->Map(m_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
    {
        POLYGON_BUFFER data;
        data.color[0] = 1.0f;
        data.color[1] = 1.0f;
        data.color[2] = 1.0f;
        data.color[3] = 1.0f;

        //�ʒu���
        data.pos[0] = x;
        data.pos[1] = y;

        //�g�嗦
        data.scale[0] = sx;
        data.scale[1] = sy;

        //��]
        data.rotation[0] = r;

        //�C���[�W�T�C�Y
        data.texsize[0] = m_width[id];
        data.texsize[1] = m_height[id];
        data.texsize[2] = 1.0f;//�e�N�X�`���L��

        memcpy_s(pData.pData, pData.RowPitch, (void*)&data, sizeof(POLYGON_BUFFER));
        //�R���X�^���g�o�b�t�@���V�F�[�_�ɗA��
        Dev::GetDeviceContext()->Unmap(m_pConstantBuffer, 0);
    }

    //�e�N�X�`���[�T���v�����g�p
    Dev::GetDeviceContext()->PSSetSamplers(0, 1, &m_pSampleLinear);
    
    //�e�N�X�`����o�^
    Dev::GetDeviceContext()->PSSetShaderResources(0, 1, &m_pTexture[id]);


    //�o�^�����������Ƀ|���S����`��
    Dev::GetDeviceContext()->DrawIndexed(6, 0, 0);

}

//�|���S���\�����̏�����
HRESULT CDraw2DPolygon::InitPolygonRender()
{
    //�e�N�X�`���̔z��̏�����
    for (int i = 0; i < 32; i++)
    {
        m_pTexture[i] = nullptr;
    }

    HRESULT hr = S_OK;

    //hlsl�t�@�C����
    const wchar_t* hlsl_name = L"PolygonDraw.hlsl";

    //hlsl�t�@�C����ǂݍ��݁@�u���u�쐬�@�u���u�Ƃ̓V�F�[�_�[�̉�݂����Ȃ���
    //XX�V�F�[�_�[�Ƃ��ē����������Ȃ��B��Ŋe��V�F�[�_�[�ƂȂ�
    ID3DBlob* pCompiledShader = NULL;
    ID3DBlob* pErrors = NULL;

    //�u���u����o�[�e�b�N�X�V�F�[�_�[�R���p�C��
    hr = D3DCompileFromFile(hlsl_name, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "vs", "vs_4_0", 0, NULL, &pCompiledShader, &pErrors);

    if (FAILED(hr))
    {
        //�G���[������ꍇ�A�����f�o�b�O��������
        char* c = (char*)pErrors->GetBufferPointer();
        MessageBox(0, L"hlsl�ǂݍ��ݎ��s1", NULL, MB_OK);
        SAFE_RELEASE(pErrors);
        return hr;
    }
    //�R���p�C�������o�[�f�b�N�X�V�F�[�_�[�����ɃC���^�[�t�F�[�X���쐬
    hr = Dev::GetDevice()->CreateVertexShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(),
        NULL, &m_pVertexShader);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pCompiledShader);
        MessageBox(0, L"�o�[�e�b�N�X�V�F�[�_�[�쐬���s", NULL, MB_OK);
        return hr;
    }
    //���_�C���v�b�g���C�A�E�g���`
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"UV"   ,0,DXGI_FORMAT_R32G32_FLOAT,      0,28,D3D11_INPUT_PER_VERTEX_DATA,0},
    };
    UINT numElements = sizeof(layout) / sizeof(layout[0]);

    //���_�C���v�b�g���C�A�E�g���쐬�E���C�A�E�g���Z�b�g
    hr = Dev::GetDevice()->CreateInputLayout(layout, numElements, pCompiledShader->GetBufferPointer(),
        pCompiledShader->GetBufferSize(), &m_pVertexLayout);
    if (FAILED(hr))
    {
        MessageBox(0, L"���C�A�E�g�쐬���s", NULL, MB_OK);
        return hr;
    }
    SAFE_RELEASE(pCompiledShader);

    //�u���u����s�N�Z���V�F�[�_�[�R���p�C��
    hr = D3DCompileFromFile(hlsl_name, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "ps", "ps_4_0", 0, NULL, &pCompiledShader, &pErrors);
    if (FAILED(hr))
    {
        //�G���[������ꍇ�A�����f�o�b�N��������
        char* c = (char*)pErrors->GetBufferPointer();
        MessageBox(0, L"hlsl�ǂݍ��ݎ��s2", NULL, MB_OK);
        SAFE_RELEASE(pErrors);
        return hr;
    }
    //�R���p�C�������s�N�Z���V�F�[�_�[�ŃC���^�[�t�F�[�X���쐬
    hr = Dev::GetDevice()->CreatePixelShader(pCompiledShader->GetBufferPointer(),
        pCompiledShader->GetBufferSize(), NULL, &m_pPixelShader);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pCompiledShader);
        MessageBox(0, L"�s�N�Z���V�F�[�_�[�쐬���s", NULL, MB_OK);
    }
    SAFE_RELEASE(pCompiledShader);

    //�O�p�|���S���̊e���_�̏��
    POINT_LAYOUT vertices[] =
    {  //  x     y    z     r     g   b    a      u     v
        {{1.0f,0.0f,0.0f},{0.5f,0.5f,0.5f,1.0f},{1.0f,0.0f},},//���_1
        {{0.0f,0.0f,0.0f},{0.5f,0.5f,0.5f,1.0f},{0.0f,0.0f},},//���_2
        {{0.0f,1.0f,0.0f},{0.5f,0.5f,0.5f,1.0f},{0.0f,1.0f},},//���_3
        {{1.0f,1.0f,0.0f},{0.5f,0.5f,0.5f,1.0f},{1.0f,1.0f},},//���_4
    };
    //�o�b�t�@�Ƀo�[�e�b�N�X�X�e�[�^�X�ݒ�
    D3D11_BUFFER_DESC bd;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(POINT_LAYOUT) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;

    //�o�b�t�@�ɓ����f�[�^��ݒ�
    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = vertices;

    //�X�e�[�^�X�ƃo�b�t�@�ɓ����f�[�^�����Ƀo�[�e�b�N�X�o�b�t�@�쐬
    hr = Dev::GetDevice()->CreateBuffer(&bd, &InitData, &m_pVertexBuffer);
    if (FAILED(hr))
    {
        MessageBox(0, L"�o�[�e�b�N�X�o�b�t�@�쐬���s", NULL, MB_OK);
        return hr;
    }
    //�|���S���̃C���f�b�N�X���
    unsigned short hIndexData[2][3] =
    {
        {0,1,2,},//1��
        {0,3,2,},//2��
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
    hr = Dev::GetDevice()->CreateBuffer(&hBufferDesc, &hSubResourceData, &m_pIndexBuffer);
    if (FAILED(hr))
    {
        MessageBox(0, L"�C���f�b�N�X�o�b�t�@�쐬���s", NULL, MB_OK);
        return hr;
    }

    //�o�b�t�@�ɃR���X�^���g�o�b�t�@�i�V�F�[�_�Ƀf�[�^�󂯓n���p�j�X�e�[�^�X�쐬
    D3D11_BUFFER_DESC cb;
    cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cb.ByteWidth = sizeof(POLYGON_BUFFER);
    cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cb.MiscFlags = 0;
    cb.StructureByteStride = 0;
    cb.Usage = D3D11_USAGE_DYNAMIC;

    //�X�e�[�^�X�����ɃR���X�^���g�o�b�t�@���쐬
    hr = Dev::GetDevice()->CreateBuffer(&cb, NULL, &m_pConstantBuffer);
    if (FAILED(hr))
    {
        MessageBox(0, L"�R���X�^���g�o�b�t�@�쐬���s", NULL, MB_OK);
        return hr;
    }

    //�e�N�X�`���[�p�T���v���[�쐬
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

//�|���S���\�����̔j��
void CDraw2DPolygon::DeletePolygonRender()
{
    //�e�N�X�`�����̔j��
    SAFE_RELEASE(m_pSampleLinear);

    for(int i=0;i<32;i++)
        SAFE_RELEASE(m_pTexture[i]);

    //GPU���ň����p
    SAFE_RELEASE(m_pVertexShader);
    SAFE_RELEASE(m_pPixelShader);
    SAFE_RELEASE(m_pVertexLayout);

    //�|���S�����o�^�p�o�b�t�@
    SAFE_RELEASE(m_pConstantBuffer);
    SAFE_RELEASE(m_pVertexBuffer);
    SAFE_RELEASE(m_pIndexBuffer);
}
