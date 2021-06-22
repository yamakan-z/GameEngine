#pragma once
//�K�v�ȃw�b�_�[
#include<Windows.h>

//�K�v�ȃw�b�_
#include<memory>
#include<list>

#include "TaskSystem.h"
using namespace std;

//�Փ˃I�u�W�F�N�g�N���X
class HitBox
{
	//CCollision�̂݃v���C�x�[�g�����J����
	friend class CCollision;
public:
	HitBox() { Init(); }
	~HitBox(){}
	//������
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
	//�ʒu
	float m_x;
	float m_y;
	//��
	float m_w;
	float m_h;
	//���̏Փ˃I�u�W�F�N�g�����I�u�W�F�N�g�̃|�C���^
	CObj* m_obj;
	//�����i�����l���m�ł͓����蔻����s��Ȃ�
	int m_element;
	//���G�itrue�ł́A�����蔻����s��Ȃ��j
	bool m_ls_invisible;
	//�������������HitBox���
	//������10�I�u�W�F�N�g�܂ŏ����Ƃ�
	HitBox* m_hit[10];
};

//�Փ˔���N���X
typedef class CCollision
{
public:
	CCollision(){}
	~CCollision(){}

	static void InitHitBox();  //������
	static void DeleteHitBox();//�j��

	static HitBox* HitBoxInsert(CObj* p);//�����蔻����쐬�����X�g�ɓo�^

private:
	//���X�g�@HitBox�����I�u�W�F�N�g�̗v�f������
	static list<shared_ptr<HitBox>>* m_hit_box_list;

}Collision;