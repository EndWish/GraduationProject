cbuffer cbCameraInfo : register(b1) {
	matrix view : packoffset(c0);
	matrix projection : packoffset(c4);
	float3 cameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b2) {
	matrix worldTransform : packoffset(c0);
};

#include "Light.hlsl"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct VS_INPUT
{
	float3 modelPosition : POSITION;
	float3 modelNormal : NORMAL;
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

	output.normal = mul(input.modelNormal, (float3x3)worldTransform);
	output.position = mul(mul(mul(float4(input.modelPosition, 1.0f), worldTransform), view), projection);
	output.normal = normalize(output.normal);
	
	output.color = float4(0, 1, 0, 1);
	return output;
}

float4 DefaultPixelShader(VS_OUTPUT output) : SV_TARGET
{
	return output.color;
}