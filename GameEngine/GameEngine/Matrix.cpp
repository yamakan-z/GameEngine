#include <math.h>

//単位行列作成関数
void IdentityMatrix(float dest[16])
{
	dest[ 0] =  1; dest[ 1] =  0; dest[ 2] =  0; dest[ 3] =  0;
	dest[ 4] =  0; dest[ 5] =  1; dest[ 6] =  0; dest[ 7] =  0;
	dest[ 8] =  0; dest[ 9] =  0; dest[10] =  1; dest[11] =  0;
	dest[12] =  0; dest[13] =  0; dest[14] =  0; dest[15] =  1;
	return;
}

//行列合成関数
void Multiply(float mat1[16], float mat2[16],float dest[16])
{
	float a = mat1[ 0], b = mat1[ 1], c = mat1[ 2], d = mat1[ 3];
	float e = mat1[ 4], f = mat1[ 5], g = mat1[ 6], h = mat1[ 7];
	float i = mat1[ 8], j = mat1[ 9], k = mat1[10], l = mat1[11];
	float m = mat1[12], n = mat1[13], o = mat1[14], p = mat1[15];
	float A = mat2[ 0], B = mat2[ 1], C = mat2[ 2], D = mat2[ 3];
	float E = mat2[ 4], F = mat2[ 5], G = mat2[ 6], H = mat2[ 7];
	float I = mat2[ 8], J = mat2[ 9], K = mat2[10], L = mat2[11];
	float M = mat2[12], N = mat2[13], O = mat2[14], P = mat2[15];

	dest[ 0] = A * a + B * e + C * i + D * m;
	dest[ 1] = A * b + B * f + C * j + D * n;
	dest[ 2] = A * c + B * g + C * k + D * o;
	dest[ 3] = A * d + B * h + C * l + D * p;
	dest[ 4] = E * a + F * e + G * i + H * m;
	dest[ 5] = E * b + F * f + G * j + H * n;
	dest[ 6] = E * c + F * g + G * k + H * o;
	dest[ 7] = E * d + F * h + G * l + H * p;
	dest[ 8] = I * a + J * e + K * i + L * m;
	dest[ 9] = I * b + J * f + K * j + L * n;
	dest[10] = I * c + J * g + K * k + L * o;
	dest[11] = I * d + J * h + K * l + L * p;
	dest[12] = M * a + N * e + O * i + P * m;
	dest[13] = M * b + N * f + O * j + P * n;
	dest[14] = M * c + N * g + O * k + P * o;
	dest[15] = M * d + N * h + O * l + P * p;
	return;
}

//転換行列関数
void Transpose(float mat[16], float dest[16])
{
	dest[ 0] = mat[ 0]; dest[ 1] = mat[ 4];
	dest[ 2] = mat[ 8]; dest[ 3] = mat[12];
	dest[ 4] = mat[ 1]; dest[ 5] = mat[ 5];
	dest[ 6] = mat[ 9]; dest[ 7] = mat[13];
	dest[ 8] = mat[ 2]; dest[ 9] = mat[ 6];
	dest[10] = mat[10]; dest[11] = mat[14];
	dest[12] = mat[ 3]; dest[13] = mat[ 7];
	dest[14] = mat[11]; dest[15] = mat[15];
	return;
}

//逆行列関数
void Inverse(float mat[16], float dest[16])
{
	float a = mat[ 0], b = mat[ 1], c = mat[ 2], d = mat[ 3];
	float e = mat[ 4], f = mat[ 5], g = mat[ 6], h = mat[ 7];
	float i = mat[ 8], j = mat[ 9], k = mat[10], l = mat[11];
	float m = mat[12], n = mat[13], o = mat[14], p = mat[15];

	float q = a * f - b * e;
	float r = a * g - c * e;
	float s = a * h - d * e;
	float t = b * g - c * f;
	float u = b * h - d * f;
	float v = c * h - d * g;
	float w = i * n - j * m;
	float x = i * o - k * m;
	float y = i * p - l * m;
	float z = j * o - k * n;
	float A = j * p - l * n;
	float B = k * p - l * o;
	float ivd = 1 / (q * B - r * A + s * z + t * y - u * x + v * w);

	dest[0]  = ( f * B - g * A + h * z) * ivd;
	dest[1]  = (-b * B + c * A - d * z) * ivd;
	dest[2]  = ( n * v - o * u + p * t) * ivd;
	dest[3]  = (-j * v + k * u - l * t) * ivd;
	dest[4]  = (-e * B + g * y - h * x) * ivd;
	dest[5]  = ( a * B - c * y + d * x) * ivd;
	dest[6]  = (-m * v + o * s - p * r) * ivd;
	dest[7]  = ( i * v - k * s + l * r) * ivd;
	dest[8]  = ( e * A - f * y + h * w) * ivd;
	dest[9]  = (-a * A + b * y - d * w) * ivd;
	dest[10] = ( m * u - n * s + p * q) * ivd;
	dest[11] = (-i * u + j * s - l * q) * ivd;
	dest[12] = (-e * z + f * x - g * w) * ivd;
	dest[13] = ( a * z - b * x + c * w) * ivd;
	dest[14] = (-m * t + n * r - o * q) * ivd;
	dest[15] = ( i * t - j * r + k * q) * ivd;
	return;
}

