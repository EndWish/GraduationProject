
#define CWIDTH 1280
#define CHEIGHT 720

// 출력하지 않는 렌더타겟이 있을 수 있다.
#pragma warning( disable :  3578 )
#include "Light.hlsl"



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
    
    float4 uvSlide = uvSlideTexture.Sample(gssWrap, input.uv);
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
