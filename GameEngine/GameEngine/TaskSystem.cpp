#include "TaskSystem.h"

//���X�g�@CObj�����I�u�W�F�N�g
list<shared_ptr<CObj>>* CTaskSystem::m_task_list;


void CTaskSystem::InitTaskSystem()
{
	//���X�g�쐬
	m_task_list = new list<shared_ptr<CObj>>();
}

void CTaskSystem::DeleteTaskSystem()
{
	//���X�g�j��
	delete m_task_list;
}

//�ǉ�
void CTaskSystem::InsertObj(CObj* obj)
{
	//�V�F�A�|�C���^�ɃI�u�W�F�N�^�̃A�h���X���i�[
	shared_ptr<CObj>sp;
	sp.reset(obj);

	//���X�g�̖��[�Ɋi�[�����I�u�W�F�N�g��ǉ�
	m_task_list->push_back(sp);
}

//���X�g���̃A�N�V�������s
void CTaskSystem::ListAction()
{
	//���X�g����is_delete��true�̗v�f���폜
	auto i = m_task_list->begin();
	while (i != m_task_list->end())
	{
		if (i->get()->is_delete == true)
		{
			//�C�e���[�^i�̗v�f���폜
			i = m_task_list->erase(i);
		}
		else
		{
			i++;
		}
	}


	//���X�g�̐�[���疖�[�܂�
	for (auto ip = m_task_list->begin(); ip != m_task_list->end(); ip++)
	{
		//���X�g���̃I�u�W�F�N�g�����A�N�V�������\�b�h�����s
		ip->get()->Action();
	}
}

//���X�g���̃h���[���s
void CTaskSystem::ListDraw()
{
	for (auto ip = m_task_list->begin(); ip != m_task_list->end(); ip++)
	{
		//���X�g���̃I�u�W�F�N�g�����h���[���\�b�h�����s
		ip->get()->Draw();
	}
}