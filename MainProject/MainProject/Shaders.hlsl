//���� ��ü�� ������ ���� ��� ���۸� �����Ѵ�. 
cbuffer cbGameObjectInfo : register(b0) {
	matrix gmtxWorld : packoffset(c0);
};

//ī�޶��� ������ ���� ��� ���۸� �����Ѵ�. 
cbuffer cbCameraInfo : register(b1) {
	matrix gmtxView : packoffset(c0);
	matrix gmtxProjection : packoffset(c4);
}

//���� ���̴��� �����Ѵ�.
void VSPrototype(float3 input : POSITION, out float4 output : POSITION) 
{
	//������ ��ȯ(���� ��ȯ, ī�޶� ��ȯ, ���� ��ȯ)�Ѵ�.
	output = mul(mul(mul(float4(input, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
}

//�ȼ� ���̴��� �����Ѵ�.
float4 PSPrototype(float4 input : POSITION) : SV_TARGET
{
	//�ԷµǴ� �ȼ��� ������ �״�� ���-���� �ܰ�(���� Ÿ��)�� ����Ѵ�. 
	return input;
}