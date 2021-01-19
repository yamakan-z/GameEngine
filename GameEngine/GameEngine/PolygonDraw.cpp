//CPU����擾���钸�_���\����
struct vertexIn
{
	float4 pos : POSITION;
	float4 col : COLOR;
};

//VS����PS�ɑ�����
struct vertexOut
{
	float4 pos : SV_POSITION;
	//�ȉ�PS�Ŏg�p����
	float4 col : COLOR;
};

//�O���[�o��
cbuffer global
{
	float4 color;
};

//���_�V�F�[�_
//���� vertexIn   :CPU����󂯎�钸�_���
//�߂�l vertexOut:PS�ɑ�����
//���_�������W�ϊ������邪����͕ϊ������Ă��Ȃ��B
vertexOut vs(vertexIn IN)
{
	vertexOut OUT;

	OUT.pos = IN.pos;
	OUT.col = IN.col;

	return OUT;
}

//�s�N�Z���V�F�[�_
//�����@�@vertexOut:VS���瑗���Ă������
//�߂�l�@float4:Color�l
//�����̏�񌳂ɐF�����߂�B����͒��_�����F�ƃO���[�o��
//���玝���Ă����F����Z���Ă�
float4 ps(vertexOut IN) :SV_Target
{
	float4 col = IN.col * color;
return col;
}