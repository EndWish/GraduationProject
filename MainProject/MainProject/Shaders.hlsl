//게임 객체의 정보를 위한 상수 버퍼를 선언한다. 
cbuffer cbGameObjectInfo : register(b0) {
	matrix gmtxWorld : packoffset(c0);
};

//카메라의 정보를 위한 상수 버퍼를 선언한다. 
cbuffer cbCameraInfo : register(b1) {
	matrix gmtxView : packoffset(c0);
	matrix gmtxProjection : packoffset(c4);
}

//정점 셰이더를 정의한다.
void VSPrototype(float3 input : POSITION, out float4 output : POSITION) 
{
	//정점을 변환(월드 변환, 카메라 변환, 투영 변환)한다.
	output = mul(mul(mul(float4(input, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
}

//픽셀 셰이더를 정의한다.
float4 PSPrototype(float4 input : POSITION) : SV_TARGET
{
	//입력되는 픽셀의 색상을 그대로 출력-병합 단계(렌더 타겟)로 출력한다. 
	return input;
}