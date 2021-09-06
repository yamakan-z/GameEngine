#include "Render3D.h"
#include "Math3D.h"

//ボーンにアニメーション情報を入れる
void CRender3D::SkinAnimation(C_SKIN_MODEL* modle, CMODEL_BONE_BUFFER* bones, int bone_id, float mat_ani[16])
{
	//アニメーション情報
	float ani[16];
	Math3D::IdentityMatrix(ani);

	//仮アニメーションを合成
	static float rr = 0.0f;
	rr += 0.01f;
	float f[3] = { 0.0f,1.0f,0.0f };
	Math3D::Rotate(ani, rr, f, ani);//回転行列を作成

	//bonesにアニメーションを合成
	float* bone  = bones->m_mat[bone_id];                             //アニメーションさせる行列
	float* bind_pos        = modle->m_bone[bone_id].m_bind_pos;       //ボーンのオフセット行列（元々のボーン向きや位置）
	float* inv_bind_pos    = modle->m_bone[bone_id].m_inv_bind_pos;   //ボーンの逆オフセット行列
	float* local_transform = modle->m_bone[bone_id].m_local_transform;//ボーンの原点移動用行列
	float  inverse[16];                                               //ボーンの逆原点移動用行列
	Math3D::Inverse(local_transform, inverse);

	Math3D::Multiply(inv_bind_pos, bone, bone);     //行列に逆オフセット行列合成
	Math3D::Multiply(local_transform, bone, bone);  //骨のローカル座標変換行列を合成
	Math3D::Multiply(ani, bone, bone);              //行列に回転等のアニメーション行列を合成
	Math3D::Multiply(inverse, bone, bone);          //行列にボーンの逆原点移動用行列合成
	Math3D::Multiply(bind_pos, bone, bone);         //行列に元々のボーン向きや位置を持つオフセット行列合成
}