// 루드 시그니처는 64개의 32-비트 배열로 구성됨.
// 많이 사용하는 매개변수를 앞쪽에 배치하는 것이 좋음.

#define CWIDTH 1280
#define CHEIGHT 720

// 출력하지 않는 렌더타겟이 있을 수 있다.
#pragma warning( disable :  3578 )



static const int3 d[8] =
{
    int3(0, -1, 0), int3(-1, 0, 0), int3(1, 0, 0), int3(0, 1, 0),
    int3(1, -1, 0), int3(-1, 1, 0), int3(1, 1, 0), int3(-1, -1, 0)
};

cbuffer cbCameraInfo : register(b1) {
	matrix view : packoffset(c0);
	matrix projection : packoffset(c4);
    float3 cameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b2) {
	matrix worldTransform : packoffset(c0);
};

cbuffer cbIntVariable : register(b5) {
    int intValue : packoffset(c0);
};

cbuffer cbFrameworkInfo : register(b11)
{
    float currentTime : packoffset(c0.x);
    float elapsedTime : packoffset(c0.y);
};
cbuffer cbFloatVariable : register(b6) {
    float floatValue : packoffset(c0);
};


cbuffer cbIntVariable : register(b10) {
    int lightIndex : packoffset(c0);
};



struct EFFECT_INDEX
{
    uint index;
    uint row;
    uint col;
};

cbuffer cbEffectIndexInfo : register(b9)
{
    EFFECT_INDEX indexInfo : packoffset(c0);
}

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


float3 RandomDirection(float seedOffset);
float RandomFloat(float2 co);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define WATER_HEIGHT 150.0f

#define MATERIAL_ALBEDO_MAP		0x01
#define MATERIAL_NORMAL_MAP      0x02
#define MATERIAL_EMISSIVE_MAP      0x04


// 텍스처
Texture2D albedoMap : register(t5);
Texture2D normalMap : register(t6);
Texture2D emissiveMap : register(t13);

Texture2D PostBuffer : register(t14);

struct G_BUFFER_OUTPUT {
    float4 color : SV_TARGET0;   // 조명을 처리하기 전의 픽셀의 색상
    float4 normal : SV_TARGET1;
    float4 position : SV_TARGET2;
    float4 emissive : SV_TARGET3;
    float4 slideUVVec : SV_TARGET4;
    float depth : SV_TARGET5;
};


struct SHADOW_MAP_VS_OUTPUT {
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
};


struct VS_INPUT {
	float3 position : POSITION;
	float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 biTangent : BITANGENT;
    float2 uv : TEXCOORD;
};

struct VS_OUTPUT {
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 biTangent : BITANGENT;
    float2 uv : TEXCOORD;
};


bool isBorder(float2 uv)
{
    int3 coord;
    float depth;
    int3 baseCoord = int3(uv.x * CWIDTH, uv.y * CHEIGHT, 0);
    float baseDepth = 0;
    baseDepth = depthTexture.Load(baseCoord);
    if (baseDepth != -1)
        return false;
    else
    {
        for (int i = 0; i < 8; ++i)
        {
            coord = baseCoord + d[i];
            depth = depthTexture.Load(coord);
            if (abs(depth - baseDepth) > 0.2f)
                return true;
        }
        return false;
    }
}

VS_OUTPUT DefaultVertexShader(VS_INPUT input){
    VS_OUTPUT output;

    output.normal = mul(input.normal, (float3x3) worldTransform);
    output.tangent = mul(input.tangent, (float3x3) worldTransform);
    output.biTangent = mul(input.biTangent, (float3x3) worldTransform);

	// 조명 계산을 위해 월드좌표내에서의 포지션값을 계산해 따로 저장
    output.positionW = (float3) mul(float4(input.position, 1.0f), worldTransform);

    output.position = mul(mul(float4(output.positionW, 1.0f), view), projection);
    output.uv = input.uv;
    return output;
}

