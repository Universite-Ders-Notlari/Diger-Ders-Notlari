//--------------------------------------------------------------------------------------
// File: Tutorial07.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register( t0 );
SamplerState samLinear : register( s0 );

cbuffer cbNeverChanges : register( b0 )
{
    matrix World;
	matrix mworldInvTranspose;
	matrix View;
	matrix Projection;
	float4 vEyePos;
    float4 vLightPos[2];
	float4 vMeshColor;
	float4 vLightColor[2];
};

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
	float2 Tex : TEXCOORD0;
    float3 Norm : NORMAL;
};

struct PS_INPUT
{
    float4 PosH  : SV_POSITION;
	float3 PosW	: POSITION;
    float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.PosH = mul( input.Pos, World );
    output.PosH = mul( output.PosH, View );
    output.PosH = mul( output.PosH, Projection );
	
    output.Norm = mul( input.Norm, World );	
	//output.Norm = mul( input.Norm, (float3x3)mworldInvTranspose );	
	output.PosW	= mul( input.Pos, World );

	output.Tex	= input.Tex;
    
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
	float4 finalColor = 0;    
    float3 diffuseColor = 0;    
    float3 specularColor = 0;    
	float3 fromLight = 0;  
    float3 toLight = 0;  
    float3 toEye = 0;

    for(int i=0; i<2; i++)
    {
		//	DIFFUSE COLOR
		toLight				= normalize( vLightPos[i] - input.PosW );
		float dotEyeNorm	= dot( toLight, input.Norm ) ;
		if( dotEyeNorm > 0.0F) 
			diffuseColor	= dotEyeNorm * vMeshColor ; 
		else
			diffuseColor	= float3( 0, 0, 0 );

		// SPECULAR COLOR
		fromLight			= normalize( input.PosW - vLightPos[i] );	
		toEye				= normalize( vEyePos - input.PosW);

		float3 reflected	= reflect( fromLight, input.Norm);
		//float3 reflected	= fromLight - 2 * dot(fromLight, input.Norm) * input.Norm; 
		
		float dotEyereflected = dot( toEye, reflected ) ;
		if( dotEyereflected > 0.0F) 
			specularColor	= pow( dotEyereflected , 32.0f ) * vLightColor[i] ; 	
		else
			specularColor = float3( 0, 0, 0 );
				
		finalColor.rgb		+= saturate( 0.2F * vMeshColor + 0.6F * diffuseColor + 0.4F * specularColor ) ;
    }
    
    finalColor.a = 1;   

    return finalColor;
}

//--------------------------------------------------------------------------------------
// Pixel Shader Textured
//--------------------------------------------------------------------------------------
float4 PS_Textured( PS_INPUT input) : SV_Target
{
	float4 finalColor = 0;    
    float3 diffuseColor = 0;    
    float3 specularColor = 0;    
	float3 fromLight = 0;  
    float3 toLight = 0;  
    float3 toEye = 0;

    for(int i=0; i<2; i++)
    {
		//	DIFFUSE COLOR
		toLight				= normalize( vLightPos[i] - input.PosW );
		float dotEyeNorm	= dot( toLight, input.Norm ) ;
		if( dotEyeNorm > 0.0F) 
			diffuseColor	= dotEyeNorm * vMeshColor ; 
		else
			diffuseColor	= float3( 0, 0, 0 );

		// SPECULAR COLOR
		fromLight			= normalize( input.PosW - vLightPos[i] );	
		toEye				= normalize( vEyePos - input.PosW);

		//float3 reflected	= reflect( fromLight, input.Norm);
		float3 reflected	= fromLight - 2 * dot(fromLight, input.Norm) * input.Norm; 
		
		float dotEyereflected = dot( toEye, reflected ) ;
		if( dotEyereflected > 0.0F) 
			specularColor	= pow( dotEyereflected , 32.0f ) * vLightColor[i] ; 	
		else
			specularColor = float3( 0, 0, 0 );
				
		finalColor.rgb		+= saturate( 0.1F * vMeshColor + 0.4F * diffuseColor + 0.4F * specularColor ) ;
    }
    
    finalColor.a = 1;   

    return txDiffuse.Sample( samLinear, input.Tex ) * finalColor;
}


//--------------------------------------------------------------------------------------
// Pixel Shader Solid
//--------------------------------------------------------------------------------------
float4 PS_Solid( PS_INPUT input) : SV_Target
{
    return vMeshColor;
}
