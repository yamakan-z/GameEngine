#include "Audio.h"

#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"libogg.lib")
#pragma comment(lib,"libvorbis_static.lib")
#pragma comment(lib,"libvorbisfile_static.lib")

#include<vector>
#include"vorbis/vorbisfile.h"
using namespace std;


IXAudio2*               CAudio::m_pXAudio2;                   //XAudio2�I�u�W�F�N�g
IXAudio2MasteringVoice* CAudio::m_pMasteringVoice;            //�}�X�^�[�{�C�X
ChunkInfo               CAudio::m_DataChunk;                  //�T�E���h���
unsigned char*          CAudio::m_pResourceData;              //�T�E���h�t�@�C���������|�C���^
IXAudio2SourceVoice*    CAudio::m_pSourceVoice;               //�T�E���h�{�C�X�C���^�[�t�F�[�X
IXAudio2SubmixVoice*    CAudio::m_pSFXSubmixVoice;            //�T�u�~�N�X�C���^�[�t�F�[�X
ChunkInfo               CAudio::m_SEDataChunk[32];            //SE�p�̃T�E���h���
unsigned char*          CAudio::m_pSEResourceData[32];        //SE�p�̃T�E���h�t�@�C���������|�C���^
IXAudio2SourceVoice*    CAudio::m_pSESourceVoice[32][16];     //SE�p�̃T�E���h�{�C�X�C���^�[�t�F�[�X
IXAudio2SubmixVoice*    CAudio::m_pSESFXSubmixVoice[32];      //SE�p�̃T�u�~�N�X�C���^�[�t�F�[�X

//BackMusic�p�̃~���[�W�b�N�̃{�����[������
void CAudio::LoopMusicVolume(float t)
{
    m_pSFXSubmixVoice->SetVolume(t, 0);
}

//SE�p�~���[�W�b�N�̃{�����[������
void CAudio::SEMusicVolume(int id, float t)
{
    m_pSESFXSubmixVoice[id]->SetVolume(t, 0);
}

//�}�X�^�[�{�����[������
void CAudio::MasterVolume(float t)
{
    m_pMasteringVoice->SetVolume(t, 0);
}

//�P��SE�p�̉��y�Đ�
void CAudio::StartMusic(int id)
{
    //�T�E���h�o�b�t�@��LoadBackMusic�œǂݍ��񂾔g�`����P���Đ��Őݒ�
    XAUDIO2_BUFFER SoundBuffer = { 0 };
    SoundBuffer.AudioBytes = m_SEDataChunk[id].Size;
    SoundBuffer.pAudioData = reinterpret_cast<BYTE*>(m_SEDataChunk[id].pData);
    SoundBuffer.LoopCount = 0;
    SoundBuffer.Flags = XAUDIO2_END_OF_STREAM;

    for (int i = 0; i < 32; i++)
    {
        //i���o�b�t�@�̍ő�v�f���������Ȃ��悤�ɂ���
        int buffer_count = i % 16;

        //���d�Đ��T�E���h�o�b�t�@�̋��T��
        XAUDIO2_VOICE_STATE stats;
        m_pSESourceVoice[id][buffer_count]->GetState(&stats);
        if (stats.BuffersQueued == 0)
        {
            //�T�E���h�o�b�t�@���Z�b�g
            m_pSESourceVoice[id][buffer_count]->SubmitSourceBuffer(&SoundBuffer);
            //�T�E���h�X�^�[�g
            m_pSESourceVoice[id][buffer_count]->Start();
        }
       
    }
    
}