[earlydepthstencil]
G_BUFFER_OUTPUT DefaultPixelShader(VS_OUTPUT input)
{
    G_BUFFER_OUTPUT output;
    float4 cColor = float4(0, 0, 0, 1);
    if (drawMask & MATERIAL_ALBEDO_MAP) {
        cColor = albedoMap.Sample(gssWrap, input.uv);
    }
    else
    {
        cColor = diffuse;
    }
    
    // 노멀값 조정
    if (drawMask & MATERIAL_NORMAL_MAP)
    {
        float3x3 TBN = float3x3(normalize(input.tangent), normalize(input.biTangent), normalize(input.normal));
        float3 vNormal = normalize(normalMap.Sample(gssWrap, input.uv).rgb * 2.0f - 1.0f); //[0, 1] → [-1, 1]
        input.normal = normalize(mul(vNormal, TBN));
    }
    else
    {
        input.normal = normalize(input.normal);
    }

    output.color = cColor;
    output.normal = float4(input.normal, 1.0f);
    output.position = float4(input.positionW, 1.0f);
    output.emissive = (drawMask & MATERIAL_EMISSIVE_MAP) ? emissiveMap.Sample(gssWrap, input.uv) * emissive : emissive;
    // drawOutline 옵션이 true인 오브젝트들은 외곽선을 그린다.
    output.depth = intValue ? -1.f : length(input.positionW - cameraPosition);
    return output;
}


SHADOW_MAP_VS_OUTPUT DefaultShadowVertexShader(VS_INPUT input)
{
    SHADOW_MAP_VS_OUTPUT output;
 	// 조명 계산을 위해 월드좌표내에서의 포지션값을 계산해 따로 저장
    output.positionW = (float3) mul(float4(input.position, 1.0f), worldTransform);
    // 이 때 변환 행렬은 조명의 위치 기준 카메라의 변환 행렬이다.
    output.position = mul(mul(float4(output.positionW, 1.0f), lights[lightIndex].view), lights[lightIndex].projection);
    
    return output;
}


[earlydepthstencil]
float DefaultShadowPixelShader(SHADOW_MAP_VS_OUTPUT input) : SV_TARGET
{
    float output;
    // 빛의 위치에서 정점의 위치까지의 거리를 저장한다.
    return length(input.positionW - lights[lightIndex].position);

}

///////////////////////////////////////////////////////////////////////////////
/// Effect Shader



struct VS_EFFECT_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 biTangent : BITANGENT;
    float2 uv : TEXCOORD;
};

struct VS_EFFECT_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 biTangent : BITANGENT;
    float2 uv : TEXCOORD;
};

VS_EFFECT_OUTPUT EffectVertexShader(VS_EFFECT_INPUT input)
{
    VS_EFFECT_OUTPUT output;

    output.normal = mul(input.normal, (float3x3) worldTransform);
    output.tangent = mul(input.tangent, (float3x3) worldTransform);
    output.biTangent = mul(input.biTangent, (float3x3) worldTransform);
    
    output.positionW = (float3) mul(float4(input.position, 1.0f), worldTransform);

    output.position = mul(mul(float4(output.positionW, 1.0f), view), projection);
   
    int x = indexInfo.index % indexInfo.col;
    int y = indexInfo.index / indexInfo.col;
    float xWid = 1.f / indexInfo.col;
    float yWid = 1.f / indexInfo.row;
    float2 startUV = float2(xWid * x, yWid * y);
    
    output.uv = startUV + input.uv * float2(xWid, yWid);
    return output;
}

[earlydepthstencil]
G_BUFFER_OUTPUT EffectPixelShader(VS_EFFECT_OUTPUT input)
{
    G_BUFFER_OUTPUT output;
    float4 cColor = float4(0, 0, 0, 1);
    if (drawMask & MATERIAL_ALBEDO_MAP)
    {
        cColor = albedoMap.Sample(gssWrap, input.uv);
    }
    else
    {
        cColor = diffuse;
    }
    
    // 노멀값 조정
    if (drawMask & MATERIAL_NORMAL_MAP)
    {
        float3x3 TBN = float3x3(normalize(input.tangent), normalize(input.biTangent), normalize(input.normal));
        float3 vNormal = normalize(normalMap.Sample(gssWrap, input.uv).rgb * 2.0f - 1.0f); //[0, 1] → [-1, 1]
        input.normal = normalize(mul(vNormal, TBN));
    }
    else
    {
        input.normal = normalize(input.normal);
    }
    if (cColor.a < 0.3f)
        discard;
    output.color = cColor;
    output.normal = float4(input.normal, 1.0f);
    output.position = float4(input.positionW, 1.0f);
    output.emissive = (drawMask & MATERIAL_EMISSIVE_MAP) ? emissiveMap.Sample(gssWrap, input.uv) * emissive : emissive;
    // Effect는 깊이값을 쓰지 않는다. (그림자 x)
    return output;
}



