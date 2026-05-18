Texture2D    textureMap		: register(t0);
SamplerState sampleLinear	: register(s0);

cbuffer SceneConstantBuffer : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 LightPos;
	float4 LightColor;
	float4 EyePos;
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

	result.position		= mul(float4(position, 1), World);
	result.position		= mul(result.position, View);
	result.position		= mul(result.position, Projection);
	result.positionW	= mul(position, World);
	result.normal		= mul(normal, World);
	result.tex			= tex;
	return result;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
	float4 ShapeColor = textureMap.Sample(sampleLinear, input.tex);

	// DIFFUSE COLOR
	float3 toLight			= normalize(LightPos - input.positionW.xyz);
	float dotEyeNorm		= dot(toLight, input.normal.xyz);
	float4 diffuseColor		= max(dotEyeNorm * ShapeColor, 0);

	// SPECULAR COLOR
	float3 fromLight		= normalize(input.positionW.xyz - LightPos);
	float3 toEye			= normalize(EyePos - input.positionW.xyz);
	float3 reflected		= fromLight - 2 * dot(fromLight, input.normal.xyz) * input.normal.xyz;
	float dotEyereflected	= dot(toEye, reflected);
	float4 specularColor	= max(pow(dotEyereflected, 32.0f) * LightColor, 0);

	return 1.0 * ShapeColor ; //+ 0.4 * diffuseColor + 0.2 * specularColor;
}