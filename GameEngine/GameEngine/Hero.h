//�g�p����w�b�_
#include "Draw2DPolygon.h"
#include "Input.h"
#include "Audio.h"
#include "TaskSystem.h"
#include "Collision.h"

//��l���N���X
class CHero :public CObj
{
public:
	CHero();
	~CHero();
	void Action();
	void Draw();
private:
	float m_x;//�ʒu
	float m_y;
	float m_vx;//�ړ��x�N�g��
	float m_vy;

	HitBox* m_p_hit_box;//�����蔻��
};