///////////////////////////////////////////////////////////////////////////////
/// 
struct VS_SKINNED_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 biTangent : BITANGENT;
    float2 uv : TEXCOORD;
    uint4 boneIndex : BONEINDEX;
    float4 boneWeight : BONEWEIGHT;
};

struct VS_SKINNED_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normal : NORMAL;
    float4 slideUVVec : slideUV;
    float3 tangent : TANGENT;
    float3 biTangent : BITANGENT;
    float2 uv : TEXCOORD;
};

VS_SKINNED_OUTPUT SkinnedVertexShader(VS_SKINNED_INPUT input)
{
    
    
    VS_SKINNED_OUTPUT output;
    output.positionW = float3(0, 0, 0);
    output.normal = float3(0, 0, 0);
    output.tangent = float3(0, 0, 0);
    output.biTangent = float3(0, 0, 0);
    
    matrix mtxVertexToBoneWorld;
    for (int i = 0; i < 4; ++i)
    {
        if (input.boneWeight[i] > 0.0001f)
        {
            mtxVertexToBoneWorld = mul(offsetTransform[input.boneIndex[i]], skinnedWorldTransforms[input.boneIndex[i]]);
            output.positionW += input.boneWeight[i] * mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
            output.normal += input.boneWeight[i] * mul(input.normal, (float3x3) mtxVertexToBoneWorld);
            output.tangent += input.boneWeight[i] * mul(input.tangent, (float3x3) mtxVertexToBoneWorld);
            output.biTangent += input.boneWeight[i] * mul(input.biTangent, (float3x3) mtxVertexToBoneWorld);
        }
    }

    output.slideUVVec = float4(output.normal, 1.0f);
    output.position = mul(mul(float4(output.positionW, 1.0f), view), projection);
    output.uv = input.uv;

    return output;
}

[earlydepthstencil]
G_BUFFER_OUTPUT SkinnedPixelShader(VS_SKINNED_OUTPUT input)
{
    G_BUFFER_OUTPUT output;
    float4 cColor = float4(0, 0, 0, 1);
    if (drawMask & MATERIAL_ALBEDO_MAP)
    {
        cColor = albedoMap.Sample(gssWrap, input.uv);
    }
    else
    {
        cColor = diffuse;
    }
    
    // 노멀값 조정
    if (drawMask & MATERIAL_NORMAL_MAP)
    {
        float3x3 TBN = float3x3(normalize(input.tangent), normalize(input.biTangent), normalize(input.normal));
        float3 vNormal = normalize(normalMap.Sample(gssWrap, input.uv).rgb * 2.0f - 1.0f); //[0, 1] → [-1, 1]
        input.normal = normalize(mul(vNormal, TBN));
    }
    else
    {
        input.normal = normalize(input.normal);
    }

    // 투명하지 않은경우에만 쓴다.
    output.color = lerp(cColor, float4(1, 0, 0, 1), floatValue);
    output.normal = float4(input.normal, 1.0f);
    output.position = float4(input.positionW, 1.0f);
    output.emissive = (drawMask & MATERIAL_EMISSIVE_MAP) ? emissiveMap.Sample(gssWrap, input.uv) * emissive : emissive;
    //output.depth = length(input.positionW - cameraPosition);
    output.depth = -1;
    return output;
}

[earlydepthstencil]
G_BUFFER_OUTPUT SkinnedTransparentPixelShader(VS_SKINNED_OUTPUT input)
{
    G_BUFFER_OUTPUT output;
    float4 cColor = float4(0, 0, 0, 1);
    if (drawMask & MATERIAL_ALBEDO_MAP)
    {
        cColor = albedoMap.Sample(gssWrap, input.uv);
    }
    else
    {
        cColor = diffuse;
    }
    
    // 노멀값 조정
    if (drawMask & MATERIAL_NORMAL_MAP)
    {
        float3x3 TBN = float3x3(normalize(input.tangent), normalize(input.biTangent), normalize(input.normal));
        float3 vNormal = normalize(normalMap.Sample(gssWrap, input.uv).rgb * 2.0f - 1.0f); //[0, 1] → [-1, 1]
        input.normal = normalize(mul(vNormal, TBN));
    }
    else
    {
        input.normal = normalize(input.normal);
    }
    

    output.depth = length(input.positionW - cameraPosition);
        
    // 투명한 경우에만 쓴다.
    output.slideUVVec = input.slideUVVec;
    
    return output;
}


