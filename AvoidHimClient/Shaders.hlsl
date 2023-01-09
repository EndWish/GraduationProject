cbuffer cbCameraInfo : register(b1) {
	matrix view : packoffset(c0);
	matrix projection : packoffset(c4);
    float3 cameraPosition : packoffset(c36);
    float3 playerPosition : packoffset(c37);
};

cbuffer cbGameObjectInfo : register(b2) {
	matrix worldTransform : packoffset(c0);
};

#include "Light.hlsl"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define WATER_HEIGHT 150.0f

#define MATERIAL_ALBEDO_MAP		0x01

#define CWIDTH 1920
#define CHEIGHT 1080


// 텍스처
Texture2D albedoMap : register(t5);

// 샘플러
SamplerState gssWrap : register(s0);
SamplerState gssClamp : register(s1);

struct VS_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

VS_OUTPUT DefaultVertexShader(VS_INPUT input)
{
    VS_OUTPUT output;

    output.normal = mul(input.normal, (float3x3) worldTransform);
    output.normal = normalize(output.normal);

	// 조명 계산을 위해 월드좌표내에서의 포지션값을 계산해 따로 저장
    output.positionW = (float3) mul(float4(input.position, 1.0f), worldTransform);

    output.position = mul(mul(float4(output.positionW, 1.0f), view), projection);
    output.uv = input.uv;
    return output;
}

[earlydepthstencil]
float4 DefaultPixelShader(VS_OUTPUT input) : SV_TARGET {
    float4 cColor;
    if (drawMask & MATERIAL_ALBEDO_MAP) {
        cColor = albedoMap.Sample(gssWrap, input.uv);
    }
    else {
        cColor = diffuse;
    }
   float4 color = CalculateLight(cColor, input.positionW, input.normal);

    return color;
}
