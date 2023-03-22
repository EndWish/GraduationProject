

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

	float3 offset;	// ���� ���� ��ü�� �߽����κ��� ������ ��
	float theta; //cos(m_fTheta), ����Ʈ����Ʈ���� ���
	float3 attenuation;
	float phi; //cos(m_fPhi), ����Ʈ����Ʈ���� ���
	float3 direction;
    float falloff;

	// �� ���� ���� �ִ� ������Ʈ�� ������ (�̻��)
	float4 object;

	// 1 = ��, 2 = ����Ʈ, 3 = ����
	int lightType;

	// �� ���� �����ִ� �������� Ȯ��
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

	// ���� ����
	float3 toLight = -lights[_nIndex].direction;
	
	// ���� ����� ������ �������� ���� ���
	float diffuseFactor = dot(toLight, _normal);
	float specularFactor = 0.0f;
	if (diffuseFactor > EPSILON) {
		// �ݻ纤�͸� ���� �ü����Ϳ� �����Ͽ� ���� �� ���
		float3 reflectVector = reflect(-toLight, _normal);
		specularFactor = pow(max(dot(reflectVector, _toCamera), 0.0f), specular.a);
	}

    return (_color * (lights[_nIndex].diffuse * max(diffuseFactor, 0) * diffuse) + (lights[_nIndex].specular * specularFactor * specular));
}


float4 PointLight(int _nIndex, float3 _position, float3 _normal, float3 _toCamera, float4 _color)
{
    float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
	// ���� �������� ���ͷ� �Ÿ� ���
	float3 toLight = lights[_nIndex].position - _position;
	float distance = length(toLight);
	
	// ������ range������ ����� ���
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
		// 1/(x+y*d+z*d*d). distance = 0�� ��� 1/x
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
	
    // �Ÿ��ȿ� �ִ��� Ȯ��
    if (distance <= lights[_nIndex].range)
    {
        float theta = acos(dot(lights[_nIndex].direction, -toLight)); // ������ ����� ������ ������Ʈ�� ���� ������ ���� (����)
        if (theta <= lights[_nIndex].theta / 2)  // ������ ���� ���� ���
        {
            /// �ܺο����ʿ� ���� ���
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
	
	// ������ unroll�Ͽ� ���� ���. max light��ŭ unroll�� ���̴� �����Ͻÿ� �ð��� �ʹ� �����ɷ� 10%�� Ÿ��

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