unsigned char* CAudio::LoadWave(ChunkInfo* p_chunk_info, WAVEFORMATEX* p_wave, const wchar_t* name)
{
    unsigned char* wave_data;
    //wave�t�@�C���̃I�[�v��
    FILE* fp;
    _wfopen_s(&fp, name, L"rb");

    //�t�@�C���T�C�Y���擾
    unsigned Size = 0;
    fseek(fp, 0, SEEK_END);
    Size = ftell(fp);
    wave_data = new unsigned char[Size];

    //�t�@�C���f�[�^���������Ɉڂ�
    fseek(fp, 0, SEEK_SET);
    fread(reinterpret_cast<char*>(wave_data), Size, 1, fp);
    fclose(fp);

    //RIFF�t�@�C�����
 
    //RIFF�f�[�^�̐擪�A�h���X��RIFF�f�[�^�T�C�Y��n��
    ChunkInfo WaveChunk = FindChunk(wave_data, "fmt");
    unsigned char* p = WaveChunk.pData;

    //wave���擾
    p_wave->wFormatTag      = GetWord(p);     p += sizeof(WORD);
    p_wave->nChannels       = GetWord(p);     p += sizeof(WORD);
    p_wave->nSamplesPerSec  = GetDword(p);    p += sizeof(DWORD);
    p_wave->nAvgBytesPerSec = GetDword(p);    p += sizeof(DWORD);
    p_wave->nBlockAlign     = GetWord(p);     p += sizeof(WORD);
    p_wave->wBitsPerSample  = GetWord(p);     p += sizeof(WORD);
    p_wave->cbSize          = GetWord(p);     p += sizeof(WORD);
    

    //�g�`�f�[�^�̐擪�̐擪�A�h���X�Ɣg�`�f�[�^�T�C�Y�l��n��
    ChunkInfo info = FindChunk(wave_data, "data");
    p_chunk_info->Size  = info.Size;
    p_chunk_info->pData = info.pData;

    return wave_data;
}

//Ogg�t�@�C���ǂݍ���
unsigned char* CAudio::LoadOgg(ChunkInfo* p_chunk_info, WAVEFORMATEX* p_wave, const char* name)
{
    //Ogg�t�@�C�����J��
    OggVorbis_File ovf;
    if (ov_fopen(name, &ovf) != 0)
    {
        return nullptr;
    }

    //Ogg�t�@�C���̉����t�H�[�}�b�g���
    vorbis_info* oggInfo = ov_info(&ovf, -1);

    //���j�APCM�i�[
    vector<char>tmpBuffer(4096);//��������đ�����f�[�^������o�b�t�@
    vector<char>buffer;         //���j�APCM�f�[�^�i�[�p�o�b�t�@

    //Ogg�̃��\�[�X���烁�����Ƀf�[�^���ڂ�
    int bitstream = 0;
    int readSize = 0;//�ǂݍ��݂ɐ��������f�[�^�̑傫��
    int comSize = 0;//���I��buffer�̌��݂̑傫��

    while (1)
    {
        //�ő�4096byte�Ƀf�[�^���������đ�����
        readSize = ov_read(&ovf, &tmpBuffer[0], 4096, 0, 2, 1, &bitstream);
        if (readSize == 0)
        {
            break;
        }

        //buffer�Ɏ擾�����f�[�^��ǉ��i�ǉ����v�f�����₷�j
        buffer.resize(comSize + readSize);                   //�v�f�ǉ�
        memcpy(&buffer[comSize], &tmpBuffer[0], readSize);   //�o�b�t�@�ɃR�s�[

        //���̃o�b�t�@�̑傫�����v�Z
        comSize += readSize;
    }

    //wave�w�b�_����
    p_wave->wFormatTag           = 1;
    p_wave->nChannels            = oggInfo->channels;
    p_wave->nSamplesPerSec       = oggInfo->rate;
    p_wave->nAvgBytesPerSec      = oggInfo->rate * 16 / 8 * oggInfo->channels;
    p_wave->nBlockAlign          = 16 / 8 * oggInfo->channels;
    p_wave->wBitsPerSample       = 16;
    p_wave->cbSize               = 0;


    //�g�`�f�[�^����
    unsigned char* wave_data = new unsigned char[comSize];
    memcpy(wave_data, &buffer[0], comSize);
    p_chunk_info->Size = comSize;
    p_chunk_info->pData = wave_data;

    ov_clear(&ovf);//Ogg�N���[�Y
    vector<char>().swap(tmpBuffer);
    vector<char>().swap(buffer);
    
    return wave_data;
}

