//STLデバッグ機能をOFFにする
#define _SECURE_SCL (0)
#define _HAS_ITERATOR_DEBUGGING (0)

#include "Audio.h"

#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"libogg.lib")
#pragma comment(lib,"libvorbis_static.lib")
#pragma comment(lib,"libvorbisfile_static.lib")

#include<vector>
#include"vorbis/vorbisfile.h"
using namespace std;


IXAudio2*               CAudio::m_pXAudio2;                   //XAudio2オブジェクト
IXAudio2MasteringVoice* CAudio::m_pMasteringVoice;            //マスターボイス
ChunkInfo               CAudio::m_DataChunk;                  //サウンド情報
unsigned char*          CAudio::m_pResourceData;              //サウンドファイル情報を持つポインタ
IXAudio2SourceVoice*    CAudio::m_pSourceVoice;               //サウンドボイスインターフェース
IXAudio2SubmixVoice*    CAudio::m_pSFXSubmixVoice;            //サブミクスインターフェース
ChunkInfo               CAudio::m_SEDataChunk[32];            //SE用のサウンド情報
unsigned char*          CAudio::m_pSEResourceData[32];        //SE用のサウンドファイル情報を持つポインタ
IXAudio2SourceVoice*    CAudio::m_pSESourceVoice[32][16];     //SE用のサウンドボイスインターフェース
IXAudio2SubmixVoice*    CAudio::m_pSESFXSubmixVoice[32];      //SE用のサブミクスインターフェース

//BackMusic用のミュージックのボリューム調整
void CAudio::LoopMusicVolume(float t)
{
    m_pSFXSubmixVoice->SetVolume(t, 0);
}

//SE用ミュージックのボリューム調整
void CAudio::SEMusicVolume(int id, float t)
{
    m_pSESFXSubmixVoice[id]->SetVolume(t, 0);
}

//マスターボリューム調整
void CAudio::MasterVolume(float t)
{
    m_pMasteringVoice->SetVolume(t, 0);
}

//単発SE用の音楽再生
void CAudio::StartMusic(int id)
{
    //サウンドバッファにLoadBackMusicで読み込んだ波形情報を単発再生で設定
    XAUDIO2_BUFFER SoundBuffer = { 0 };
    SoundBuffer.AudioBytes = m_SEDataChunk[id].Size;
    SoundBuffer.pAudioData = reinterpret_cast<BYTE*>(m_SEDataChunk[id].pData);
    SoundBuffer.LoopCount = 0;
    SoundBuffer.Flags = XAUDIO2_END_OF_STREAM;

    for (int i = 0; i < 32; i++)
    {
        //iがバッファの最大要素数をこえないようにする
        int buffer_count = i % 16;

        //多重再生サウンドバッファの空を探す
        XAUDIO2_VOICE_STATE stats;
        m_pSESourceVoice[id][buffer_count]->GetState(&stats);
        if (stats.BuffersQueued == 0)
        {
            //サウンドバッファをセット
            m_pSESourceVoice[id][buffer_count]->SubmitSourceBuffer(&SoundBuffer);
            //サウンドスタート
            m_pSESourceVoice[id][buffer_count]->Start();
        }
       
    }
    
}