[earlydepthstencil]
float4 SkinnedLobbyPixelShader(VS_SKINNED_OUTPUT input) : SV_TARGET
{
    float4 cColor = float4(0, 0, 0, 1);
    if (drawMask & MATERIAL_ALBEDO_MAP)
    {
        cColor = albedoMap.Sample(gssWrap, input.uv);
    }
    else
    {
        cColor = diffuse;
    }
    
    // 노멀값 조정
    if (drawMask & MATERIAL_NORMAL_MAP)
    {
        float3x3 TBN = float3x3(normalize(input.tangent), normalize(input.biTangent), normalize(input.normal));
        float3 vNormal = normalize(normalMap.Sample(gssWrap, input.uv).rgb * 2.0f - 1.0f); //[0, 1] → [-1, 1]
        input.normal = normalize(mul(vNormal, TBN));
    }
    else
    {
        input.normal = normalize(input.normal);
    }
    
    float4 color = CalculateLight(cColor, input.positionW, input.normal);
    return color;
}



SHADOW_MAP_VS_OUTPUT SkinnedShadowVertexShader(VS_SKINNED_INPUT input)
{ 
    SHADOW_MAP_VS_OUTPUT output;
    output.positionW = float3(0, 0, 0);
    
    matrix mtxVertexToBoneWorld;
    for (int i = 0; i < 4; ++i)
    {
        if (input.boneWeight[i] > 0.0001f)
        {
            mtxVertexToBoneWorld = mul(offsetTransform[input.boneIndex[i]], skinnedWorldTransforms[input.boneIndex[i]]);
            output.positionW += input.boneWeight[i] * mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
        }
    }
    output.position = mul(mul(float4(output.positionW, 1.0f), lights[lightIndex].view), lights[lightIndex].projection);

    return output;
}

[earlydepthstencil]
float SkinnedShadowPixelShader(SHADOW_MAP_VS_OUTPUT input) : SV_TARGET
{
    float output;
    // 빛의 위치에서 정점의 위치까지의 거리를 저장한다.
    return length(input.positionW - lights[lightIndex].position);
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
    // 버튼이 눌린 상태를 표현하기 위함
    if (worldTransform._14 > 0.1f) {
        uv.x -= 0.05f;
        uv.y -= 0.05f;
        if (uv.x < 0.f || uv.y < 0.f)
            discard;
    }
    if (sizeuv.x + startuv.x < input.uv.x) 
        discard;
    if (sizeuv.y - startuv.y < input.uv.y) 
        discard;
    float4 color = albedoMap.Sample(gssClamp, uv);
    if (color.a < 0.5f)
        discard;
    if (worldTransform._14 > 0.1f || worldTransform._24 > 0.1f)
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
    // 현재 프러스텀은 정점에 월드변환이 적용되어 오는 상태이다. (수정 필요)
    VS_BOUNDING_OUTPUT output;
    output.position = mul(mul(mul(float4(input.position, 1.0f), worldTransform), view), projection);
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
    float3 tangent : TANGENT;
    float3 biTangent : BITANGENT;
    float2 uv : TEXCOORD;
    float4x4 worldMatrix : WORLDMAT;
};


VS_OUTPUT InstanceVertexShader(VS_INSTANCING_INPUT input) {
    VS_OUTPUT output;
    
    output.normal = mul(input.normal, (float3x3) input.worldMatrix);
    output.tangent = mul(input.tangent, (float3x3) input.worldMatrix);
    output.biTangent = mul(input.biTangent, (float3x3) input.worldMatrix);

	// 조명 계산을 위해 월드좌표내에서의 포지션값을 계산해 따로 저장
    output.positionW = (float3) mul(float4(input.position, 1.0f), input.worldMatrix);

    output.position = mul(mul(float4(output.positionW, 1.0f), view), projection);


    output.uv = input.uv;
    return output;
}


