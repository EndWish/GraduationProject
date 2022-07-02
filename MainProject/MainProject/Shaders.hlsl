//���� ��ü�� ������ ���� ��� ���۸� �����Ѵ�. 
cbuffer cbGameObjectInfo : register(b0) {
	matrix g_mtxWorld : packoffset(c0);
};

//ī�޶��� ������ ���� ��� ���۸� �����Ѵ�. 
cbuffer cbCameraInfo : register(b1) {
	matrix g_viewTransform : packoffset(c0);
	matrix g_projectionTransform : packoffset(c4);
};

//���� ���̴��� �����Ѵ�.
void VSPrototype(float3 input : POSITION, out float4 output : POSITION) 
{
	//������ ��ȯ(���� ��ȯ, ī�޶� ��ȯ, ���� ��ȯ)�Ѵ�.
	output = mul(mul(mul(float4(input, 1.0f), g_mtxWorld), g_viewTransform), g_projectionTransform);
}

//�ȼ� ���̴��� �����Ѵ�.
float4 PSPrototype(float4 input : POSITION) : SV_TARGET
{
	//�ԷµǴ� �ȼ��� ������ �״�� ���-���� �ܰ�(���� Ÿ��)�� ����Ѵ�. 
	return input;
}