unsigned char* CAudio::LoadWave(ChunkInfo* p_chunk_info, WAVEFORMATEX* p_wave, const wchar_t* name)
{
    unsigned char* wave_data;
    //waveファイルのオープン
    FILE* fp;
    _wfopen_s(&fp, name, L"rb");

    //ファイルサイズを取得
    unsigned Size = 0;
    fseek(fp, 0, SEEK_END);
    Size = ftell(fp);
    wave_data = new unsigned char[Size];

    //ファイルデータをメモリに移す
    fseek(fp, 0, SEEK_SET);
    fread(reinterpret_cast<char*>(wave_data), Size, 1, fp);
    fclose(fp);

    //RIFFファイル解析
 
    //RIFFデータの先頭アドレスとRIFFデータサイズを渡す
    ChunkInfo WaveChunk = FindChunk(wave_data, "fmt");
    unsigned char* p = WaveChunk.pData;

    //wave情報取得
    p_wave->wFormatTag      = GetWord(p);     p += sizeof(WORD);
    p_wave->nChannels       = GetWord(p);     p += sizeof(WORD);
    p_wave->nSamplesPerSec  = GetDword(p);    p += sizeof(DWORD);
    p_wave->nAvgBytesPerSec = GetDword(p);    p += sizeof(DWORD);
    p_wave->nBlockAlign     = GetWord(p);     p += sizeof(WORD);
    p_wave->wBitsPerSample  = GetWord(p);     p += sizeof(WORD);
    p_wave->cbSize          = GetWord(p);     p += sizeof(WORD);
    

    //波形データの先頭の先頭アドレスと波形データサイズ値を渡す
    ChunkInfo info = FindChunk(wave_data, "data");
    p_chunk_info->Size  = info.Size;
    p_chunk_info->pData = info.pData;

    return wave_data;
}

//Oggファイル読み込み
unsigned char* CAudio::LoadOgg(ChunkInfo* p_chunk_info, WAVEFORMATEX* p_wave, const char* name)
{
    //Oggファイルを開く
    OggVorbis_File ovf;
    if (ov_fopen(name, &ovf) != 0)
    {
        return nullptr;
    }

    //Oggファイルの音声フォーマット情報
    vorbis_info* oggInfo = ov_info(&ovf, -1);

    //リニアPCM格納
    vector<char>tmpBuffer(4096);//分割されて送られるデータを入れるバッファ
    vector<char>buffer;         //リニアPCMデータ格納用バッファ

    //Oggのリソースからメモリにデータを移す
    int bitstream = 0;
    int readSize = 0;//読み込みに成功したデータの大きさ
    int comSize = 0;//動的なbufferの現在の大きさ

    while (1)
    {
        //最大4096byteにデータが分割して送られる
        readSize = ov_read(&ovf, &tmpBuffer[0], 4096, 0, 2, 1, &bitstream);
        if (readSize == 0)
        {
            break;
        }

        //bufferに取得したデータを追加（追加分要素も増やす）
        buffer.resize(comSize + readSize);                   //要素追加
        memcpy(&buffer[comSize], &tmpBuffer[0], readSize);   //バッファにコピー

        //今のバッファの大きさを計算
        comSize += readSize;
    }

    //waveヘッダ部分
    p_wave->wFormatTag           = 1;
    p_wave->nChannels            = oggInfo->channels;
    p_wave->nSamplesPerSec       = oggInfo->rate;
    p_wave->nAvgBytesPerSec      = oggInfo->rate * 16 / 8 * oggInfo->channels;
    p_wave->nBlockAlign          = 16 / 8 * oggInfo->channels;
    p_wave->wBitsPerSample       = 16;
    p_wave->cbSize               = 0;


    //波形データ部分
    unsigned char* wave_data = new unsigned char[comSize];
    memcpy(wave_data, &buffer[0], comSize);
    p_chunk_info->Size = comSize;
    p_chunk_info->pData = wave_data;

    ov_clear(&ovf);//Oggクローズ
    vector<char>().swap(tmpBuffer);
    vector<char>().swap(buffer);
    
    return wave_data;
}

//SE用の音楽読み込み
void CAudio::LoadSEMusic(int id, const wchar_t* name)
{
    //Waveファイル取得
    WAVEFORMATEX WaveformatEx;
    m_pSEResourceData[id] = LoadWave(&m_SEDataChunk[id], &WaveformatEx, name);

    
    for (int i = 0; i < 16; i++)
    {
        //再生のためのインターフェース生成
        //サブミクスボイスをセット
        XAUDIO2_SEND_DESCRIPTOR data;
        data.Flags = 0;
        data.pOutputVoice = m_pSESFXSubmixVoice[id];
        XAUDIO2_VOICE_SENDS SFXSendList;
        memset(&SFXSendList, 0x00, sizeof(XAUDIO2_VOICE_SENDS));
        SFXSendList.SendCount = 1;
        SFXSendList.pSends = &data;
        //ソースボイス作成
        m_pXAudio2->CreateSourceVoice(&m_pSESourceVoice[id][i], &WaveformatEx, 0U, 2.0F, 0, &SFXSendList, 0);
    }
   
}

