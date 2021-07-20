#pragma once

//3D���w���Z�n
namespace Math3D
{
	void IdentityMatrix(float dest[16]);                           //�P�ʍs��쐬�֐�
	void Multiply(float mat1[16], float mat2[16], float dest[16]); //�s�񍇐��֐�
	void Transpose(float mat[16], float dest[16]);                 //�]���s��֐�
	void Inverse(float mat[16], float dest[16]);                   //�t�s��֐�
	void Transform(float pos[3], float mat[16], float dest[3]);    //�ʒu�ƍs��̍����֐�
	void Scale(float mat[16], float vec[3], float dest[16]);       //�g�k�s��֐�
	void Translation(float mat[16], float vec[3], float dest[16]);   //���s�ړ��s��֐�
	void Rotate(float mat[16], float angle, float axis[3], float dest[16]);             //��]�s��֐�
	void LookAt(float eye[3], float center[3], float up[3], float dest[16]);            //�r���[�s��֐�
	void Perspective(float fovy, float aspect, float near, float far, float dest[16]);  //�v���_�N�V�����s��֐�

};
