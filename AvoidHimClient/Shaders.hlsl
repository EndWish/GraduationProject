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



struct VS_2D_IN {
    float2 position : POSITION;
};

struct VS_2D_OUT {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VS_2D_OUT Vertex2DShader(VS_2D_IN input) {
    VS_2D_OUT output;
    
    float2 startpos = float2(worldTransform._11, worldTransform._22);
    float2 startuv = float2(worldTransform._41, worldTransform._43);

    //startpos = float2(0,0);
    
    // 2DUI는 항상 z = 0 (맨 앞)에 그려지고, depth값도 쓴다.
    output.position = float4(input.position.x + startpos.x, input.position.y + startpos.y, 0, 1);
    
    float x, y;
    if (input.position.x == 0)
        output.uv.x = startuv.x;
    else
        output.uv.x = startuv.x + 1;
    
    if (input.position.y == 0)
        output.uv.y = startuv.y;
    else
        output.uv.y = startuv.y - 1;

    return output;
}

float4 Pixel2DShader(VS_2D_OUT input) : SV_TARGET {
    float2 startuv = float2(worldTransform._41, worldTransform._43);
    float2 sizeuv = float2(worldTransform._42, worldTransform._44);
    if (sizeuv.x + startuv.x < input.uv.x) 
        discard;
    if (sizeuv.y - startuv.y < input.uv.y) 
        discard;
    float4 color = albedoMap.Sample(gssWrap, input.uv);
    if (color.a < 0.01f)
        discard;
    if (worldTransform._14 > 0.1f)
        color.rgb *= 0.3f;
    return color;
}