//ループ用の音楽読み込み
void CAudio::LoadBackMusic(const wchar_t* name)
{
    //Waveファイル取得
    WAVEFORMATEX WaveformatEx;
    m_pResourceData = LoadWave(&m_DataChunk, &WaveformatEx, name);

    //再生のためのインターフェース生成
    XAUDIO2_SEND_DESCRIPTOR data;
    data.Flags = 0;
    data.pOutputVoice = m_pSFXSubmixVoice;
    XAUDIO2_VOICE_SENDS SFXSendList;
    memset(&SFXSendList, 0x00, sizeof(XAUDIO2_VOICE_SENDS));
    SFXSendList.SendCount = 1;
    SFXSendList.pSends = &data;
    //ソースボイス作成
    m_pXAudio2->CreateSourceVoice(&m_pSourceVoice, &WaveformatEx, 0U, 2.0F, 0, &SFXSendList, 0);

}

//SE用の音楽読み込みOgg用
void CAudio::LoadSEMusic(int id, char* name)//間違いの可能性（指南書15-13)
{
    //Waveファイル取得
    WAVEFORMATEX WaveformatEx;
    m_pSEResourceData[id] = LoadOgg(&m_SEDataChunk[id], &WaveformatEx, name);

    //再生のためのインターフェース生成
    for (int i = 0; i < 16; i++)
    {
        m_pXAudio2->CreateSourceVoice(&m_pSESourceVoice[id][i], &WaveformatEx);
    }


}

//ループ用の音楽読み込みOgg用
void CAudio::LoadBackMusic(const char* name)//間違いの可能性（指南書15-13)
{
    //Waveファイル取得
    WAVEFORMATEX WaveformatEx;
    m_pResourceData = LoadOgg(&m_DataChunk, &WaveformatEx, name);

    //再生のためのインターフェース生成
    m_pXAudio2->CreateSourceVoice(&m_pSourceVoice, &WaveformatEx);

}



//ループ用の音楽停止
void CAudio::StopLoopMusic()
{
    m_pSourceVoice->Stop();                //音楽停止
    m_pSourceVoice->FlushSourceBuffers();  //サウンドボイスに保留中バッファ破棄
}

//ループ用の音楽再生
void CAudio::StartLoopMusic()
{
    //サウンドバッファにLoadBackMusicで読み込んだ波形情報をLoop用で再生
    XAUDIO2_BUFFER SoundBuffer = { 0 };
    SoundBuffer.AudioBytes     = m_DataChunk.Size;
    SoundBuffer.pAudioData     = reinterpret_cast<BYTE*>(m_DataChunk.pData);
    SoundBuffer.LoopCount      = XAUDIO2_LOOP_INFINITE;//ループ設定
    SoundBuffer.Flags          = XAUDIO2_END_OF_STREAM;
    //サウンドバッファをセット
    m_pSourceVoice->SubmitSourceBuffer(&SoundBuffer);
    //サウンドスタート
    m_pSourceVoice->Start();
}


