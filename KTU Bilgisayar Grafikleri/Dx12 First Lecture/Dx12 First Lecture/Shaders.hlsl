
cbuffer SceneConstantBuffer : register(b6)
{
	matrix World;
	float4 color;
}

cbuffer SceneConstantBuffer : register(b1)
{
	matrix View;
	matrix Projection;
}

struct VSOutput
{
	float4 position : SV_POSITION;
	//float4 color    : COLOR;
};

VSOutput VSMain(float3 position : POSITION)
{
	VSOutput result;

	result.position = mul(float4(position, 1), World);
	result.position = mul(result.position, View);
	result.position = mul(result.position, Projection);

	//result.color = color;

	return result;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
	return color;
}