//SE�p�̉��y�ǂݍ���
void CAudio::LoadSEMusic(int id, const wchar_t* name)
{
    //Wave�t�@�C���擾
    WAVEFORMATEX WaveformatEx;
    m_pSEResourceData[id] = LoadWave(&m_SEDataChunk[id], &WaveformatEx, name);

    
    for (int i = 0; i < 16; i++)
    {
        //�Đ��̂��߂̃C���^�[�t�F�[�X����
        //�T�u�~�N�X�{�C�X���Z�b�g
        XAUDIO2_SEND_DESCRIPTOR data;
        data.Flags = 0;
        data.pOutputVoice = m_pSESFXSubmixVoice[id];
        XAUDIO2_VOICE_SENDS SFXSendList;
        memset(&SFXSendList, 0x00, sizeof(XAUDIO2_VOICE_SENDS));
        SFXSendList.SendCount = 1;
        SFXSendList.pSends = &data;
        //�\�[�X�{�C�X�쐬
        m_pXAudio2->CreateSourceVoice(&m_pSESourceVoice[id][i], &WaveformatEx, 0U, 2.0F, 0, &SFXSendList, 0);
    }
   
}

//���[�v�p�̉��y�ǂݍ���
void CAudio::LoadBackMusic(const wchar_t* name)
{
    //Wave�t�@�C���擾
    WAVEFORMATEX WaveformatEx;
    m_pResourceData = LoadWave(&m_DataChunk, &WaveformatEx, name);

    //�Đ��̂��߂̃C���^�[�t�F�[�X����
    XAUDIO2_SEND_DESCRIPTOR data;
    data.Flags = 0;
    data.pOutputVoice = m_pSFXSubmixVoice;
    XAUDIO2_VOICE_SENDS SFXSendList;
    memset(&SFXSendList, 0x00, sizeof(XAUDIO2_VOICE_SENDS));
    SFXSendList.SendCount = 1;
    SFXSendList.pSends = &data;
    //�\�[�X�{�C�X�쐬
    m_pXAudio2->CreateSourceVoice(&m_pSourceVoice, &WaveformatEx, 0U, 2.0F, 0, &SFXSendList, 0);

}

//SE�p�̉��y�ǂݍ���Ogg�p
void CAudio::LoadSEMusic(int id, char* name)//�ԈႢ�̉\���i�w�쏑15-13)
{
    //Wave�t�@�C���擾
    WAVEFORMATEX WaveformatEx;
    m_pSEResourceData[id] = LoadOgg(&m_SEDataChunk[id], &WaveformatEx, name);

    //�Đ��̂��߂̃C���^�[�t�F�[�X����
    for (int i = 0; i < 16; i++)
    {
        m_pXAudio2->CreateSourceVoice(&m_pSESourceVoice[id][i], &WaveformatEx);
    }


}

//���[�v�p�̉��y�ǂݍ���Ogg�p
void CAudio::LoadBackMusic(const char* name)//�ԈႢ�̉\���i�w�쏑15-13)
{
    //Wave�t�@�C���擾
    WAVEFORMATEX WaveformatEx;
    m_pResourceData = LoadOgg(&m_DataChunk, &WaveformatEx, name);

    //�Đ��̂��߂̃C���^�[�t�F�[�X����
    m_pXAudio2->CreateSourceVoice(&m_pSourceVoice, &WaveformatEx);

}



//���[�v�p�̉��y��~
void CAudio::StopLoopMusic()
{
    m_pSourceVoice->Stop();                //���y��~
    m_pSourceVoice->FlushSourceBuffers();  //�T�E���h�{�C�X�ɕۗ����o�b�t�@�j��
}

//���[�v�p�̉��y�Đ�
void CAudio::StartLoopMusic()
{
    //�T�E���h�o�b�t�@��LoadBackMusic�œǂݍ��񂾔g�`����Loop�p�ōĐ�
    XAUDIO2_BUFFER SoundBuffer = { 0 };
    SoundBuffer.AudioBytes     = m_DataChunk.Size;
    SoundBuffer.pAudioData     = reinterpret_cast<BYTE*>(m_DataChunk.pData);
    SoundBuffer.LoopCount      = XAUDIO2_LOOP_INFINITE;//���[�v�ݒ�
    SoundBuffer.Flags          = XAUDIO2_END_OF_STREAM;
    //�T�E���h�o�b�t�@���Z�b�g
    m_pSourceVoice->SubmitSourceBuffer(&SoundBuffer);
    //�T�E���h�X�^�[�g
    m_pSourceVoice->Start();
}


