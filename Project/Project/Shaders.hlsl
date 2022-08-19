cbuffer cbCameraInfo : register(b1) {
	matrix view : packoffset(c0);
	matrix projection : packoffset(c4);
	float3 cameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b2) {
	matrix worldTransform : packoffset(c0);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct VS_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
};

VS_OUTPUT DefaultVertexShader(VS_INPUT input)
{
	VS_OUTPUT output;
	//float4 output;
	
	output.normal = mul(input.normal, (float3x3)worldTransform);
	output.position = mul(mul(mul(float4(input.position, 1.0f), worldTransform), view), projection);
	//output.position = float4(input.modelPosition, 1.0f);
	output.normal = normalize(output.normal);
	output.color = float4(1, 1, 1, 1);
	return output;
}

float4 DefaultPixelShader(VS_OUTPUT input) : SV_TARGET
{
	return input.color;
}