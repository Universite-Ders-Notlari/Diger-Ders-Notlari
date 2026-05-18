
cbuffer SceneConstantBuffer : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
}

struct VSOutput
{
	float4 position : SV_POSITION;
	float4 color    : COLOR;
};

VSOutput VSMain(float3 position : POSITION, float4 color : COLOR)
{
	VSOutput result;

	result.position = mul(float4(position,1), World);
	result.position = mul(result.position, View);
	result.position = mul(result.position, Projection);

	result.color = color;

	return result;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
	return input.color;
}