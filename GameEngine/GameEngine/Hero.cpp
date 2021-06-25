#include"Hero.h"

CHero::CHero()
{
	//ランダムに初期値を決める
	m_x = rand() % 300 + 100;
	m_y = rand() % 300 + 100;
	//初期値
	m_vx = 1.0f;
	m_vy = 1.0f;

	//HEROオブジェクトの各当たり判定の属性をバラバラにする
	static int count = 0;
	count++;

	//ヒットボックス作成（）
	m_p_hit_box = Collision::HitBoxInsert(this);
	//作成したヒットボックスの値を決定
	m_p_hit_box->SetPos(m_x, m_y);
	m_p_hit_box->SetWH(256.0f, 256.0f);
	m_p_hit_box->SetElement(count);         //属性はcountにする
	m_p_hit_box->SetInvisible(false);   //無敵モード無効
}

CHero::~CHero()
{
	

}

void CHero::Action()
{
	//削除実行
	if (Input::KeyPush('Z'))
	{
		is_delete = true;  //オブジェクトの削除
		m_p_hit_box->SetDelete(true);
	}


	//領域に出ないように反射させる
	if (m_x < 0.0f)m_vx = +1.0f;
	if (m_x > 800.0f - 256.0f)m_vx = -1.0f;
	if (m_y < 0.0f)m_vy = +1.0f;
	if (m_y > 600.0f - 256.0f)m_vy = -1.0f;

	//移動方向に位置*速度を加えるF
	m_x += m_vx * 5.0f;
	m_y += m_vy * 5.0f;
}

void CHero::Draw()
{
	//描画
	Draw::Draw2D(0, m_x, m_y);
}