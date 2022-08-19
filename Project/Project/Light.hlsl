#define MAX_LIGHTS			100


#define POINT_LIGHT			1
#define SPOT_LIGHT			2
#define DIRECTIONAL_LIGHT	3

struct LIGHT
{
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
	float padding;

	// 이 빛을 내고 있는 오브젝트의 포인터 (미사용)
	float4 object;

	// 1 = 점, 2 = 스포트, 3 = 직접
	int lightType;

	// 이 빛이 켜져있는 상태인지 확인
	bool enable;
	float2 padding2;
};

cbuffer cbLightInfo : register(b3) {
	LIGHT lights[MAX_LIGHTS];
	float4 globalAmbient;
	int	nLight;
}

float4 DirectionalLight(int _nIndex, float3 _normal, float3 _toCamera)
{
	float4 matAmbient = float4(0.0f, 1.0f, 0.0f, 1.0f);
	float4 matDiffuse = float4(0.0f, 0.0f, 1.0f, 1.0f); 
	float4 matSpecular = float4(1.0f, 0.0f, 0.0f, 10000.0f);

	float3 toLight = -lights[_nIndex].direction;
	float diffuseFactor = dot(toLight, _normal);
	float specularFactor = 0.0f;
	if (diffuseFactor > 0.0f)
	{
		float3 reflectVector = reflect(-toLight, _normal);
		specularFactor = pow(max(dot(reflectVector, _toCamera), 0.0f), matSpecular.a);
	}
	
	return((lights[_nIndex].ambient * matAmbient) + (lights[_nIndex].diffuse * diffuseFactor * matDiffuse) + (lights[_nIndex].specular * specularFactor * matSpecular));
}


float4 CalculateLight(float3 _Position, float3 _Normal) {
	
	float3 toCamera = normalize(cameraPosition - _Position);
	float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// 루프를 unroll하여 성능 향상. max light만큼 unroll시 셰이더 컴파일시에 시간이 너무 오래걸려 10%로 타협
	[unroll(MAX_LIGHTS / 10)] for (int i = 0; i < nLight; i++)
	{
		if (lights[i].enable)
		{
			if (lights[i].lightType == DIRECTIONAL_LIGHT)
			{
				//color = float4(1.0f, 1.0f, 1.0f, 0.0f);
				color += DirectionalLight(i, _Normal, toCamera);
			}
		/*	else if (gLights[i].lightType == POINT_LIGHT)
			{
				color += PointLight(i, vPosition, vNormal, vToCamera);
			}*/
			/*else if (gLights[i].lightType == SPOT_LIGHT)
			{
				color += SpotLight(i, vPosition, vNormal, vToCamera);
			}*/
		}
	}
	//color += (gcGlobalAmbientLight * gMaterial.m_cAmbient);
	color += globalAmbient;
	//color.a = gMaterial.m_cDiffuse.a;
	return color;
}

