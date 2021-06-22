#include "Collision.h"

list<shared_ptr<HitBox>>* CCollision::m_hit_box_list;//���X�g�@HitBox�p

//�����蔻����쐬�����X�g�ɓo�^
HitBox* CCollision::HitBoxInsert(CObj* p)
{
	//�q�b�g�{�b�N�X�쐬
	shared_ptr<HitBox>sp(new HitBox());

	//�q�b�g�{�b�N�X�ɏ�������(�t�����h�Ȃ̂Œ��ڃA�N�Z�X�o����j
	sp->m_obj = p;             //���̓����蔻��̎��I�u�W�F�N�g�̃A�h���X(�I�u�W�F�N�g�A�h���X�͈���p������������ė���j
	sp->m_x = -999.0f;         //�����蔻���X�ʒu
	sp->m_y = -999.0f;         //�����蔻���Y�ʒu
	sp->m_w = 64.0f;           //�����蔻��̉���
	sp->m_h = 64.0f;           //�����蔻��̏c��
	sp->m_ls_invisible = false;//�����蔻��̖��G���[�hOFF
	sp->m_element = 0;         //�����蔻��̑���

	//���X�g�ɓo�^
	m_hit_box_list->push_back(sp);

	//sp�̃��t���b�V���|�C���^��Ԃ�
	return sp.get();
}

//������
void CCollision::InitHitBox()
{
	m_hit_box_list = new list<shared_ptr<HitBox>>;
	m_hit_box_list->clear();
}

//�j��
void CCollision::DeleteHitBox()
{
	m_hit_box_list->clear();
	delete m_hit_box_list;
}