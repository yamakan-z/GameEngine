#pragma once

#include<stdio.h>
#include<Windows.h>
#include<XAudio2.h>

//RIFFファイルフォーマットを読み取る
class ChunkInfo
{
public:
	ChunkInfo() :Size(0), pData(nullptr) {}
	unsigned int   Size;   //チャンクデータ部のサイズ
	unsigned char* pData;  //チャンクデータ部の先頭ポインタ
};

typedef class CAudio
{
public:
	CAudio(){}
	~CAudio(){}

	
	static void InitAudio();//初期化
	static void DeleteAudio();//破棄

	static void LoopMusicVolume(float t);       //BackMusic用のミュージックのボリューム調整
	static void SEMusicVolume(int id, float t); //SE用ミュージックのボリューム調整
	static void MasterVolume(float t);//マスターボリューム調整

	//wave用
	static void LoadBackMusic(const wchar_t* name);//ループ用の音楽読み込み
	static void LoadSEMusic(int id, const wchar_t* name);//SE用の音楽読み込み
	//ogg用
	static void LoadBackMusic(const char* name);//ループ用の音楽読み込み
	static void LoadSEMusic(int id, char* name);//SE用の音楽読み込み

	static void StartLoopMusic();          //ループ用の音楽再生
	static void StopLoopMusic();           //ループ用の音楽停止

	static void StartMusic(int id);        //単発SE用の音楽再生

private:
	static unsigned char* LoadWave(ChunkInfo* p_chunk_info, WAVEFORMATEX* p_wave, const wchar_t* name);//Wave読み込み
	static unsigned char* LoadOgg(ChunkInfo* p_chunk_info, WAVEFORMATEX* p_wave, const char* name);//Ogg読み込み

	static WORD  GetWord(const unsigned char* pData);
	static DWORD GetDword(const unsigned char* pData);
	static ChunkInfo FindChunk(const unsigned char* pData, const char* pChunkName);

	static IXAudio2*               m_pXAudio2;             //XAudio2オブジェクト
	static IXAudio2MasteringVoice* m_pMasteringVoice;      //マスターボイス
	static ChunkInfo               m_DataChunk;            //サウンド情報
	static unsigned char*          m_pResourceData;        //サウンドファイル情報を持つポインタ
	static IXAudio2SourceVoice*    m_pSourceVoice;         //サウンドボイスインターフェース
	static IXAudio2SubmixVoice*    m_pSFXSubmixVoice;      //サブミクスインターフェース

	static ChunkInfo              m_SEDataChunk[32];           //SE用のサウンド情報
	static unsigned char*         m_pSEResourceData[32];       //SE用のサウンドファイル情報を持つポインタ
	static IXAudio2SourceVoice*   m_pSESourceVoice[32][16];    //SE用のサウンドボイスインターフェース
	static IXAudio2SubmixVoice*   m_pSESFXSubmixVoice[32];     //SE用のサブミクスインターフェース


}Audio;