#define MAX_LIGHTS			20


#define POINT_LIGHT			1
#define SPOT_LIGHT			2
#define DIRECTIONAL_LIGHT	3

#define EPSILON				1.0e-5
#define NUM_SHADOW_MAP 10

struct LIGHT
{
    float4x4 view;
    float4x4 projection;
    
	float4 ambient;
	float4 diffuse;
	float4 specular;

	float3 position;
	float range;

	float3 offset;	// 빛을 내는 물체의 중심으로부터 떨어진 값
	float theta; //cos(m_fTheta), 스포트라이트에서 사용
	float3 attenuation;
	float phi; //cos(m_fPhi), 스포트라이트에서 사용
	float3 direction;
    float falloff;

	// 이 빛을 내고 있는 오브젝트의 포인터 (미사용)
	float4 object;

	// 1 = 점, 2 = 스포트, 3 = 직접
	int lightType;

	// 이 빛이 켜져있는 상태인지 확인
	bool enable;

    float2 padding;
};

// 샘플러
SamplerState gssWrap : register(s0);
SamplerState gssClamp : register(s1);
SamplerComparisonState gssPCFShadow : register(s2);

cbuffer cbLightInfo : register(b3) {
    LIGHT lights[MAX_LIGHTS];
    float4 globalAmbient;
    int nLight;
}

cbuffer cbMaterialInfo : register(b4) {
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float4 emissive;
    uint drawMask;
}


Texture2D<float4> colorTexture : register(t7);
Texture2D<float4> normalTexture : register(t8);
Texture2D<float4> positionTexture : register(t9);
Texture2D<float4> emissiveTexture : register(t10);
Texture2D<float4> uvSlideTexture : register(t11);
Texture2D<float> depthTexture : register(t12);


Texture2D<float> shadowMapTexture_1 : register(t21);
Texture2D<float> shadowMapTexture_2 : register(t22);
Texture2D<float> shadowMapTexture_3 : register(t23);
Texture2D<float> shadowMapTexture_4 : register(t24);
Texture2D<float> shadowMapTexture_5 : register(t25);
Texture2D<float> shadowMapTexture_6 : register(t26);
Texture2D<float> shadowMapTexture_7 : register(t27);
Texture2D<float> shadowMapTexture_8 : register(t28);
Texture2D<float> shadowMapTexture_9 : register(t29);
Texture2D<float> shadowMapTexture_10 : register(t30);

float2 convertViewportToUV(float2 _viewport) {
    return float2((_viewport.x + 1) / 2, 1 - (_viewport.y + 1) / 2);
    
}

float4 DirectionalLight(int _nIndex, float3 _normal, float3 _toCamera, float4 _color)
{

	// 빛의 방향
	float3 toLight = -lights[_nIndex].direction;
	
	// 빛의 방향과 정점의 법선으로 각도 계산
	float diffuseFactor = dot(toLight, _normal);
	float specularFactor = 0.0f;
	if (diffuseFactor > EPSILON) {
		// 반사벡터를 구해 시선벡터와 내적하여 빛의 양 계산
		float3 reflectVector = reflect(-toLight, _normal);
		specularFactor = pow(max(dot(reflectVector, _toCamera), 0.0f), specular.a);
	}

    return (_color * (lights[_nIndex].diffuse * max(diffuseFactor, 0) * diffuse) + (lights[_nIndex].specular * specularFactor * specular));
}


float4 PointLight(int _nIndex, float3 _position, float3 _normal, float3 _toCamera, float4 _color)
{
    float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
	// 빛과 정점사이 벡터로 거리 계산
	float3 toLight = lights[_nIndex].position - _position;
	float distance = length(toLight);
	
	// 설정한 range값보다 가까울 경우
	if (distance <= lights[_nIndex].range)	{
		float specularFactor = 0.0f;
		toLight /= distance;
		
		float diffuseFactor = dot(toLight, _normal);
        if (diffuseFactor > EPSILON) {
			if (specular.a != 0.0f)	{
				float3 reflectVec = reflect(-toLight, _normal);
                specularFactor = pow(max(dot(reflectVec, _toCamera), 0.0f), specular.a);
            }
		}
		// 1/(x+y*d+z*d*d). distance = 0일 경우 1/x
        float attenuationFactor = 1.0f / dot(lights[_nIndex].attenuation, float3(1.0f, distance, distance*distance));
        color = (_color * (lights[_nIndex].diffuse * diffuseFactor * diffuse) + (lights[_nIndex].specular * specularFactor * specular)) * attenuationFactor;
    }
	return color;
}


float4 SpotLight(int _nIndex, float3 _position, float3 _normal, float3 _toCamera, float4 _color) {
    float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float3 toLight = lights[_nIndex].position - _position;
    float distance = length(toLight);
    toLight /= distance;
	
    // 거리안에 있는지 확인
    if (distance <= lights[_nIndex].range)
    {
        float theta = acos(dot(lights[_nIndex].direction, -toLight)); // 조명의 방향과 조명에서 오브젝트로 가는 방향의 각도 (라디안)
        if (theta <= lights[_nIndex].theta / 2)  // 조명을 받지 않을 경우
        {
            /// 외부원안쪽에 있을 경우
            float fDiffuseFactor = max(0, dot(toLight, _normal));
            float specularFactor = 0.0f;
            if (specular.a != 0.0f)
            {
                float3 reflectVec = reflect(-toLight, _normal);
                specularFactor = pow(max(dot(reflectVec, _toCamera), 0.0f), specular.a);
            }
            
            float spotFactor = theta <= lights[_nIndex].phi / 2 ? 1.f : lights[_nIndex].falloff;
            
            float attenuationFactor = 1.0f / dot(lights[_nIndex].attenuation, float3(1.0f, distance, distance * distance));
            color = (_color * (lights[_nIndex].diffuse * fDiffuseFactor * diffuse) + (lights[_nIndex].specular * specularFactor * specular)) * attenuationFactor * spotFactor;
        }
    }
    
    color = color * clamp((15 - length(_position - cameraPosition)) / 5.f, 0.f, 1.f);
    
    return color;
}
    
