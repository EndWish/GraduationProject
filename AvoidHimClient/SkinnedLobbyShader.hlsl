
#include "Light.hlsl"

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

