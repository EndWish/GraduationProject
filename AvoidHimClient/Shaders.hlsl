// ��� �ñ״�ó�� 64���� 32-��Ʈ �迭�� ������.
// ���� ����ϴ� �Ű������� ���ʿ� ��ġ�ϴ� ���� ����.

// ������� �ʴ� ����Ÿ���� ���� �� �ִ�.
#pragma warning( disable :  3578 )

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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define WATER_HEIGHT 150.0f

#define MATERIAL_ALBEDO_MAP		0x01
#define MATERIAL_NORMAL_MAP      0x02


// �ؽ�ó
Texture2D albedoMap : register(t5);
Texture2D normalMap : register(t6);

struct G_BUFFER_OUTPUT {
    float4 color : SV_TARGET0;   // ������ ó���ϱ� ���� �ȼ��� ����
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

VS_OUTPUT DefaultVertexShader(VS_INPUT input) {
    VS_OUTPUT output;

    output.normal = mul(input.normal, (float3x3) worldTransform);
    output.tangent = mul(input.tangent, (float3x3) worldTransform);
    output.biTangent = mul(input.biTangent, (float3x3) worldTransform);

	// ���� ����� ���� ������ǥ�������� �����ǰ��� ����� ���� ����
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
    
    // ��ְ� ����
    if (drawMask & MATERIAL_NORMAL_MAP)
    {
        float3x3 TBN = float3x3(normalize(input.tangent), normalize(input.biTangent), normalize(input.normal));
        float3 vNormal = normalize(normalMap.Sample(gssWrap, input.uv).rgb * 2.0f - 1.0f); //[0, 1] �� [-1, 1]
        input.normal = normalize(mul(vNormal, TBN));
    }
    else
    {
        input.normal = normalize(input.normal);
    }

    output.color = cColor;
    output.normal = float4(input.normal, 1.0f);
    output.position = float4(input.positionW, 1.0f);
    output.emissive = float4(0, 0, 0, 1);
    // ���̰����� ī�޶󿡼� �ش� ���� ��ġ������ �Ÿ��� ��´�.
    output.depth = length(input.positionW - cameraPosition);
    return output;
}


SHADOW_MAP_VS_OUTPUT DefaultShadowVertexShader(VS_INPUT input)
{
    SHADOW_MAP_VS_OUTPUT output;
 	// ���� ����� ���� ������ǥ�������� �����ǰ��� ����� ���� ����
    output.positionW = (float3) mul(float4(input.position, 1.0f), worldTransform);
    // �� �� ��ȯ ����� ������ ��ġ ���� ī�޶��� ��ȯ ����̴�.
    output.position = mul(mul(float4(output.positionW, 1.0f), lights[intValue].view), lights[intValue].projection);
    
    return output;
}


[earlydepthstencil]
float DefaultShadowPixelShader(SHADOW_MAP_VS_OUTPUT input) : SV_TARGET
{
    float output;
    // ���� ��ġ���� ������ ��ġ������ �Ÿ��� �����Ѵ�.
    return length(input.positionW - lights[intValue].position);

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
    
    // ��ְ� ����
    if (drawMask & MATERIAL_NORMAL_MAP)
    {
        float3x3 TBN = float3x3(normalize(input.tangent), normalize(input.biTangent), normalize(input.normal));
        float3 vNormal = normalize(normalMap.Sample(gssWrap, input.uv).rgb * 2.0f - 1.0f); //[0, 1] �� [-1, 1]
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
    output.emissive = float4(0, 0, 0, 1);
    // Effect�� ���̰��� ���� �ʴ´�. (�׸��� x)
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
    
    // ��ְ� ����
    if (drawMask & MATERIAL_NORMAL_MAP)
    {
        float3x3 TBN = float3x3(normalize(input.tangent), normalize(input.biTangent), normalize(input.normal));
        float3 vNormal = normalize(normalMap.Sample(gssWrap, input.uv).rgb * 2.0f - 1.0f); //[0, 1] �� [-1, 1]
        input.normal = normalize(mul(vNormal, TBN));
    }
    else
    {
        input.normal = normalize(input.normal);
    }
    

    output.depth = length(input.positionW - cameraPosition);

    // �������� ������쿡�� ����.
    output.color = cColor;
    output.normal = float4(input.normal, 1.0f);
    output.position = float4(input.positionW, 1.0f);
    output.emissive = float4(0, 0, 0, 1);

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
    
    // ��ְ� ����
    if (drawMask & MATERIAL_NORMAL_MAP)
    {
        float3x3 TBN = float3x3(normalize(input.tangent), normalize(input.biTangent), normalize(input.normal));
        float3 vNormal = normalize(normalMap.Sample(gssWrap, input.uv).rgb * 2.0f - 1.0f); //[0, 1] �� [-1, 1]
        input.normal = normalize(mul(vNormal, TBN));
    }
    else
    {
        input.normal = normalize(input.normal);
    }
    

    output.depth = length(input.positionW - cameraPosition);
        
    // ������ ��쿡�� ����.
    output.slideUVVec = input.slideUVVec;
    
    return output;
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
    output.position = mul(mul(float4(output.positionW, 1.0f), lights[intValue].view), lights[intValue].projection);

    return output;
}

[earlydepthstencil]
float SkinnedShadowPixelShader(SHADOW_MAP_VS_OUTPUT input) : SV_TARGET
{
    float output;
    // ���� ��ġ���� ������ ��ġ������ �Ÿ��� �����Ѵ�.
    return length(input.positionW - lights[intValue].position);
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
    
    // 2DUI�� �׻� z = 0 (�� ��)�� �׷�����, depth���� ����.
    // �̹����� �׸� �⺻ �簢�� ��ǥ�� �̹����� ��ǥ���� ���̴��ܰ迡�� ���Ѵ�. 
    output.position = float4(input.position.x + startpos.x, input.position.y + startpos.y, 0, 1);
    
    float x, y;
    
    // 
    if (input.position.x == 0)
        output.uv.x = startuv.x;
    else  
        output.uv.x = startuv.x + 1;
    
    // ���κ��� 1, �Ʒ��� 0���� 
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
    // ��ư�� ���� ���¸� ǥ���ϱ� ����
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
    // ���� ���������� ������ ���庯ȯ�� ����Ǿ� ���� �����̴�. (���� �ʿ�)
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

	// ���� ����� ���� ������ǥ�������� �����ǰ��� ����� ���� ����
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
    
    // ��ְ� ����
    if (drawMask & MATERIAL_NORMAL_MAP)
    {
        float3x3 TBN = float3x3(normalize(input.tangent), normalize(input.biTangent), normalize(input.normal));
        float3 vNormal = normalize(normalMap.Sample(gssWrap, input.uv).rgb * 2.0f - 1.0f); //[0, 1] �� [-1, 1]
        input.normal = normalize(mul(vNormal, TBN));
    }
    else
    {
        input.normal = normalize(input.normal);
    }
    
    output.color = cColor;
    output.normal = float4(input.normal, 1.0f);
    output.position = float4(input.positionW, 1.0f);
    output.emissive = float4(0, 0, 0, 1);
    output.depth = length(input.positionW - cameraPosition);
    return output;
}



SHADOW_MAP_VS_OUTPUT InstanceShadowVertexShader(VS_INSTANCING_INPUT input)
{
    SHADOW_MAP_VS_OUTPUT output;
    output.positionW = (float3) mul(float4(input.position, 1.0f), input.worldMatrix);
    // �� �� ��ȯ ����� ������ ��ġ ���� ī�޶��� ��ȯ ����̴�.
    output.position = mul(mul(float4(output.positionW, 1.0f), lights[intValue].view), lights[intValue].projection);
    return output;
}


[earlydepthstencil]
float InstanceShadowPixelShader(SHADOW_MAP_VS_OUTPUT input) : SV_TARGET
{
    float output;
    // ���� ��ġ���� ������ ��ġ������ �Ÿ��� �����Ѵ�.
    return length(input.positionW - lights[intValue].position);
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

float4 LightingPixelShader(VS_LIGHTING_OUT input) : SV_TARGET
{
    
    float4 uvSlide =  uvSlideTexture.Sample(gssWrap, input.uv);
    //return abs(uvSlideTexture.Sample(gssWrap, input.uv));
    float3 positionW = positionTexture.Sample(gssWrap, input.uv).xyz;
    
    float3 changeNormal = uvSlideTexture.Sample(gssWrap, input.uv).xyz;
    float3 normal = lerp(normalTexture.Sample(gssWrap, input.uv).xyz, changeNormal, float3(0.5, 0.5, 0.5));
    //float3 normal =normalTexture.Sample(gssWrap, input.uv).xyz;
    float4 color = colorTexture.Sample(gssWrap, input.uv);

    //return float4(depthTexture.Sample(gssWrap, input.uv) / 20, 0, 0, 1);
    // �̰����� ����ó���� ���ش�.
    float4 cColor = CalculateLight(color, positionW, normal);
    return cColor;
}

