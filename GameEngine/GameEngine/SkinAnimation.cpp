#include "Render3D.h"
#include "Math3D.h"

//�{�[���ɃA�j���[�V������������
void CRender3D::SkinAnimation(C_SKIN_MODEL* modle, CMODEL_BONE_BUFFER* bones, int bone_id, float mat_ani[16])
{
	//�A�j���[�V�������
	float ani[16];
	Math3D::IdentityMatrix(ani);

	//���A�j���[�V����������
	static float rr = 0.0f;
	rr += 0.01f;
	float f[3] = { 0.0f,1.0f,0.0f };
	Math3D::Rotate(ani, rr, f, ani);//��]�s����쐬

	//bones�ɃA�j���[�V����������
	float* bone  = bones->m_mat[bone_id];                             //�A�j���[�V����������s��
	float* bind_pos        = modle->m_bone[bone_id].m_bind_pos;       //�{�[���̃I�t�Z�b�g�s��i���X�̃{�[��������ʒu�j
	float* inv_bind_pos    = modle->m_bone[bone_id].m_inv_bind_pos;   //�{�[���̋t�I�t�Z�b�g�s��
	float* local_transform = modle->m_bone[bone_id].m_local_transform;//�{�[���̌��_�ړ��p�s��
	float  inverse[16];                                               //�{�[���̋t���_�ړ��p�s��
	Math3D::Inverse(local_transform, inverse);

	Math3D::Multiply(inv_bind_pos, bone, bone);     //�s��ɋt�I�t�Z�b�g�s�񍇐�
	Math3D::Multiply(local_transform, bone, bone);  //���̃��[�J�����W�ϊ��s�������
	Math3D::Multiply(ani, bone, bone);              //�s��ɉ�]���̃A�j���[�V�����s�������
	Math3D::Multiply(inverse, bone, bone);          //�s��Ƀ{�[���̋t���_�ړ��p�s�񍇐�
	Math3D::Multiply(bind_pos, bone, bone);         //�s��Ɍ��X�̃{�[��������ʒu�����I�t�Z�b�g�s�񍇐�
}