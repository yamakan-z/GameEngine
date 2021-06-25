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

//list内の当たり判定全チェック開始
void CCollision::CheckStart()
{
	//リスト内のls_deleteがtrueの要素を削除
	auto i = m_hit_box_list->begin();
	while (i != m_hit_box_list->end())
	{
		if (i->get()->ls_delete == true)
		{
			//イテレータiの要素を削除
			i = m_hit_box_list->erase(i);
		}
		else
		{
			i++;
		}
	}
	
	
	//複数との衝突情報制御用
	int hit_count = 0;

	//リスト内のヒットボックスで当たり判定を実装
	for (auto ip_a=m_hit_box_list->begin();ip_a!=m_hit_box_list->end();ip_a++)//Aヒットボックス
	{
		//衝突回数の初期化
		hit_count = 0;
		//Aの情報の衝突情報の初期化
		for (int i = 0; i < 10; i++)
		{
			(*ip_a)->m_hit[i] = nullptr;
		}

		//Aの当たり判定無視チェック
		//無敵
		if ((*ip_a)->m_ls_invisible)
			continue;
		for (auto ip_b = m_hit_box_list->begin(); ip_b != m_hit_box_list->end(); ip_b++)//Bヒットボックス
		{
			
			//当たり判定無視チェック
	        //AとBが同じヒットボックス
			if (ip_a == ip_b)
				continue;
			//無敵
			if ((*ip_b)->m_ls_invisible)
				continue;

			//AとBのヒットボックスが同属性
			if ((*ip_a)->m_element == (*ip_b)->m_element)
				continue;

	        //当たり判定を実装
			bool ls_hit = HitAB((*ip_a)->m_x,(*ip_a)->m_y, (*ip_a)->m_w, (*ip_a)->m_h,
			                    (*ip_b)->m_x,(*ip_b)->m_y, (*ip_b)->m_w, (*ip_b)->m_h
			                    );
			//衝突している場合
			if (ls_hit == true)
			{
				//aのヒットボックスに当たっているbのアドレスを与える
				int buffer_count = hit_count % 10;             //m_hit[]をキューバッファとして扱う
				(*ip_a)->m_hit[buffer_count] = ip_b->get();    //bのアドレス（フレッシュポインタを渡す）
				hit_count++;
			}
		}
		
	}
}

//個々の当たり判定
bool CCollision::HitAB(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh)
{
	//当たり判定
	if (ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by)
	{
		//衝突している
		return true;
	}
	//衝突していない
	return false;
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