void CAudio::InitAudio()
{
    unsigned XAudio2CreateFlags = 0;

    //XAudio2�C���^�[�t�F�[�X�쐬
    XAudio2Create(&m_pXAudio2, XAudio2CreateFlags);

    //�}�X�^�[�{�C�X�쐬
    m_pXAudio2->CreateMasteringVoice(&m_pMasteringVoice);

    //�~�b�N�X�{�C�X�쐬
    m_pXAudio2->CreateSubmixVoice(&m_pSFXSubmixVoice, 1, 44100, 0, 0, 0, 0);

    for (int i = 0; i < 32; i++)
    {
        m_pXAudio2->CreateSubmixVoice(&m_pSESFXSubmixVoice[i], 1, 44100, 0, 0, 0, 0);
    }

    //�T�E���h�{�C�X������
    m_pSourceVoice = nullptr;

    //SE�p�̃T�E���h�{�C�X
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
    //SE�p�̃T�E���h�{�C�X�ƃ��\�[�X�̔j��
    for (int i = 0; i < 32; i++)
    {
        //�T�E���h�{�C�X�̔j��
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

    //LOOP�p�̃T�E���h�{�C�X�ƃ��\�[�X�̔j��
    if (m_pSourceVoice != nullptr)
    {
        m_pSourceVoice->Stop();
        m_pSourceVoice->FlushSourceBuffers();
        m_pSourceVoice->DestroyVoice();
    }

    //�T�E���h�f�[�^�j��
    if (m_DataChunk.pData != nullptr)
    {
        delete[] m_pResourceData;
    }

    //SE�p�̃T�u�~�b�N�X�T�E���h�j��
    for (int i = 0; i < 32; i++)
    {
        m_pSESFXSubmixVoice[i]->DestroyVoice();
    }

    //�~�b�N�X�T�E���h�j��
    m_pSFXSubmixVoice->DestroyVoice();

    //�}�X�^�[�{�C�X�j��
    m_pMasteringVoice->DestroyVoice();

    //XAudio2�C���^�[�t�F�[�X�j��
    m_pXAudio2->Release();
}

//Word�^�ϊ��֐�
//����1�@const  unsinged char* pData:Word�^�ɕϊ�����z��
//�߂�l�@Word�ɕϊ������l
//�w�肵���z��v�f2��(2byte)��WORD�^�̒l�Ƃ��ďo��
WORD CAudio::GetWord(const unsigned char* pData)
{
    WORD value = pData[0] | pData[1] << 8;
    return value;
}

//DWord�^�ϊ��ϐ�
//����1�@const unsigned char* pData:DWord
//�߂�l�@DWord�^�ɕϊ������l
//�w�肵���z��v�f4��(4byte)��DWORD�̒l�Ƃ��ďo��
DWORD CAudio::GetDword(const unsigned char* pData)
{
    DWORD value = pData[0] | pData[1] << 8 | pData[2] << 16 | pData[3] << 24;
    return value;
}

//�w�肵���`�����N�l��������֐�
//����1 const unsigned char* pData:Wave�t�@�C���f�[�^���������z��
//����2 const char* pChunkName :�T���`�����N�l�[��
//�߂�l:ChunkInfo:�`�����N�ȊO�ɂ���t�@�C���T�C�Y�l�ƃf�[�^���̐擪�A�h���X�Ԃ�
//�w�肵���`�����N��z�񂩂�T���o���āA�`�����N�ȉ��ɐݒ肳��Ă�f�[�^�T�C�Y��
//�f�[�^���̐擪�A�h���X��Ԃ��B�܂��A�K���`�����N�͌�����Ƃ��Ċȗ������Ă�
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

