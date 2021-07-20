#pragma once

//3D数学演算系
namespace Math3D
{
	void IdentityMatrix(float dest[16]);                           //単位行列作成関数
	void Multiply(float mat1[16], float mat2[16], float dest[16]); //行列合成関数
	void Transpose(float mat[16], float dest[16]);                 //転換行列関数
	void Inverse(float mat[16], float dest[16]);                   //逆行列関数
	void Transform(float pos[3], float mat[16], float dest[3]);    //位置と行列の合成関数
	void Scale(float mat[16], float vec[3], float dest[16]);       //拡縮行列関数
	void Translation(float mat[16], float vec[3], float dest[16]);   //平行移動行列関数
	void Rotate(float mat[16], float angle, float axis[3], float dest[16]);             //回転行列関数
	void LookAt(float eye[3], float center[3], float up[3], float dest[16]);            //ビュー行列関数
	void Perspective(float fovy, float aspect, float near, float far, float dest[16]);  //プロダクション行列関数

};
