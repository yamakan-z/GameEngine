#pragma once

#include<stdio.h>
#include<Windows.h>
#include<XAudio2.h>

//RIFF�t�@�C���t�H�[�}�b�g��ǂݎ��
class ChunkInfo
{
public:
	ChunkInfo() :Size(0), pData(nullptr) {}
	unsigned int   Size;   //�`�����N�f�[�^���̃T�C�Y
	unsigned char* pData;  //�`�����N�f�[�^���̐擪�|�C���^
};

typedef class CAudio
{
public:
	CAudio(){}
	~CAudio(){}

	
	static void InitAudio();//������
	static void DeleteAudio();//�j��

	static void LoopMusicVolume(float t);       //BackMusic�p�̃~���[�W�b�N�̃{�����[������
	static void SEMusicVolume(int id, float t); //SE�p�~���[�W�b�N�̃{�����[������
	static void MasterVolume(float t);//�}�X�^�[�{�����[������

	//wave�p
	static void LoadBackMusic(const wchar_t* name);//���[�v�p�̉��y�ǂݍ���
	static void LoadSEMusic(int id, const wchar_t* name);//SE�p�̉��y�ǂݍ���
	//ogg�p
	static void LoadBackMusic(char* name);//���[�v�p�̉��y�ǂݍ���
	static void LoadSEMusic(int id, char* name);//SE�p�̉��y�ǂݍ���

	static void StartLoopMusic();          //���[�v�p�̉��y�Đ�
	static void StopLoopMusic();           //���[�v�p�̉��y��~

	static void StartMusic(int id);        //�P��SE�p�̉��y�Đ�

private:
	static unsigned char* LoadWave(ChunkInfo* p_chunk_info, WAVEFORMATEX* p_wave, const wchar_t* name);//Wave�ǂݍ���
	static unsigned char* LoadOgg(ChunkInfo* p_chunk_info, WAVEFORMATEX* p_wave, char* name);//Ogg�ǂݍ���

	static WORD  GetWord(const unsigned char* pData);
	static DWORD GetDword(const unsigned char* pData);
	static ChunkInfo FindChunk(const unsigned char* pData, const char* pChunkName);

	static IXAudio2*               m_pXAudio2;             //XAudio2�I�u�W�F�N�g
	static IXAudio2MasteringVoice* m_pMasteringVoice;      //�}�X�^�[�{�C�X
	static ChunkInfo               m_DataChunk;            //�T�E���h���
	static unsigned char*          m_pResourceData;        //�T�E���h�t�@�C���������|�C���^
	static IXAudio2SourceVoice*    m_pSourceVoice;         //�T�E���h�{�C�X�C���^�[�t�F�[�X
	static IXAudio2SubmixVoice*    m_pSFXSubmixVoice;      //�T�u�~�N�X�C���^�[�t�F�[�X

	static ChunkInfo              m_SEDataChunk[32];           //SE�p�̃T�E���h���
	static unsigned char*         m_pSEResourceData[32];       //SE�p�̃T�E���h�t�@�C���������|�C���^
	static IXAudio2SourceVoice*   m_pSESourceVoice[32][16];    //SE�p�̃T�E���h�{�C�X�C���^�[�t�F�[�X
	static IXAudio2SubmixVoice*   m_pSESFXSubmixVoice[32];     //SE�p�̃T�u�~�N�X�C���^�[�t�F�[�X


}Audio;