[earlydepthstencil]
G_BUFFER_OUTPUT InstancePixelShader(VS_OUTPUT input)
{
    G_BUFFER_OUTPUT output;
    float4 cColor = diffuse;
    if (drawMask & MATERIAL_ALBEDO_MAP)
    {
        cColor = albedoMap.Sample(gssWrap, input.uv);
    }
    else
    {
        cColor = diffuse;
    }
    
    // 노멀값 조정
    if (drawMask & MATERIAL_NORMAL_MAP)
    {
        float3x3 TBN = float3x3(normalize(input.tangent), normalize(input.biTangent), normalize(input.normal));
        float3 vNormal = normalize(normalMap.Sample(gssWrap, input.uv).rgb * 2.0f - 1.0f); //[0, 1] → [-1, 1]
        input.normal = normalize(mul(vNormal, TBN));
    }
    else
    {
        input.normal = normalize(input.normal);
    }
    
    output.color = cColor;
    output.normal = float4(input.normal, 1.0f);
    output.position = float4(input.positionW, 1.0f);
    output.emissive = (drawMask & MATERIAL_EMISSIVE_MAP) ? emissiveMap.Sample(gssWrap, input.uv) * emissive : emissive;
    output.depth = length(input.positionW - cameraPosition);
    return output;
}



SHADOW_MAP_VS_OUTPUT InstanceShadowVertexShader(VS_INSTANCING_INPUT input)
{
    SHADOW_MAP_VS_OUTPUT output;
    output.positionW = (float3) mul(float4(input.position, 1.0f), input.worldMatrix);
    // 이 때 변환 행렬은 조명의 위치 기준 카메라의 변환 행렬이다.
    output.position = mul(mul(float4(output.positionW, 1.0f), lights[lightIndex].view), lights[lightIndex].projection);
    return output;
}


[earlydepthstencil]
float InstanceShadowPixelShader(SHADOW_MAP_VS_OUTPUT input) : SV_TARGET
{
    float output;
    // 빛의 위치에서 정점의 위치까지의 거리를 저장한다.
    return length(input.positionW - lights[lightIndex].position);
}


struct VS_S_IN
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct VS_S_OUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};


VS_S_OUT SkyBoxVertexShader(VS_S_IN input)
{
    
    float3 pos = input.position + cameraPosition;
    VS_S_OUT output;
    
    output.position = mul(mul(float4(pos, 1.0f), view), projection).xyww;
    output.uv = input.uv;
    return output;
}

float4 SkyBoxPixelShader(VS_S_OUT input) : SV_TARGET
{
    float2 uv = input.uv;
    uv.y = 1 - uv.y;
    float4 cColor = albedoMap.Sample(gssClamp, uv);
    return cColor;
}

struct VS_LIGHTING_IN
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct VS_LIGHTING_OUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};


VS_LIGHTING_OUT LightingVertexShader(VS_LIGHTING_IN input)
{
    VS_LIGHTING_OUT output;
    output.position = float4(input.position, 1.0f);
    output.uv = input.uv;
    return output;
}



float4 PreLightingColor(float4 baseColor, float2 uv)
{
    float4 color = baseColor;

    return color;
}

float4 PostLightingColor(float4 baseColor, float2 uv)
{
    float4 color = baseColor;
    // hit value
    if (intValue == 1)
    {
        float dist = length(uv - float2(0.5f, 0.5f));
        // 화면 바깥 부분이 빨갛게 보이는 효과
        color = lerp(color, float4(1, 0, 0, 1), max(dist - 0.3f, 0) * floatValue);
    }
    return color;
}