//位置と行列の合成関数
void Transform(float pos[3], float mat[16], float dest[3])
{
	float a = mat[ 0], b = mat[ 1], c = mat[ 2], d = mat[ 3];
	float e = mat[ 4], f = mat[ 5], g = mat[ 6], h = mat[ 7];
	float i = mat[ 8], j = mat[ 9], k = mat[10], l = mat[11];
	float m = mat[12], n = mat[13], o = mat[14], p = mat[15];

	float A = pos[ 0], B = pos[ 1], C = pos[ 2], D = 1.0f;

	dest[0] = A * a + B * e + C * i + D * m;
	dest[1] = A * b + B * f + C * j + D * n;
	dest[2] = A * c + B * g + C * k + D * o;
}

//拡縮行列関数
void Scale(float mat[16], float vec[3], float dest[16])
{
	dest[ 0] = mat[ 0] * vec[0];
	dest[ 1] = mat[ 1] * vec[0];
	dest[ 2] = mat[ 2] * vec[0];
	dest[ 3] = mat[ 3] * vec[0];
	dest[ 4] = mat[ 4] * vec[1];
	dest[ 5] = mat[ 5] * vec[1];
	dest[ 6] = mat[ 6] * vec[1];
	dest[ 7] = mat[ 7] * vec[1];
	dest[ 8] = mat[ 8] * vec[2];
	dest[ 9] = mat[ 9] * vec[2];
	dest[10] = mat[10] * vec[2];
	dest[11] = mat[11] * vec[2];
	dest[12] = mat[12]; 
	dest[13] = mat[13];
	dest[14] = mat[14]; 
	dest[15] = mat[15];
}

//平行移動行列関数
void Translate(float mat[16], float vec[3], float dest[16])
{
	dest[ 0] = mat[ 0]; dest[ 1] = mat[ 1]; dest[ 2] = mat[ 2]; dest[ 3] = mat[ 3];
	dest[ 4] = mat[ 4]; dest[ 5] = mat[ 5]; dest[ 6] = mat[ 6]; dest[ 7] = mat[ 7];
	dest[ 8] = mat[ 8]; dest[ 9] = mat[ 9]; dest[10] = mat[10]; dest[11] = mat[11];
	dest[12] = mat[ 0] * vec[ 0] + mat[ 4] * vec[ 1] + mat[ 8] + vec[ 2] + mat[12];
	dest[13] = mat[ 1] * vec[ 0] + mat[ 5] * vec[ 1] + mat[ 9] + vec[ 2] + mat[13];
	dest[14] = mat[ 2] * vec[ 0] + mat[ 6] * vec[ 1] + mat[10] + vec[ 2] + mat[14];
	dest[15] = mat[ 3] * vec[ 0] + mat[ 7] * vec[ 1] + mat[11] + vec[ 2] + mat[15];
	return;
}

//回転行列関数
void Rotate(float mat[16], float angle, float axis[3], float dest[16])
{
	float sq = sqrt(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] + axis[2]);
	float a = axis[0], b = axis[1], c = axis[2];

	if (sq == 0.0f)
	{
		return;
	}

	if (sq != 1.0f)
	{
		sq = 1.0f / sq;
		a *= sq;
		b *= sq;
		c *= sq;
	}

	float d = sin(angle), e = cos(angle), f = 1 - e;
	float g = mat[ 0], h = mat[ 1], i = mat[ 2], j = mat[ 3];
	float k = mat[ 4], l = mat[ 5], m = mat[ 6], n = mat[ 7];
	float o = mat[ 8], p = mat[ 9], q = mat[10], r = mat[11];
	float s = a * a * f + e;
	float t = b * a * f + c * d;
	float u = c * a * f - b * d;
	float v = a * b * f - c * d;
	float w = b * b * f + e;
	float x = c * b * f + a * d;
	float y = a * c * f + b * d;
	float z = b * c * f - a * d;
	float A = c * c * f + e;

}