void CAudio::InitAudio()
{
    unsigned XAudio2CreateFlags = 0;

    //XAudio2インターフェース作成
    XAudio2Create(&m_pXAudio2, XAudio2CreateFlags);

    //マスターボイス作成
    m_pXAudio2->CreateMasteringVoice(&m_pMasteringVoice);

    //ミックスボイス作成
    m_pXAudio2->CreateSubmixVoice(&m_pSFXSubmixVoice, 1, 44100, 0, 0, 0, 0);

    for (int i = 0; i < 32; i++)
    {
        m_pXAudio2->CreateSubmixVoice(&m_pSESFXSubmixVoice[i], 1, 44100, 0, 0, 0, 0);
    }

    //サウンドボイス初期化
    m_pSourceVoice = nullptr;

    //SE用のサウンドボイス
    for (int i = 0; i < 32; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            m_pSESourceVoice[i][j] = nullptr;
        }
      
    }
}

void CAudio::DeleteAudio()
{
    //SE用のサウンドボイスとリソースの破棄
    for (int i = 0; i < 32; i++)
    {
        //サウンドボイスの破棄
        for (int j = 0; j < 16; j++)
        {
            if (m_pSESourceVoice[i][j] != nullptr)
            {
                m_pSESourceVoice[i][j]->Stop();
                m_pSESourceVoice[i][j]->FlushSourceBuffers();
                m_pSESourceVoice[i][j]->DestroyVoice();
            }
        }
        

        if (m_SEDataChunk[i].pData != nullptr)
        {
            delete[] m_pSEResourceData[i];
        }

    }

    //LOOP用のサウンドボイスとリソースの破棄
    if (m_pSourceVoice != nullptr)
    {
        m_pSourceVoice->Stop();
        m_pSourceVoice->FlushSourceBuffers();
        m_pSourceVoice->DestroyVoice();
    }

    //サウンドデータ破棄
    if (m_DataChunk.pData != nullptr)
    {
        delete[] m_pResourceData;
    }

    //SE用のサブミックスサウンド破棄
    for (int i = 0; i < 32; i++)
    {
        m_pSESFXSubmixVoice[i]->DestroyVoice();
    }

    //ミックスサウンド破棄
    m_pSFXSubmixVoice->DestroyVoice();

    //マスターボイス破棄
    m_pMasteringVoice->DestroyVoice();

    //XAudio2インターフェース破棄
    m_pXAudio2->Release();
}

//Word型変換関数
//引数1　const  unsinged char* pData:Word型に変換する配列
//戻り値　Wordに変換した値
//指定した配列要素2つ分(2byte)をWORD型の値として出力
WORD CAudio::GetWord(const unsigned char* pData)
{
    WORD value = pData[0] | pData[1] << 8;
    return value;
}

//DWord型変換変数
//引数1　const unsigned char* pData:DWord
//戻り値　DWord型に変換した値
//指定した配列要素4つ分(4byte)をDWORDの値として出力
DWORD CAudio::GetDword(const unsigned char* pData)
{
    DWORD value = pData[0] | pData[1] << 8 | pData[2] << 16 | pData[3] << 24;
    return value;
}

//指定したチャンク値を見つける関数
//引数1 const unsigned char* pData:Waveファイルデータを持った配列
//引数2 const char* pChunkName :探すチャンクネーム
//戻り値:ChunkInfo:チャンク以外にあるファイルサイズ値とデータ部の先頭アドレス返す
//指定したチャンクを配列から探し出して、チャンク以下に設定されてるデータサイズと
//データ部の先頭アドレスを返す。また、必ずチャンクは見つかるとして簡略化してる
ChunkInfo CAudio::FindChunk(const unsigned char* pData, const char* pChunkName)
{
    const unsigned CHUNKNAME_LENGTH = strlen(pChunkName);
    while (true)
    {
        bool IsFind = true;
        for (unsigned i = 0; i < CHUNKNAME_LENGTH; ++i)
        {
            if (pData[i] != pChunkName[i])
            {
                IsFind = false;
                break;
            }
        }
        if (IsFind)
        {
            ChunkInfo info;
            info.Size = pData[4 + 0] | pData[4 + 1] << 8 | pData[4 + 2] << 16 | pData[4 + 3] << 24;
            info.pData = const_cast<unsigned char*>(pData + 8);
            return info;
        }
        pData++;
    }
    return ChunkInfo();
}

