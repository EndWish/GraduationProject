

#define MAX_LIGHTS			20


#define POINT_LIGHT			1
#define SPOT_LIGHT			2
#define DIRECTIONAL_LIGHT	3

#define EPSILON				1.0e-5

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


float4 CalculateLight(float4 color, float3 _Position, float3 _Normal) {
    float alpha = color.a;
    float4 newColor = float4(0, 0, 0, alpha);
    float3 toCamera = normalize(cameraPosition - _Position);
	 //float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
	//color = float4(1.0f, 1.0f, 1.0f, 0.0f);
	
	// 루프를 unroll하여 성능 향상. max light만큼 unroll시 셰이더 컴파일시에 시간이 너무 오래걸려 10%로 타협

    [unroll(MAX_LIGHTS)]
    for (int i = 0; i < MAX_LIGHTS; ++i)
    {
        if (nLight <= i || (1 <= newColor.r && 1 <= newColor.g && 1 <= newColor.b))
            break;
        if (lights[i].enable) {
            if (lights[i].lightType == DIRECTIONAL_LIGHT) {
            newColor += DirectionalLight(i, _Normal, toCamera, color);
            }
            else if (lights[i].lightType == POINT_LIGHT) {
            newColor += PointLight(i, _Position, _Normal, toCamera, color);
            }
            else if (lights[i].lightType == SPOT_LIGHT) {
            newColor += SpotLight(i, _Position, _Normal, toCamera, color);
            }
        }
    }

    newColor += color * globalAmbient;
    return newColor;
}