float4 LightingPixelShader(VS_LIGHTING_OUT input) : SV_TARGET
{
    
    float4 uvSlide =  uvSlideTexture.Sample(gssWrap, input.uv);
    //return abs(uvSlideTexture.Sample(gssWrap, input.uv));
    float3 positionW = positionTexture.Sample(gssWrap, input.uv).xyz;
    
    float3 changeNormal = uvSlideTexture.Sample(gssWrap, input.uv).xyz;

    // 투명한 경우 기존 픽셀의 노말을 흔들어 주고, 약간 어둡게 칠한다.
    float3 normal = lerp(normalTexture.Sample(gssWrap, input.uv).xyz, changeNormal, float3(0.5, 0.5, 0.5));
    float4 color;
    
    if (isBorder(input.uv))
    {
        color = float4(0.0, 0, 0.1, 1);
    }
    else
    {
        color = colorTexture.Sample(gssWrap, input.uv);
        if (length(changeNormal) > 0.f)
            color *= 0.9f;
    }

 
    
    color = PreLightingColor(color, input.uv);

    // 이곳에서 조명처리를 해준다.
    float4 cColor = CalculateLight(color, positionW, normal);
    
    
    // uv좌표를 텍셀좌표로 바꾼다. (Load함수를 쓰기 위함)
    int3 texLocation = int3(input.uv.x * CWIDTH, input.uv.y * CHEIGHT, 0);
    cColor += emissiveTexture.Load(texLocation) / 5.f;
    
    [unroll(8)]
    for (int i = 0; i < 8; ++i)
    {
        [unroll(15)]
        for (int j = 1; j <= 15; ++j)
        {
            float4 emissiveColor = emissiveTexture.Load(texLocation + d[i] * j);
            cColor += emissiveColor / (pow(j, 1.7f) + 3);
        }
    }
    
    cColor = PostLightingColor(cColor, input.uv);
    return cColor;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ParticleStreamOut
#define GRAVITY 98

#define PARTICLES_TEXTURE_ROW 5
#define PARTICLES_TEXTURE_COLUMN 5

#define WATER_DISPENSER_USE 0
#define HEAL_ITEM_USE 1

struct VS_PARTICLE_INPUT
{
    float3 position : POSITION;
    float3 velocity : VELOCITY;
    float2 boardSize : BOARDSIZE;
    float lifetime : LIFETIME;
    uint type : PARTICLETYPE;
};

VS_PARTICLE_INPUT ParticleStreamOutVertexShader(VS_PARTICLE_INPUT input)
{
    return input;
}

void WaterDispenserUseParticle(VS_PARTICLE_INPUT input, inout PointStream<VS_PARTICLE_INPUT> outStream);
void HealItemUseParticle(VS_PARTICLE_INPUT input, inout PointStream<VS_PARTICLE_INPUT> outStream);

[maxvertexcount(20)] // sizeof(VS_PARTICLE_INPUT) 와 maxvertexcount 를 곱해서 1024바이트를 넘으면 안된다.
void ParticleStreamOutGeometryShader(point VS_PARTICLE_INPUT input[1], inout PointStream<VS_PARTICLE_INPUT> outStream)
{
    VS_PARTICLE_INPUT particle = input[0];
    switch (particle.type)
    {
        case WATER_DISPENSER_USE:
            WaterDispenserUseParticle(particle, outStream);
            break;
        case HEAL_ITEM_USE:
            HealItemUseParticle(particle, outStream);
            break;
    }
}

void WaterDispenserUseParticle(VS_PARTICLE_INPUT input, inout PointStream<VS_PARTICLE_INPUT> outStream)
{
    // 수명이 남아 있으면 이동시키고(중력의 영향을 받도록 한다.), 
    if (input.lifetime > 0.0f)  // 수명이 남아있을 경우
    {
        input.lifetime -= elapsedTime;
        input.position += input.velocity * elapsedTime;
        input.velocity *= 1.0 - elapsedTime * 4.f;
        
        outStream.Append(input);
    }
}
void HealItemUseParticle(VS_PARTICLE_INPUT input, inout PointStream<VS_PARTICLE_INPUT> outStream)
{
    // 수명이 남아 있으면 이동시키고(중력의 영향을 받도록 한다.), 
    if (input.lifetime > 0.0f)  // 수명이 남아있을 경우
    {
        input.lifetime -= elapsedTime;
        input.position += input.velocity * elapsedTime;
        input.position.y += elapsedTime;
        input.velocity *= 1.0 - elapsedTime * 4.f;
        
        outStream.Append(input);
    }
}


float3 RandomDirection(float seedOffset)
{
    float2 seed = float2(currentTime + seedOffset, currentTime - seedOffset);
    float3 direction;
    
    direction.x = RandomFloat(seed) - 0.5f;
    direction.y = RandomFloat(seed + float2(0.435, 0.12)) - 0.5f;
    direction.z = RandomFloat(seed + float2(0.0345, 0.74)) - 0.5f;
    direction = normalize(direction);
    return direction;
}

float RandomFloat(float2 co)
{
    return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453123);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ParticleDraw

struct VS_PARTICLE_OUTPUT
{
    float3 positionW : POSITION;
    float2 boardSize : BOARDSIZE;
    float3 normal : NORMAL;
    uint type : PARTICLETYPE;
    float lifetime : LIFETIME;
};

struct GS_PARTICLE_OUTPUT
{
    float3 positionW : POSITION;
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    uint type : PARTICLETYPE;
    float lifetime : LIFETIME;
};

VS_PARTICLE_OUTPUT ParticleDrawVertexShader(VS_PARTICLE_INPUT input)
{
    VS_PARTICLE_OUTPUT output;
    
    output.positionW = input.position;
    output.boardSize = input.boardSize;

    output.normal = cameraPosition - output.positionW;
    output.normal = normalize(output.normal);
    
    output.type = input.type;
    output.lifetime = input.lifetime;
    
    return output;
}

[maxvertexcount(4)]
void ParticleDrawGeometryShader(point VS_PARTICLE_OUTPUT input[1], inout TriangleStream<GS_PARTICLE_OUTPUT> outStream)
{
    GS_PARTICLE_OUTPUT output;
    output.normal = input[0].normal;
    output.type = input[0].type;
    output.lifetime = input[0].lifetime;
    
    float3 rightVector = normalize(cross(float3(0, 1, 0), input[0].normal)); // Y축과 look벡터 외적해서 rightVector를 얻는다.
    float3 upVector = normalize(cross(input[0].normal, rightVector)); //
    
    float3 dxOffset = rightVector * input[0].boardSize.x / 2.0f;
    float3 dyOffset = upVector * input[0].boardSize.y / 2.0f;
    
     // 시계방향이 앞쪽
    float3 dx[4] = { -dxOffset, dxOffset, -dxOffset, dxOffset };
    float3 dy[4] = { -dyOffset, -dyOffset, dyOffset, dyOffset };
    
    float2 startUV = float2(output.type % PARTICLES_TEXTURE_COLUMN / (float) (PARTICLES_TEXTURE_COLUMN), output.type / PARTICLES_TEXTURE_ROW / (float) (PARTICLES_TEXTURE_ROW));
    float2 widUV = float2(1.0f / PARTICLES_TEXTURE_COLUMN, 1.0f / PARTICLES_TEXTURE_ROW);
    
    float2 uv[4] = { startUV + widUV, startUV + float2(0, widUV.y), startUV + float2(widUV.x, 0), startUV }; // 오른쪽위, 왼쪽위, 오른쪽아래, 왼쪽아래
    for (int i = 0; i < 4; ++i)
    {
        output.positionW = input[0].positionW + dx[i] + dy[i];
        output.position = mul(mul(float4(output.positionW, 1.0f), view), projection);
        output.uv = uv[i];
        outStream.Append(output);
    }
}

G_BUFFER_OUTPUT ParticleDrawPixelShader(GS_PARTICLE_OUTPUT input)
{
    // type을 이용하여 원하는 텍스처를 선택
    float4 color = albedoMap.Sample(gssWrap, input.uv);
    
    if (color.a < 0.1f)
        discard;
    
    G_BUFFER_OUTPUT output;
    output.depth = length(input.positionW - cameraPosition);
    output.normal = float4(input.normal, 1.0f);
    output.position = float4(input.positionW, 1.0f);
    
    //output.color = color;
    if (input.type == WATER_DISPENSER_USE || input.type == HEAL_ITEM_USE)
    {
        output.emissive = color * (input.lifetime / 1.f);
    }
    else
    {
        output.emissive = color * (input.lifetime / 1.f);;
    }
    
    return output;
}

struct VS_POST_IN
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct VS_POST_OUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};


VS_POST_OUT PostVertexShader(VS_POST_IN input)
{
    VS_POST_OUT output;
    output.position = float4(input.position, 1.0f);
    output.uv = input.uv;
    return output;
}

float4 PostPixelShader(VS_POST_OUT input) : SV_TARGET
{
    return PostBuffer.Sample(gssWrap, input.uv);
}