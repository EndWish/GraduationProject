cbuffer cbCameraInfo : register(b1) {
	matrix view : packoffset(c0);
	matrix projection : packoffset(c4);
    float3 cameraPosition : packoffset(c36);
    float3 playerPosition : packoffset(c37);
};

cbuffer cbGameObjectInfo : register(b2) {
	matrix worldTransform : packoffset(c0);
};

#define MAX_BONE 100
cbuffer cbSkinnedOffsetTransforms : register(b7)
{
    matrix offsetTransform[MAX_BONE];
}
cbuffer cbSkinnedWorldTransforms : register(b8)
{
    matrix skinnedWorldTransforms[MAX_BONE];
}

#include "Light.hlsl"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define WATER_HEIGHT 150.0f

#define MATERIAL_ALBEDO_MAP		0x01
#define MATERIAL_NORMAL_MAP      0x02
#define CWIDTH 1920
#define CHEIGHT 1080

// 텍스처
Texture2D albedoMap : register(t5);
Texture2D normalMap : register(t6);

// 샘플러
SamplerState gssWrap : register(s0);
SamplerState gssClamp : register(s1);

struct VS_INPUT {
	float3 position : POSITION;
	float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VS_OUTPUT {
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

VS_OUTPUT DefaultVertexShader(VS_INPUT input) {
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
    float4 cColor = float4(0, 0, 0, 1);
    if (drawMask & MATERIAL_ALBEDO_MAP) {
        cColor = albedoMap.Sample(gssWrap, input.uv);
    }
    if (drawMask & MATERIAL_NORMAL_MAP)
    {
        // 노말 매핑 수행
    }
    else {
        cColor = diffuse;
    }
   float4 color = CalculateLight(cColor, input.positionW, input.normal);

    return color;
}


///////////////////////////////////////////////////////////////////////////////
/// 
struct VS_SKINNED_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    uint4 boneIndex : BONEINDEX;
    float4 boneWeight : BONEWEIGHT;
};

struct VS_SKINNED_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

VS_SKINNED_OUTPUT SkinnedVertexShader(VS_SKINNED_INPUT input)
{
 //   VS_OUTPUT output;

 //   output.normal = mul(input.normal, (float3x3) worldTransform);
 //   output.normal = normalize(output.normal);

	//// 조명 계산을 위해 월드좌표내에서의 포지션값을 계산해 따로 저장
 //   output.positionW = (float3) mul(float4(input.position, 1.0f), worldTransform);

 //   output.position = mul(mul(float4(output.positionW, 1.0f), view), projection);
 //   output.uv = input.uv;
 //   return output;

    VS_OUTPUT output;
    output.positionW = float3(0, 0, 0);
    output.normal = float3(0, 0, 0);
    
    matrix mtxVertexToBoneWorld;
    for (int i = 0; i < 4; ++i)
    {
        if (input.boneWeight[i] > 0.0001f)
        {
            mtxVertexToBoneWorld = mul(offsetTransform[input.boneIndex[i]], skinnedWorldTransforms[input.boneIndex[i]]);
            output.positionW += input.boneWeight[i] * mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
            output.normal += input.boneWeight[i] * mul(input.normal, (float3x3) mtxVertexToBoneWorld);
        }
    }
    output.normal = normalize(output.normal);
    output.position = mul(mul(float4(output.positionW, 1.0f), view), projection);
    output.uv = input.uv;

    return output;
}

[earlydepthstencil]
float4 SkinnedPixelShader(VS_SKINNED_OUTPUT input) : SV_TARGET
{
    float4 cColor = float4(0, 0, 0, 1);
    if (drawMask & MATERIAL_ALBEDO_MAP)
    {
        cColor = albedoMap.Sample(gssWrap, input.uv);
    }
    if (drawMask & MATERIAL_NORMAL_MAP)
    {
        // 노말 매핑 수행
    }
    else
    {
        cColor = diffuse;
    }
    float4 color = CalculateLight(cColor, input.positionW, input.normal);

    return color;
}


///////////////////////////////////////////////////////////////////////////////
///

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
    // 이미지를 그릴 기본 사각형 좌표에 이미지의 좌표값을 쉐이더단계에서 더한다. 
    output.position = float4(input.position.x + startpos.x, input.position.y + startpos.y, 0, 1);
    
    float x, y;
    
    // 
    if (input.position.x == 0)
        output.uv.x = startuv.x;
    else  
        output.uv.x = startuv.x + 1;
    
    // 윗부분을 1, 아래를 0으로 
    if (input.position.y == 0)
        output.uv.y = startuv.y + 1;
    else
        output.uv.y = startuv.y;

    return output;
}

float4 Pixel2DShader(VS_2D_OUT input) : SV_TARGET {
    
    float2 uv = input.uv;
    float2 startuv = float2(worldTransform._41, worldTransform._43);
    float2 sizeuv = float2(worldTransform._42, worldTransform._44);
    if (worldTransform._14 > 0.1f) {
        uv.x -= 0.05f;
        uv.y -= 0.05f;     
    }
    if (sizeuv.x + startuv.x < input.uv.x) 
        discard;
    if (sizeuv.y - startuv.y < input.uv.y) 
        discard;
    float4 color = albedoMap.Sample(gssClamp, uv);
    if (color.a < 0.01f)
        discard;
    if (worldTransform._14 > 0.1f)
        color.rgb *= 0.3f;
    return color;
}


struct VS_BOUNDING_INPUT {
    float3 position : POSITION;
};

struct VS_BOUNDING_OUTPUT {
    float4 position : SV_POSITION;
};


VS_BOUNDING_OUTPUT BoundingVertexShader(VS_BOUNDING_INPUT input) {
    // 현재 프러스텀은 정점에 월드변환이 적용되어 오기 때문에 임시로 world를 뺀 상태이다.
    VS_BOUNDING_OUTPUT output;
    output.position = mul(mul(float4(input.position, 1.0f), view), projection);
    return output;
}

float4 BoundingPixelShader(VS_BOUNDING_OUTPUT input) : SV_TARGET {
    float4 color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    return color;
}

/////////////////////////////////////////

struct VS_INSTANCING_INPUT {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float4x4 worldMatrix : WORLDMAT;
};


VS_OUTPUT InstanceVertexShader(VS_INSTANCING_INPUT input) {
    VS_OUTPUT output;
    
    output.normal = mul(input.normal, (float3x3) input.worldMatrix);
    output.normal = normalize(output.normal);

	// 조명 계산을 위해 월드좌표내에서의 포지션값을 계산해 따로 저장
    output.positionW = (float3) mul(float4(input.position, 1.0f), input.worldMatrix);

    output.position = mul(mul(float4(output.positionW, 1.0f), view), projection);


    output.uv = input.uv;
    return output;
}


[earlydepthstencil]
float4 InstancePixelShader(VS_OUTPUT input) : SV_TARGET
{
    float4 cColor = diffuse;
    if (drawMask & MATERIAL_ALBEDO_MAP)
    {
        cColor = albedoMap.Sample(gssWrap, input.uv);
    }
    if (drawMask & MATERIAL_NORMAL_MAP)
    {
        // 노말 매핑 수행
        
    }
    float4 color = CalculateLight(cColor, input.positionW, input.normal);
    //color = cColor;

    return color;
}
