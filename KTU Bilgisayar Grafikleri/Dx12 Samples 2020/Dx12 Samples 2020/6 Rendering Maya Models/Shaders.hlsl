Texture2D    textureMap		: register(t0);
SamplerState sampleLinear	: register(s0);

cbuffer SceneConstantBuffer : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	float3 LightPos;
	float3 EyePos;
}

struct VSOutput
{
	float4 position	 : SV_POSITION;
	float4 positionW : POSITION;
	float4 normal	 : NORMAL;
	float2 tex		 : TEXCOORD;
};

VSOutput VSMain(float3 position : POSITION, float3 normal : NORMAL, float2 tex : TEXCOORD)
{
	VSOutput result;

	result.position  = mul(float4(position, 1), World);
	result.position  = mul(result.position, View);
	result.position  = mul(result.position, Projection);
	result.positionW = mul(float4(position, 1), World);
	result.normal	 = mul(float4(normal, 1), World);
	result.tex       = tex;
	return result;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
	float4 ShapeColor = textureMap.Sample(sampleLinear, input.tex);
	//float4 ShapeColor = float4(1, 1, 1, 1); // for STL format

	float4 LightColor = float4(1, 1, 1, 1);

	// DIFFUSE COLOR
	float3 toLight			= normalize(LightPos - input.positionW);
	float dotEyeNorm		= max(dot(toLight, input.normal), 0);
	float4 diffuseColor		= dotEyeNorm * ShapeColor;

	// SPECULAR COLOR
	float3 fromLight		= normalize(input.positionW - LightPos);
	float3 toEye			= normalize(EyePos - input.positionW);
	float3 reflected		= fromLight - 2 * dot(fromLight, input.normal) * input.normal;
	float dotEyereflected	= max(dot(toEye, reflected), 0);
	float4 specularColor	= pow(dotEyereflected, 32.0f) * LightColor;

	return 0.3 * ShapeColor + 0.4 * diffuseColor + 0.3 * specularColor;
}