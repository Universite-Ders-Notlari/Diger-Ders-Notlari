//--------------------------------------------------------------------------------------
// File: Tutorial_6.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 vLightPos[2];
	float4 vLightColor[2];
	float4 vMeshColor;
	float4 vEyePos;
}
//--------------------------------------------------------------------------------------

struct VS_INPUT_PN
{
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
};

struct VS_INPUT_P
{
	float4 Pos : POSITION;
};

struct PS_INPUT_PN
{
    float4 PosH  : SV_POSITION;
	float3 PosW	: POSITION;
    float3 Norm : NORMAL;
};

struct PS_INPUT_P
{
	float4 Pos  : SV_POSITION;
};


//--------------------------------------------------------------------------------------
// Vertex Shader Position Normal
//--------------------------------------------------------------------------------------
PS_INPUT_PN VS_PositionNormal( VS_INPUT_PN input )
{
    PS_INPUT_PN output = (PS_INPUT_PN)0;
    output.PosH = mul( input.Pos, World );
    output.PosH = mul( output.PosH, View );
    output.PosH = mul( output.PosH, Projection );

    output.Norm = input.Norm;					// Zeminin normali degismediginde World ile carpmaya gerek yok	
	//output.Norm = mul(input.Norm, World);		// Zeminin normali degisirse World ile carpmak gerekir

	output.PosW	= mul( input.Pos, World );
    
    return output;
}

//--------------------------------------------------------------------------------------
// Vertex Shader Position
//--------------------------------------------------------------------------------------
PS_INPUT_P VS_Position(VS_INPUT_P input)
{
	PS_INPUT_P output = (PS_INPUT_P)0;
	output.Pos = mul(input.Pos, World);
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);

	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader Phong
//--------------------------------------------------------------------------------------
float4 PS_Phong( PS_INPUT_PN input) : SV_Target
{
	float4 finalColor = 0;    
    float3 diffuseColor = 0;    
    float3 specularColor = 0;    
	float3 fromLight = 0;  
    float3 toLight = 0;  
    float3 toEye = 0;

    for(int i=0; i<2; i++)
    {
		// DIFFUSE COLOR
		toLight				= normalize( vLightPos[i] - input.PosW );
		float dotEyeNorm	= dot( toLight, input.Norm ) ;
		diffuseColor		= max(dotEyeNorm * vMeshColor, 0) ; 

		// SPECULAR COLOR
		fromLight			= normalize( input.PosW - vLightPos[i] );	
		toEye				= normalize( vEyePos - input.PosW);
		//float3 reflected	= reflect( fromLight, input.Norm);
		float3 reflected	= fromLight - 2 * dot(fromLight, input.Norm) * input.Norm; 		
		float dotEyereflected = dot( toEye, reflected ) ;
		specularColor = max(pow(dotEyereflected, 32.0f) * vLightColor[i], 0);
		
		finalColor.rgb		+= min( 0.1F * vMeshColor + 0.4F * diffuseColor + 0.3F * specularColor, 1);
		//finalColor.rgb += min(0.0F * vMeshColor + 0.0F * diffuseColor + 0.5F * specularColor, 1);
    }
    
    finalColor.a = 1;   
    return finalColor;
}

//--------------------------------------------------------------------------------------
// Pixel Shader Solid
//--------------------------------------------------------------------------------------
float4 PS_Solid( PS_INPUT_P input) : SV_Target
{
    return vMeshColor;
}