float CheckShadow(Texture2D<float> _shadowMap, int3 texLocation, float compareValue) {
    // 쉐도우 맵내 깊이를 구한다.
    float bias = 0.2f;

    float depth = _shadowMap.Load(texLocation);

    // 빛에서 정점까지의 거리가 쉐도우 맵에서의 거리보다 큰경우 그림자이다.
    //if (depth + bias < compareValue)
    if (depth + bias < compareValue)
    {
        return 0.f;
    }
    // 그림자가 아닐경우
    else
    {
        return 1.f;
    }
    
}

float GetShadowRate(float3 _Position, int _Index) {
    // 현재 빛 관점에서의 뷰, 투영 행렬
    float shadowCount = 0;
    float3 lightPos = lights[_Index].position;
    float4x4 lightView = lights[_Index].view;
    float4x4 lightProjection = lights[_Index].projection;

    // 현재 점의 쉐도우맵에서의 뷰포트좌표를 구한다.(-1, 1)
    float4 posLightAxis = mul(mul(float4(_Position, 1.0f), lightView), lightProjection);
    
    posLightAxis.xyz /= posLightAxis.www;
    float2 shadowMapUV = float2(posLightAxis.xy);

    // 텍스처 좌표계로 변환한다. (0~1)
    shadowMapUV.x = (shadowMapUV.x + 1.0f) / 2.0f;
    // 뷰포트는 아래가 -y방향, 텍스처는 아래가 +y (v) 방향 
    
    shadowMapUV.y = 1 - ((shadowMapUV.y + 1.0f) / 2.0f);
    
    // 텍스처의 바깥 좌표일경우 쉐도우맵 반경 바깥이므로 처리하지 않는다.
    if (shadowMapUV.x < 0 || shadowMapUV.x > 1 || shadowMapUV.y < 0 || shadowMapUV.y > 1)
        return 1;
    else
    {      
        Texture2D<float> shadowMap;
        // uv좌표를 텍셀좌표로 바꾼다. (Load함수를 쓰기 위함)
        int3 location = int3(shadowMapUV.x * CWIDTH, shadowMapUV.y * CHEIGHT, 0);
        
        // 빛과 정점의 거리를 구한다.
        float distance = length(lightPos - _Position);

        if (_Index == 0) 
            shadowMap = shadowMapTexture_1;
        else if (_Index == 1) 
            shadowMap = shadowMapTexture_2;
        else if (_Index == 2) 
            shadowMap = shadowMapTexture_3;
        else if (_Index == 3) 
            shadowMap = shadowMapTexture_4;
        else if (_Index == 4) 
            shadowMap = shadowMapTexture_5;
        else if (_Index == 5) 
            shadowMap = shadowMapTexture_6;
        else if (_Index == 6) 
            shadowMap = shadowMapTexture_7;
        else if (_Index == 7) 
            shadowMap = shadowMapTexture_8;
        else if (_Index == 8) 
            shadowMap = shadowMapTexture_9;
        else if (_Index == 9) 
            shadowMap = shadowMapTexture_10;

        shadowCount += CheckShadow(shadowMap, location, distance);
        // 각 방향으로 다시 검사를 하여 그림자의 세기를 정한다.
        [unroll(8)]
        for (int i = 0; i < NUM_SHADOW_MAP; ++i)
        {
            shadowCount += CheckShadow(shadowMap, location + d[i], distance);
        }
        return (shadowCount / (1 + NUM_SHADOW_MAP));
    }
}

float4 CalculateLight(float4 color, float3 _Position, float3 _Normal) {
    float alpha = color.a;
    float4 newColor = float4(0, 0, 0, alpha);
    float3 toCamera = normalize(cameraPosition - _Position);
    float lightPercent = 1;
	 //float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
	//color = float4(1.0f, 1.0f, 1.0f, 0.0f);
    //return float4(shadowMapTexture_1.Sample(gssWrap, _uv) / 20, 0, 0, 1);
    
	// 루프를 unroll하여 성능 향상. 
    [unroll(MAX_LIGHTS)]
        for (int i = 0; i < MAX_LIGHTS; ++i)
        {
            if (nLight <= i || (1 <= newColor.r && 1 <= newColor.g && 1 <= newColor.b))
                break;
            if (lights[i].enable)
            {
                // 그림자일 경우 가려지는 점이므로 건너뛴다.

                if (lights[i].lightType == DIRECTIONAL_LIGHT)
                {
                    newColor += DirectionalLight(i, _Normal, toCamera, color);
                }
                else if (lights[i].lightType == POINT_LIGHT)
                {
                    newColor += PointLight(i, _Position, _Normal, toCamera, color);
                }
                else if (lights[i].lightType == SPOT_LIGHT)
                {
                    if (i < NUM_SHADOW_MAP)
                    {
                        lightPercent = GetShadowRate(_Position, i);
                        if (lightPercent <= 0)
                            continue;
                    }
                    else
                        lightPercent = 1;
                
                    newColor += SpotLight(i, _Position, _Normal, toCamera, color) * lightPercent;

            }
        }
        }

    newColor += color * globalAmbient;
    return newColor;
}

