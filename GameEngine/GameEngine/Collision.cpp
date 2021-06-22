#include "Collision.h"

list<shared_ptr<HitBox>>* CCollision::m_hit_box_list;//リスト　HitBox用

//当たり判定を作成しリストに登録
HitBox* CCollision::HitBoxInsert(CObj* p)
{
	//ヒットボックス作成
	shared_ptr<HitBox>sp(new HitBox());

	//ヒットボックスに情報を入れる(フレンドなので直接アクセス出来る）
	sp->m_obj = p;             //この当たり判定の持つオブジェクトのアドレス(オブジェクトアドレスは引数pから引っ張って来る）
	sp->m_x = -999.0f;         //当たり判定のX位置
	sp->m_y = -999.0f;         //当たり判定のY位置
	sp->m_w = 64.0f;           //当たり判定の横幅
	sp->m_h = 64.0f;           //当たり判定の縦幅
	sp->m_ls_invisible = false;//当たり判定の無敵モードOFF
	sp->m_element = 0;         //当たり判定の属性

	//リストに登録
	m_hit_box_list->push_back(sp);

	//spのリフレッシュポインタを返す
	return sp.get();
}

//初期化
void CCollision::InitHitBox()
{
	m_hit_box_list = new list<shared_ptr<HitBox>>;
	m_hit_box_list->clear();
}

//破棄
void CCollision::DeleteHitBox()
{
	m_hit_box_list->clear();
	delete m_hit_box_list;
}