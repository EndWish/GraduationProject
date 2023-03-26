
#define CWIDTH 600
#define CHEIGHT 400
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

// ���÷�
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
Texture2D<float> depthTexture : register(t11);

Texture2D<float> shadowMapTexture_1 : register(t12);
Texture2D<float> shadowMapTexture_2 : register(t13);
Texture2D<float> shadowMapTexture_3 : register(t14);
Texture2D<float> shadowMapTexture_4 : register(t15);


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

static const float2 d[4] = { float2(0, -2 / CHEIGHT), float2(-2 / CWIDTH, 0), float2(2 / CWIDTH, 0), float2(0, 2 / CHEIGHT) };
    
float CheckShadow(Texture2D<float> _shadowMap, float2 uv, float compareValue) {
    // ������ �ʳ� ���̸� ���Ѵ�.
    float bias = 0.01f;
    float depth = _shadowMap.Sample(gssWrap, uv);

    // ������ ���������� �Ÿ��� ������ �ʿ����� �Ÿ����� ū��� �׸����̴�.
    if (depth + bias < compareValue)
    //if (depth  < 2.0f)
    {
        return 0.f;
    }
    // �׸��ڰ� �ƴҰ��
    else
    {
        return 1.f;
    }
    
}

float GetShadowRate(float3 _Position, int _Index) {
    // ���� �� ���������� ��, ���� ���
    float shadowCount = 0;
    float3 lightPos = lights[_Index].position;
    float4x4 lightView = lights[_Index].view;
    float4x4 lightProjection = lights[_Index].projection;

    // ���� ���� ������ʿ����� ����Ʈ��ǥ�� ���Ѵ�.(-1, 1)
    float4 posLightAxis = mul(mul(float4(_Position, 1.0f), lightView), lightProjection);
    
    posLightAxis.xyz /= posLightAxis.www;
    float2 shadowMapUV = float2(posLightAxis.xy);

    // �ؽ�ó ��ǥ��� ��ȯ�Ѵ�. (0~1)
    shadowMapUV.x = (shadowMapUV.x + 1.0f) / 2.0f;
    // ����Ʈ�� �Ʒ��� -y����, �ؽ�ó�� �Ʒ��� +y (v) ���� 
    
    shadowMapUV.y = 1 - ((shadowMapUV.y + 1.0f) / 2.0f);
    
    // �ؽ�ó�� �ٱ� ��ǥ�ϰ�� ������� �ݰ� �ٱ��̹Ƿ� ó������ �ʴ´�.
    if (shadowMapUV.x < 0 || shadowMapUV.x > 1 || shadowMapUV.y < 0 || shadowMapUV.y > 1)
        return 5000;
    else
    {      
        Texture2D<float> shadowMap;
        
        // ���� ������ �Ÿ��� ���Ѵ�.
        float distance = length(lightPos - _Position);

        if (_Index == 0) 
            shadowMap = shadowMapTexture_1;
        else if (_Index == 1) 
            shadowMap = shadowMapTexture_2;
        else if (_Index == 2) 
            shadowMap = shadowMapTexture_3;
        else
            shadowMap = shadowMapTexture_4;
        
        shadowCount += CheckShadow(shadowMap, shadowMapUV, distance);
        
      
        return (shadowCount);
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
    
	// ������ unroll�Ͽ� ���� ���. 
    [unroll(MAX_LIGHTS)]
        for (int i = 0; i < MAX_LIGHTS; ++i)
        {
            if (nLight <= i || (1 <= newColor.r && 1 <= newColor.g && 1 <= newColor.b))
                break;
            if (lights[i].enable)
            {
                // �׸����� ��� �������� ���̹Ƿ� �ǳʶڴ�.

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
                    if (i <= 0)
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

