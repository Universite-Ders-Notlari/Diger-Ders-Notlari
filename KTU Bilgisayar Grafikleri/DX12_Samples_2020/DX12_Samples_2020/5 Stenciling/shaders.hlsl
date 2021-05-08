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
	float4 MeshColor;
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

float4 PS_Textured(VSOutput input) : SV_TARGET
{
	float4 ShapeColor = textureMap.Sample(sampleLinear, input.tex);

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

	return 0.2 * ShapeColor + 0.5 * diffuseColor + 0.3 * specularColor;
}

float4 PS_Phong(VSOutput input) : SV_TARGET
{
	// DIFFUSE COLOR
	float3 toLight			= normalize(LightPos - input.positionW);
	float dotEyeNorm		= dot(toLight, input.normal);
	float4 diffuseColor		= max(dotEyeNorm * MeshColor, 0);

	// SPECULAR COLOR
	float3 fromLight		= normalize(input.positionW - LightPos);
	float3 toEye			= normalize(EyePos - input.positionW);
	float3 reflected		= fromLight - 2 * dot(fromLight, input.normal) * input.normal;
	float dotEyereflected	= dot(toEye, reflected);
	float4 specularColor	= max(pow(dotEyereflected, 32.0f) * LightColor, 0);

	return 0.2 * MeshColor + 0.5 * diffuseColor + 0.3 * specularColor;
}

float4 PS_Solid(VSOutput input) : SV_TARGET
{
	return MeshColor;
}