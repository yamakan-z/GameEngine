#include"Hero.h"

CHero::CHero()
{
	//�����_���ɏ����l�����߂�
	m_x = rand() % 300 + 100;
	m_y = rand() % 300 + 100;
	//�����l
	m_vx = 1.0f;
	m_vy = 1.0f;
}

CHero::~CHero()
{

}

void CHero::Action()
{
	//�폜���s
	if (Input::KeyPush('Z'))
	{
		is_delete = true;
	}


	//�̈�ɏo�Ȃ��悤�ɔ��˂�����
	if (m_x < 0.0f)m_vx = +1.0f;
	if (m_x > 800.0f - 256.0f)m_vx = -1.0f;
	if (m_y < 0.0f)m_vy = +1.0f;
	if (m_y > 600.0f - 256.0f)m_vy = -1.0f;

	//�ړ������Ɉʒu*���x��������
	m_x += m_vx * 5.0f;
	m_y += m_vy * 5.0f;
}

void CHero::Draw()
{
	//�`��
	Draw::Draw2D(0, m_x, m_y);
}