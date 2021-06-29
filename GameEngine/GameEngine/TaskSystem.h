#pragma once

//�K�v�ȃw�b�_
#include<memory>
#include<list>

using namespace std;


//�I�u�W�F�N�g�N���X
class CObj
{
public:
	CObj() 
	{
		is_delete = false;
		m_priority = 0;
	}
	virtual ~CObj() {};
	virtual void Action() = 0;
	virtual void Draw() = 0;

	bool is_delete;//�폜�t���O
	unsigned long m_priority;//�`��D�揇��
};

//�^�X�N�V�X�e��
typedef class CTaskSystem
{
public:
	CTaskSystem(){}
	~CTaskSystem(){}

	static void InsertObj(CObj* obj);//�ǉ�
	static void ListAction();        //���X�g���̃A�N�V�������s
	static void ListDraw();          //���X�g���̃h���[���s

	static void InitTaskSystem();   //������
	static void DeleteTaskSystem(); //�j��

	static void SortPriority();//���X�g���̃I�u�W�F�N�g���v���C�I���e�B���Ƀ\�[�g

private:
	//���X�g�@CObj�����I�u�W�F�N�g�̗v�f������
	static list<shared_ptr<CObj>>* m_task_list;

}TaskSystem;