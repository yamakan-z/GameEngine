#include"Hero.h"

CHero::CHero()
{
	//�����_���ɏ����l�����߂�
	m_x = rand() % 300 + 100;
	m_y = rand() % 300 + 100;
	//�����l
	m_vx = 1.0f;
	m_vy = 1.0f;

	//HERO�I�u�W�F�N�g�̊e�����蔻��̑������o���o���ɂ���
	static int count = 0;
	count++;

	//�q�b�g�{�b�N�X�쐬�i�j
	m_p_hit_box = Collision::HitBoxInsert(this);
	//�쐬�����q�b�g�{�b�N�X�̒l������
	m_p_hit_box->SetPos(m_x, m_y);
	m_p_hit_box->SetWH(256.0f, 256.0f);
	m_p_hit_box->SetElement(count);         //������count�ɂ���
	m_p_hit_box->SetInvisible(false);   //���G���[�h����
}

CHero::~CHero()
{
	

}

void CHero::Action()
{
	//�폜���s
	if (Input::KeyPush('Z'))
	{
		is_delete = true;  //�I�u�W�F�N�g�̍폜
		m_p_hit_box->SetDelete(true);
	}


	//�̈�ɏo�Ȃ��悤�ɔ��˂�����
	if (m_x < 0.0f)m_vx = +1.0f;
	if (m_x > 800.0f - 256.0f)m_vx = -1.0f;
	if (m_y < 0.0f)m_vy = +1.0f;
	if (m_y > 600.0f - 256.0f)m_vy = -1.0f;

	//�ړ������Ɉʒu*���x��������F
	m_x += m_vx * 5.0f;
	m_y += m_vy * 5.0f;
}

void CHero::Draw()
{
	//�`��
	Draw::Draw2D(0, m_x, m_y);
}