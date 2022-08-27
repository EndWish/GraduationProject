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
	float3 position : POSITION;
	float3 normal : NORMAL;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normal : NORMAL;
};



VS_OUTPUT DefaultVertexShader(VS_INPUT input)
{
	VS_OUTPUT output;

	output.normal = mul(input.normal, (float3x3)worldTransform);
	output.normal = normalize(output.normal);

	// 조명 계산을 위해 월드좌표내에서의 포지션값을 계산해 따로 저장
	output.positionW = (float3)mul(float4(input.position, 1.0f), worldTransform);

	output.position = mul(mul(float4(output.positionW, 1.0f), view), projection);
	return output;
}

float4 DefaultPixelShader(VS_OUTPUT input) : SV_TARGET
{
	float4 color = CalculateLight(input.positionW, input.normal);
	return color;
}


struct VS_HITBOX_INPUT {
    float3 position : POSITION;
};

struct VS_HITBOX_OUTPUT {
    float4 position : SV_POSITION;
};


VS_HITBOX_OUTPUT HitboxVertexShader(VS_HITBOX_INPUT input) {
    VS_HITBOX_OUTPUT output;
    output.position = mul(mul(mul(float4(input.position, 1.0f), worldTransform), view), projection);
    return output;
}

float4 HitboxPixelShader(VS_HITBOX_OUTPUT input) : SV_TARGET {
    float4 color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    return color;
}