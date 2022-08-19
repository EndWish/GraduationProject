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

	float3 offset;	// ���� ���� ��ü�� �߽����κ��� ������ ��
	float theta; //cos(m_fTheta), ����Ʈ����Ʈ���� ���
	float3 attenuation;
	float phi; //cos(m_fPhi), ����Ʈ����Ʈ���� ���
	float3 direction;
	float padding;

	// �� ���� ���� �ִ� ������Ʈ�� ������ (�̻��)
	float4 object;

	// 1 = ��, 2 = ����Ʈ, 3 = ����
	int lightType;

	// �� ���� �����ִ� �������� Ȯ��
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

	// ������ unroll�Ͽ� ���� ���. max light��ŭ unroll�� ���̴� �����Ͻÿ� �ð��� �ʹ� �����ɷ� 10%�� Ÿ��
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

