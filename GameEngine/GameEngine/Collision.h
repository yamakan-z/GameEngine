#pragma once
//必要なヘッダー
#include<Windows.h>

//必要なヘッダ
#include<memory>
#include<list>

#include "TaskSystem.h"
using namespace std;

//衝突オブジェクトクラス
class HitBox
{
	//CCollisionのみプライベートを公開する
	friend class CCollision;
public:
	HitBox() { Init(); }
	~HitBox(){}
	//初期化
	void Init()
	{
		m_x = m_y = -999.0f;
		m_w = m_h = 0.0f;
		m_obj = nullptr;
		m_element = -1;
		for (int i = 0; i < 10; i++)
			m_hit[i] = nullptr;
	}

private:
	//位置
	float m_x;
	float m_y;
	//幅
	float m_w;
	float m_h;
	//この衝突オブジェクトを持つオブジェクトのポインタ
	CObj* m_obj;
	//属性（同じ値同士では当たり判定を行わない
	int m_element;
	//無敵（trueでは、当たり判定を行わない）
	bool m_ls_invisible;
	//当たった相手のHitBox情報
	//同時に10オブジェクトまで情報をとる
	HitBox* m_hit[10];
};

//衝突判定クラス
typedef class CCollision
{
public:
	CCollision(){}
	~CCollision(){}

	static void InitHitBox();  //初期化
	static void DeleteHitBox();//破棄

	static HitBox* HitBoxInsert(CObj* p);//当たり判定を作成しリストに登録

private:
	//リスト　HitBoxを持つオブジェクトの要素を持つ
	static list<shared_ptr<HitBox>>* m_hit_box_list;

}Collision;