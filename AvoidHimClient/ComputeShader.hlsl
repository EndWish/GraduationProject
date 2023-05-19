


cbuffer cbFloatVariable : register(b0)
{
    float radarDepth : packoffset(c0.x);
    float radarRatio : packoffset(c0.y);
};

Texture2D<float4> renderTexture : register(t15);


Texture2D<float4> colorTexture : register(t16);
Texture2D<float4> normalTexture : register(t17);
Texture2D<float4> positionTexture : register(t18);
Texture2D<float4> emissiveTexture : register(t19);
Texture2D<float4> uvSlideTexture : register(t20);
Texture2D<float> depthTexture : register(t21);

Texture2D<float4> radarTexture : register(t22);

Texture2D<float> bakedShadow : register(t23);
Texture2D<float> dynamicShadow : register(t24);

RWTexture2D<float4> output : register(u0);

const static int BlurRange = 3;
const static float NearDepth = 15.0f;

groupshared float4 groupSharedCache[BlurRange + 32 + BlurRange][BlurRange + 32 + BlurRange];
    
float gfGaussianBlurMask2D[5][5];

const static float PI = 3.141592f;

float gaussian(float depth, float distance)
{
    // depth값이 NearDepth 미만일경우 1/sqrt(2*pi). => 현 위치 픽셀만 가중치 1.
    float sigma = lerp(1 / sqrt(2 * PI), 10, smoothstep(NearDepth, 30.0f, depth));
    
    // depth값이 점점 커질수록 distance에 따라 고르게 분포된다.
    float result = 1.0f / (sigma * sqrt(2.0 * PI)) * exp(-0.5 * pow((distance / sigma), 2.0f));
    return result;
}


float4 GaussianBlur(int3 n3GroupThreadID, int3 threadID)
{
    float4 result = float4(0, 0, 0, 1);
    float depth = depthTexture[threadID.xy];

    // 겉 픽셀이여서 샘플링할 수 없는 부분은 그대로 출력한다.
    if ((threadID.x < BlurRange) || (threadID.x >= int(renderTexture.Length.x - BlurRange)) || (threadID.y < BlurRange) || (threadID.y >= int(renderTexture.Length.y - BlurRange)))
    {
        result = renderTexture[threadID.xy];
    }
    // nearDepth미만일경우 for문을 돌면서 계산을 하지 않는다.
    else if (depth < NearDepth)
        result = renderTexture[threadID.xy];
    else
    {
        float weightsum = 0.f;
        float4 color = float4(0, 0, 0, 0);
        for (int i = -BlurRange; i <= BlurRange; i++)
        {
            for (int j = -BlurRange; j <= BlurRange; j++)
            {
                float weight = gaussian(depth, length(float2(i, j)));
                color += weight * renderTexture[threadID.xy + int2(i, j)];
                weightsum += weight;
            }
        }     
        result = color / weightsum;
        
    }
    return result;
}

[numthreads(32, 32, 1)]
void radarResult(int3 n3GroupThreadID : SV_GroupThreadID, int3 threadID : SV_DispatchThreadID)
{
    float4 nColor = GaussianBlur(n3GroupThreadID, threadID);
    float depth = depthTexture[threadID.xy];
    float4 radarColor = radarTexture[threadID.xy];
    
    // 외곽선을 그리는 물체들은 depth buffer에서의 값이 음수임
    if (depth < 0)
        depth = -depth;
    // 레이더 범위내에 들어올 경우 와이어 프레임 텍스처를 출력한다.
    
    if (depth <= radarDepth && depth != 0)
    {
        float ratio = radarRatio * clamp((depth - (radarDepth - 30.f)) / 20.f, 0.f, 1.f);
        //float ratio = radarRatio * clamp(depth / max(0.1f, radarDepth - 10.f), 0.f, 1.f);
        output[threadID.xy] = lerp(nColor, radarColor, ratio);
    }
        
    // 아닐경우 기존 블러 처리를 하여 그대로 내보낸다.
    else
    {
        output[threadID.xy] = nColor;
    }
}


[numthreads(32, 32, 1)]
void MergeShadow(int3 n3GroupThreadID : SV_GroupThreadID, int3 threadID : SV_DispatchThreadID)
{
    float bakeDepth = bakedShadow[threadID.xy] == 0 ? 1000 : bakedShadow[threadID.xy];
    float dynamicDepth = dynamicShadow[threadID.xy] == 0 ? 1000 : dynamicShadow[threadID.xy];

    output[threadID.xy] = min(bakeDepth, dynamicDepth);
}