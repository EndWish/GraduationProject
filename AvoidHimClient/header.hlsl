

// 출력하지 않는 렌더타겟이 있을 수 있다.
#pragma warning( disable :  3578 )

// 루드 시그니처는 64개의 32-비트 배열로 구성됨.
// 많이 사용하는 매개변수를 앞쪽에 배치하는 것이 좋음.


// 샘플러
SamplerState gssWrap : register(s0);
SamplerState gssClamp : register(s1);
SamplerComparisonState gssPCFShadow : register(s2);

cbuffer cbCameraInfo : register(b1)
{
    matrix view : packoffset(c0);
    matrix projection : packoffset(c4);
    float3 cameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b2)
{
    matrix worldTransform : packoffset(c0);
};

cbuffer cbIntVariable : register(b5)
{
    int intValue : packoffset(c0);
};

cbuffer cbFrameworkInfo : register(b11)
{
    float currentTime : packoffset(c0.x);
    float elapsedTime : packoffset(c0.y);
    uint CWIDTH : packoffset(c0.z);
    uint CHEIGHT : packoffset(c0.w);
};
cbuffer cbFloatVariable : register(b6)
{
    float floatValue : packoffset(c0);
};


cbuffer cbIntVariable : register(b10)
{
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