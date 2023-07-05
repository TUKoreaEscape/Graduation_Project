struct MATERIAL
{
	float4					m_cAmbient;
	float4					m_cDiffuse;
	float4					m_cSpecular; //a = power
	float4					m_cEmissive;
};

cbuffer cbCameraInfo : register(b1)
{
	matrix					gmtxView : packoffset(c0);
	matrix					gmtxProjection : packoffset(c4);
	matrix					gmtxInverseProjection : packoffset(c8);
	float3					gvCameraPosition : packoffset(c12);
	float3					gf3CameraDirection : packoffset(c13);

};

cbuffer cbGameObjectInfo : register(b0)
{
	matrix					gmtxGameObject : packoffset(c0);
	MATERIAL				gMaterial : packoffset(c4);
	uint					gnTexturesMask : packoffset(c8.x);
	int						gnObjectType : packoffset(c8.y);
};

cbuffer cbDebug : register(b2)
{
	int4 gvDebugOptions : packoffset(c0);
};

#include "Light.hlsl"


matrix scaleMatrix = { 1.1f, 0.0f, 0.0f, 0.0f,
						0.0f, 1.1f, 0.0f, 0.0f,
						0.0f, 0.0f, 1.1f, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f };

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//#define _WITH_VERTEX_LIGHTING

#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

#define TYPE_OBJECT 0
#define TYPE_PLAYER 2;
#define TYPE_TAGGER 4;
#define TYPE_DEAD_PLAYER 8;

Texture2D gtxtAlbedoTexture : register(t6);
Texture2D gtxtSpecularTexture : register(t7);
Texture2D gtxtNormalTexture : register(t8);
Texture2D gtxtMetallicTexture : register(t9);
Texture2D gtxtEmissionTexture : register(t10);
Texture2D gtxtDetailAlbedoTexture : register(t11);
Texture2D gtxtDetailNormalTexture : register(t12);

SamplerState gssWrap : register(s0);

//SamplerState gssDefaultSamplerState : register(s0);//추가된 애

//Color, NormalW, Texture, Illumination, ObjectID+zDepth, NormalV, Depth 

struct VS_STANDARD_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct VS_STANDARD_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL0;
	float3 normalV : NORMAL1;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
	float2 uv : TEXCOORD;
};

struct PS_MULTIPLE_RENDER_TARGETS_OUTPUT
{
	float4 f4Scene : SV_TARGET0; //Swap Chain Back Buffer

	float4 f4Color : SV_TARGET1;
	float4 f4Normal : SV_TARGET2;
	float4 f4Texture : SV_TARGET3;
	float4 f4Illumination : SV_TARGET4;
	float2 f2ObjectIDzDepth : SV_TARGET5;
	float4 f4CameraNormal : SV_TARGET6;
};

VS_STANDARD_OUTPUT VSStandard(VS_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.normalV = mul(float4(output.normalW, 1.0f), gmtxView).xyz;
	output.tangentW = mul(input.tangent, (float3x3)gmtxGameObject);
	output.bitangentW = mul(input.bitangent, (float3x3)gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

VS_STANDARD_OUTPUT VSOutline(VS_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	matrix OutlineGameObject = gmtxGameObject;
	OutlineGameObject[0][0] *= 1.1f; 	OutlineGameObject[1][0] *= 1.1f; 	OutlineGameObject[2][0] *= 1.1f;
	OutlineGameObject[0][1] *= 1.1f; 	OutlineGameObject[1][1] *= 1.1f; 	OutlineGameObject[2][1] *= 1.1f;
	OutlineGameObject[0][2] *= 1.1f; 	OutlineGameObject[1][2] *= 1.1f; 	OutlineGameObject[2][2] *= 1.1f;
	OutlineGameObject[0][3] *= 1.1f; 	OutlineGameObject[1][3] *= 1.1f; 	OutlineGameObject[2][3] *= 1.1f;

	output.positionW = mul(float4(input.position, 1.0f), OutlineGameObject).xyz;
	output.normalW = mul(input.normal, (float3x3)OutlineGameObject);
	output.tangentW = mul(input.tangent, (float3x3)OutlineGameObject);
	output.bitangentW = mul(input.bitangent, (float3x3)OutlineGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSStandard(VS_STANDARD_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID)
{
	PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;

	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) {
		cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
		cAlbedoColor = cAlbedoColor * gMaterial.m_cDiffuse;
	}
	else cAlbedoColor = gMaterial.m_cDiffuse;
	float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) {
		cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
		cSpecularColor = cSpecularColor * gMaterial.m_cSpecular;
	}
	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);
	float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) {
		cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uv);
	}
	float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) {
		cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uv);
		cEmissionColor = cEmissionColor * gMaterial.m_cEmissive;
	}
	float4 cColor = cAlbedoColor + cSpecularColor + cMetallicColor + cEmissionColor;
	float3 normalW;

	if (gnTexturesMask & MATERIAL_NORMAL_MAP)
	{
		float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
		float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] → [-1, 1]
		normalW = normalize(mul(vNormal, TBN));
	}
	else
	{
		normalW = normalize(input.normalW);
	}
	float4 cIllumination = Lighting(input.positionW, normalW);
	output.f4Illumination = cIllumination;

	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	output.f4Texture = cAlbedoColor;

	output.f4Scene = output.f4Color = lerp(output.f4Illumination, output.f4Texture, 0.7f);

	output.f4Normal = float4(normalW.xyz * 0.5f + 0.5f, input.position.z);
	output.f4CameraNormal = float4(input.normalV.xyz * 0.5f + 0.5f, input.position.z);

	output.f2ObjectIDzDepth.x = (float)1.0f;
	output.f2ObjectIDzDepth.y = 1.0f - input.position.z;

	if (cColor.w < 0.524f)
		discard;

	output.f4Color.w = gnObjectType / 10.0f;

	return(output);
}

PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSAlpha(VS_STANDARD_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID)
{
	PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;

	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) {
		cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
		cAlbedoColor = cAlbedoColor * gMaterial.m_cDiffuse;
	}
	else cAlbedoColor = gMaterial.m_cDiffuse;
	float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) {
		cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
		cSpecularColor = cSpecularColor * gMaterial.m_cSpecular;
	}
	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);
	float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) {
		cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uv);
	}
	float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) {
		cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uv);
		cEmissionColor = cEmissionColor * gMaterial.m_cEmissive;
	}
	float4 cColor = cAlbedoColor + cSpecularColor + cMetallicColor + cEmissionColor;
	float3 normalW;
	if (gnTexturesMask & MATERIAL_NORMAL_MAP)
	{
		float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
		float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] → [-1, 1]
		normalW = normalize(mul(vNormal, TBN));
	}
	else
	{
		normalW = normalize(input.normalW);
	}
	float4 cIllumination = Lighting(input.positionW, normalW);

	output.f4Illumination = cIllumination;

	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	output.f4Texture = cAlbedoColor;

	output.f4Scene = output.f4Color = lerp(output.f4Illumination, output.f4Texture, 0.7f);

	output.f4Normal = float4(normalW.xyz * 0.5f + 0.5f, input.position.z);
	output.f4CameraNormal = float4(input.normalV.xyz * 0.5f + 0.5f, input.position.z);

	output.f2ObjectIDzDepth.x = (float)1.0f;
	output.f2ObjectIDzDepth.y = 1.0f - input.position.z;

	//if (cColor.w < 0.15f)
		//discard;
	clip(cColor.a - 0.15f);

	output.f4Color.w = gnObjectType / 10.0f;
	return(output);
}


float4 PSOutline(VS_STANDARD_OUTPUT input) : SV_TARGET
{
	return(float4(1.0f, 0.0f, 0.0f, 1.0f));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define MAX_VERTEX_INFLUENCES			4
#define SKINNED_ANIMATION_BONES			64

cbuffer cbBoneOffsets : register(b7)
{
	float4x4 gpmtxBoneOffsets[SKINNED_ANIMATION_BONES];
};

cbuffer cbBoneTransforms : register(b8)
{
	float4x4 gpmtxBoneTransforms[SKINNED_ANIMATION_BONES];
};

struct VS_SKINNED_STANDARD_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
	uint4 indices : BONEINDEX;
	float4 weights : BONEWEIGHT;
};

VS_STANDARD_OUTPUT VSSkinnedAnimationStandard(VS_SKINNED_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	float4x4 mtxVertexToBoneWorld = (float4x4)0.0f;
	for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
	{
		//		mtxVertexToBoneWorld += input.weights[i] * gpmtxBoneTransforms[input.indices[i]];
		mtxVertexToBoneWorld += input.weights[i] * mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
	}
	output.positionW = mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
	output.normalW = mul(input.normal, (float3x3)mtxVertexToBoneWorld).xyz;
	output.normalV = mul(float4(output.normalW, 1.0f), gmtxView).xyz;
	output.tangentW = mul(input.tangent, (float3x3)mtxVertexToBoneWorld).xyz;
	output.bitangentW = mul(input.bitangent, (float3x3)mtxVertexToBoneWorld).xyz;

	//	output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;

	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

VS_STANDARD_OUTPUT VSSkinnedAnimationOutline(VS_SKINNED_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	float4x4 mtxVertexToBoneWorld = (float4x4)0.0f;
	for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
	{
		//		mtxVertexToBoneWorld += input.weights[i] * gpmtxBoneTransforms[input.indices[i]];
		mtxVertexToBoneWorld += input.weights[i] * mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
	}

	matrix OutlineGameObject = mtxVertexToBoneWorld;
	OutlineGameObject[0][0] *= 1.1f; 	OutlineGameObject[1][0] *= 1.1f; 	OutlineGameObject[2][0] *= 1.1f;
	OutlineGameObject[0][1] *= 1.1f; 	OutlineGameObject[1][1] *= 1.1f; 	OutlineGameObject[2][1] *= 1.1f;
	OutlineGameObject[0][2] *= 1.1f; 	OutlineGameObject[1][2] *= 1.1f; 	OutlineGameObject[2][2] *= 1.1f;
	OutlineGameObject[0][3] *= 1.1f; 	OutlineGameObject[1][3] *= 1.1f; 	OutlineGameObject[2][3] *= 1.1f;

	output.positionW = mul(float4(input.position, 1.0f), OutlineGameObject).xyz;
	output.normalW = mul(input.normal, (float3x3)OutlineGameObject).xyz;
	output.tangentW = mul(input.tangent, (float3x3)OutlineGameObject).xyz;
	output.bitangentW = mul(input.bitangent, (float3x3)OutlineGameObject).xyz;

	//	output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;

	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
Texture2D gtxtTerrainBaseTexture : register(t1);
Texture2D gtxtTerrainDetailTexture : register(t2);

struct VS_TERRAIN_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

struct VS_TERRAIN_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;

	float3 positionW : POSITION;
	float3 normalW : NORMAL0;
	float3 normalV : NORMAL1;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
};

VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
	VS_TERRAIN_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.positionW = input.position;
	output.normalW = float3(0, 1, 0);
	output.normalV = float3(0, 1, 0);
	output.tangentW = float3(0, 0, 0);
	output.bitangentW = float3(0, 0, 0);
	output.color = input.color;
	output.uv0 = input.uv0;
	output.uv1 = input.uv1;

	return(output);
}

PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSTerrain(VS_TERRAIN_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID)
{
	PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;

	float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gssWrap, input.uv0);
	//float4 cDetailTexColor = gtxtTerrainDetailTexture.Sample(gssWrap, input.uv1);
	//	float4 cColor = saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));
	//float4 cColor = input.color * saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));

	float4 cColor = input.color * cBaseTexColor;

	float3 normalW = normalize(input.normalW);

	float4 cIllumination = Lighting(input.positionW, normalW);
	output.f4Illumination = cIllumination;

	//float3 uvw = float3(input.uv0, nPrimitiveID / 2);
	output.f4Texture = cColor;

	output.f4Scene = output.f4Color = cColor;

	output.f4Normal = float4(normalW.xyz * 0.5f + 0.5f, input.position.z);
	output.f4CameraNormal = float4(input.normalV.xyz * 0.5f + 0.5f, input.position.z);

	output.f2ObjectIDzDepth.x = (float)1.0f;
	output.f2ObjectIDzDepth.y = 1.0f - input.position.z;
	output.f4Color.w = gnObjectType / 10.0f;
	return(output);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_SKYBOX_CUBEMAP_INPUT
{
	float3 position : POSITION;
};

struct VS_SKYBOX_CUBEMAP_OUTPUT
{
	float3	positionL : POSITION;
	float4	position : SV_POSITION;

	float2 uv : TEXCOORD;
	float3 normalW : NORMAL0;
	float3 normalV : NORMAL1;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
};

VS_SKYBOX_CUBEMAP_OUTPUT VSSkyBox(VS_SKYBOX_CUBEMAP_INPUT input)
{
	VS_SKYBOX_CUBEMAP_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.positionL = input.position;
	output.uv = float2(0, 0);
	output.normalW = float3(0, 0, 0);
	output.normalV = float3(0, 0, 0);
	output.tangentW = float3(0, 0, 0);
	output.bitangentW = float3(0, 0, 0);

	return(output);
}

TextureCube gtxtSkyCubeTexture : register(t13);
SamplerState gssClamp : register(s1);

PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSSkyBox(VS_SKYBOX_CUBEMAP_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID)
{
	PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;

	float4 cColor = gtxtSkyCubeTexture.Sample(gssClamp, input.positionL);

	float3 normalW = normalize(input.normalW);

	float4 cIllumination = Lighting(input.positionL, normalW);
	output.f4Illumination = cIllumination;

	output.f4Texture = cColor;

	output.f4Scene = output.f4Color = cColor;

	output.f4Normal = float4(normalW.xyz * 0.5f + 0.5f, input.position.z);
	output.f4CameraNormal = float4(input.normalV.xyz * 0.5f + 0.5f, input.position.z);

	output.f2ObjectIDzDepth.x = (float)1.0f;
	output.f2ObjectIDzDepth.y = 1.0f - input.position.z;
	output.f4Color.w = gnObjectType / 10.0f;
	return(output);
}

struct VS_WALL_INPUT
{
	float3		position : POSITION;
	float3		normal : NORMAL;
	float2		uv : TEXTURECOORD;
};

struct VS_WALL_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL0;
	float3 normalV : NORMAL1;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
	float2 uv : TEXCOORD;
};

VS_WALL_OUTPUT VSWall(VS_WALL_INPUT input)
{
	VS_WALL_OUTPUT output;

	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.normalV = mul(float4(output.normalW, 1.0f), gmtxView).xyz;
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.tangentW = float3(0, 0, 0);
	output.bitangentW = float3(0, 0, 0);
	output.uv = input.uv;

	return(output);
}

PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSWall(VS_WALL_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID)
{
	PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;

	float4 cAlbedoColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
	float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);
	float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uv);
	float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uv);

	float4 cColor = cAlbedoColor * gMaterial.m_cDiffuse + cSpecularColor * gMaterial.m_cSpecular + cMetallicColor + cEmissionColor * gMaterial.m_cEmissive;

	float3 normalW = normalize(input.normalW);

	float4 cIllumination = Lighting(input.positionW, normalW);
	output.f4Illumination = cIllumination;

	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	output.f4Texture = cColor;

	output.f4Scene = output.f4Color = lerp(output.f4Illumination, output.f4Texture, 0.7f);

	output.f4Normal = float4(normalW.xyz * 0.5f + 0.5f, input.position.z);
	output.f4CameraNormal = float4(input.normalV.xyz * 0.5f + 0.5f, input.position.z);

	output.f2ObjectIDzDepth.x = (float)1.0f;
	output.f2ObjectIDzDepth.y = 1.0f - input.position.z;
	output.f4Color.w = gnObjectType / 10.0f;
	return(output);
}

struct VS_TEXTURED_LIGHTING_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct VS_TEXTURED_LIGHTING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL0;
	float3 normalV : NORMAL1;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
	float2 uv : TEXCOORD;
};

VS_TEXTURED_LIGHTING_OUTPUT VSTexturedLighting(VS_TEXTURED_LIGHTING_INPUT input)
{
	VS_TEXTURED_LIGHTING_OUTPUT output;

	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.normalV = mul(float4(output.normalW, 1.0f), gmtxView).xyz;
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.tangentW = mul(input.tangent, (float3x3)gmtxGameObject);
	output.bitangentW = mul(input.bitangent, (float3x3)gmtxGameObject);
	output.uv = input.uv;

	return(output);
}

float4 PSTexturedLighting(VS_TEXTURED_LIGHTING_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET0
{
	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	float4 cColor = gtxtAlbedoTexture.Sample(gssWrap, uvw);
	input.normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, input.normalW);
	//output.f4Color.w = (float)gnObjectType;
	return(cColor * cIllumination);
}



PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSTexturedLightingToMultipleRTs(VS_TEXTURED_LIGHTING_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID)
{
	PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;

	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) {
		cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
		cAlbedoColor = cAlbedoColor * gMaterial.m_cDiffuse;
	}
	else cAlbedoColor = gMaterial.m_cDiffuse;
	float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) {
		cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
		cSpecularColor = cSpecularColor * gMaterial.m_cSpecular;
	}
	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);
	float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) {
		cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uv);
	}
	float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) {
		cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uv);
		cEmissionColor = cEmissionColor * gMaterial.m_cEmissive;
	}
	float4 cColor = cAlbedoColor + cSpecularColor + cEmissionColor;
	
	if (gnObjectType < 0)
		clip(cColor.a - 0.1f);

	float3 normalW;
	if (gnTexturesMask & MATERIAL_NORMAL_MAP)
	{
		float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
		float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] → [-1, 1]
		normalW = normalize(mul(vNormal, TBN));
	}
	else
	{
		normalW = normalize(input.normalW);
	}

	float4 cIllumination = Lighting(input.positionW, normalW);
	//return(lerp(cColor, cIllumination, 0.5f));
	output.f4Illumination = cIllumination;

	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	//output.f4Texture = gtxtAlbedoTexture.Sample(gssWrap, uvw);// Texture2DArray gtxtTextureArray : register(t0); 정의해야함   또 원래는 gtxtTextureArray를 사용하고있었는데 임시로 알베도로 바꿔놓음
	output.f4Texture = cColor;
	input.normalW = normalize(input.normalW);
	//output.f4Illumination = Lighting(input.positionW, input.normalW);

	//output.f4Scene = output.f4Color = output.f4Illumination * output.f4Texture;
	output.f4Scene = output.f4Color = lerp(output.f4Illumination, output.f4Texture, 0.7f);
	//output.f4Normal = float4(input.normalW.xyz * 0.5f + 0.5f, input.position.z);
	//output.f4CameraNormal = float4(input.normalV.xyz * 0.5f + 0.5f, input.position.z);

	output.f4Normal = float4(normalW.xyz * 0.5f + 0.5f, input.position.z);
	output.f4CameraNormal = float4(input.normalV.xyz * 0.5f + 0.5f, input.position.z);

	output.f2ObjectIDzDepth.x = (float)1.0f;
	output.f2ObjectIDzDepth.y = 1.0f - input.position.z;
	//output.f4Scene = output.f4Texture;
	//output.f4Scene = float4(1,1,1,1);
	output.f4Color.w = gnObjectType / 10.0f;
	return(output);
}

struct VS_SCREEN_RECT_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
	float3 viewSpaceDir : TEXCOORD1;
	uint screenNum : NUMBER;
};

VS_SCREEN_RECT_TEXTURED_OUTPUT VSScreenRectSamplingTextured(uint nVertexID : SV_VertexID)
{
	VS_SCREEN_RECT_TEXTURED_OUTPUT output = (VS_SCREEN_RECT_TEXTURED_OUTPUT)0;

	if (gvDebugOptions.x == 10)
	{
		if (nVertexID == 0) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
		else if (nVertexID == 1) { output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 0.0f); }
		else if (nVertexID == 2) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
		else if (nVertexID == 3) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
		else if (nVertexID == 4) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
		else if (nVertexID == 5) { output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 1.0f); }
	}
	else
	{
		int screen = nVertexID / 6;
		int add = 0;
		if (nVertexID > 23)
		{
			add = 1;
		}
		if (nVertexID % 6 == 0) { output.position = float4((0.25 * screen) - 1.0f, +1.0f, 0.0f, 1.0f);		output.uv = float2(0.0f, 0.0f);		output.screenNum = screen + add; }
		else if (nVertexID % 6 == 1) { output.position = float4((0.25 * (screen + 1.0f)) - 1.0f, +1.0f, 0.0f, 1.0f);		output.uv = float2(1.0f, 0.0f);		output.screenNum = screen + add; }
		else if (nVertexID % 6 == 2) { output.position = float4((0.25 * (screen + 1.0f)) - 1.0f, +0.5f, 0.0f, 1.0f);		output.uv = float2(1.0f, 1.0f);		output.screenNum = screen + add; }
		else if (nVertexID % 6 == 3) { output.position = float4((0.25 * screen) - 1.0f, +1.0f, 0.0f, 1.0f);		output.uv = float2(0.0f, 0.0f);		output.screenNum = screen + add; }
		else if (nVertexID % 6 == 4) { output.position = float4((0.25 * (screen + 1.0f)) - 1.0f, +0.5f, 0.0f, 1.0f);		output.uv = float2(1.0f, 1.0f);		output.screenNum = screen + add; }
		else if (nVertexID % 6 == 5) { output.position = float4((0.25 * screen) - 1.0f, +0.5f, 0.0f, 1.0f);		output.uv = float2(0.0f, 1.0f);		output.screenNum = screen + add; }
	}
	output.viewSpaceDir = mul(output.position, gmtxInverseProjection).xyz;
	return(output);
}

float4 GetColorFromDepth(float fDepth)
{
	float4 cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	if (fDepth > 1.0f) cColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	else cColor = float4(fDepth * 1.2, fDepth * 1.2, fDepth * 1.2, 1.0f);
	return(cColor);
}

Texture2D gtxtInputTextures[7] : register(t14); //Color, NormalW, Texture, Illumination, ObjectID+zDepth, NormalV, Depth

static float gfLaplacians[9] = { -1.0f, -1.0f, -1.0f, -1.0f, 8.0f, -1.0f, -1.0f, -1.0f, -1.0f };
static int2 gnOffsets[9] = { { -1,-1 }, { 0,-1 }, { 1,-1 }, { -1,0 }, { 0,0 }, { 1,0 }, { -1,1 }, { 0,1 }, { 1,1 } };

float4 LaplacianEdge(float4 position)
{
	float fObjectEdgeness = 0.0f, fNormalEdgeness = 0.0f, fDepthEdgeness = 0.0f;
	float3 f3NormalEdgeness = float3(0.0f, 0.0f, 0.0f), f3DepthEdgeness = float3(0.0f, 0.0f, 0.0f);
	if ((uint(position.x) >= 1) || (uint(position.y) >= 1) || (uint(position.x) <= gtxtInputTextures[0].Length.x - 2) || (uint(position.y) <= gtxtInputTextures[0].Length.y - 2))
	{
		float fObjectID = gtxtInputTextures[4][int2(position.xy)].r;
		for (int input = 0; input < 9; input++)
		{
			//			if (fObjectID != gtxtInputTextures[4][int2(position.xy) + gnOffsets[input]].r) fObjectEdgeness = 1.0f;

			float3 f3Normal = gtxtInputTextures[1][int2(position.xy) + gnOffsets[input]].xyz * 2.0f - 1.0f;
			float3 f3Depth = gtxtInputTextures[6][int2(position.xy) + gnOffsets[input]].xyz * 2.0f - 1.0f;
			f3NormalEdgeness += gfLaplacians[input] * f3Normal;
			f3DepthEdgeness += gfLaplacians[input] * f3Depth;
		}
		fNormalEdgeness = f3NormalEdgeness.r * 0.3f + f3NormalEdgeness.g * 0.59f + f3NormalEdgeness.b * 0.11f;
		fDepthEdgeness = f3DepthEdgeness.r * 0.3f + f3DepthEdgeness.g * 0.59f + f3DepthEdgeness.b * 0.11f;
	}
	float3 cColor = gtxtInputTextures[0][int2(position.xy)].rgb;
	/*
		float fNdotV = 1.0f - dot(gtxtInputTextures[1][int2(position.xy)].xyz * 2.0f - 1.0f, gf3CameraDirection);
		float fNormalThreshold = (saturate((fNdotV - 0.5f) / (1.0f - 0.5f)) * 7.0f) + 1.0f;
		float fDepthThreshold = 150.0f * gtxtInputTextures[6][int2(position.xy)].r * fNormalThreshold;
	*/
	if (fObjectEdgeness == 1.0f)
		cColor = float3(1.0f, 0.0f, 0.0f);
	else
	{
		cColor.g += fNormalEdgeness;
		cColor.r += fDepthEdgeness;
		//		cColor.g += (fNormalEdgeness > fNormalThreshold) ? 1.0f : 0.0f;
		//		cColor.r += (fDepthEdgeness > fDepthThreshold) ? 1.0f : 0.0f;
	}
	return(float4(cColor, 0.1f));
}

float4 AlphaBlend(float4 top, float4 bottom)
{
	float3 color = (top.rgb * top.a) + (bottom.rgb * (1 - top.a));
	float alpha = top.a + bottom.a * (1 - top.a);

	return(float4(color, alpha));
}

float4 Outline(VS_SCREEN_RECT_TEXTURED_OUTPUT input)
{
	float fHalfScaleFloor = floor(1.0f * 0.5f);
	float fHalfScaleCeil = ceil(1.0f * 0.5f);

	float2 f2BottomLeftUV = input.uv - float2((1.0f / gtxtInputTextures[0].Length.x), (1.0f / gtxtInputTextures[0].Length.y)) * fHalfScaleFloor;
	float2 f2TopRightUV = input.uv + float2((1.0f / gtxtInputTextures[0].Length.x), (1.0f / gtxtInputTextures[0].Length.y)) * fHalfScaleCeil;
	float2 f2BottomRightUV = input.uv + float2((1.0f / gtxtInputTextures[0].Length.x) * fHalfScaleCeil, -(1.0f / gtxtInputTextures[0].Length.y * fHalfScaleFloor));
	float2 f2TopLeftUV = input.uv + float2(-(1.0f / gtxtInputTextures[0].Length.x) * fHalfScaleFloor, (1.0f / gtxtInputTextures[0].Length.y) * fHalfScaleCeil);

	float3 f3NormalV0 = gtxtInputTextures[5].Sample(gssWrap, f2BottomLeftUV).rgb;
	float3 f3NormalV1 = gtxtInputTextures[5].Sample(gssWrap, f2TopRightUV).rgb;
	float3 f3NormalV2 = gtxtInputTextures[5].Sample(gssWrap, f2BottomRightUV).rgb;
	float3 f3NormalV3 = gtxtInputTextures[5].Sample(gssWrap, f2TopLeftUV).rgb;

	float fDepth0 = gtxtInputTextures[6].Sample(gssWrap, f2BottomLeftUV).r;
	float fDepth1 = gtxtInputTextures[6].Sample(gssWrap, f2TopRightUV).r;
	float fDepth2 = gtxtInputTextures[6].Sample(gssWrap, f2BottomRightUV).r;
	float fDepth3 = gtxtInputTextures[6].Sample(gssWrap, f2TopLeftUV).r;

	float3 f3NormalV = f3NormalV0 * 2.0f - 1.0f;
	float fNdotV = 1.0f - dot(f3NormalV, -input.viewSpaceDir);

	float fNormalThreshold01 = saturate((fNdotV - 0.5f) / (1.0f - 0.5f));
	float fNormalThreshold = (fNormalThreshold01 * 7.0f) + 1.0f;

	float fDepthThreshold = 1.5f * fDepth0 * fNormalThreshold;

	float fDepthDifference0 = fDepth1 - fDepth0;
	float fDepthDifference1 = fDepth3 - fDepth2;
	float fDdgeDepth = sqrt(pow(fDepthDifference0, 2) + pow(fDepthDifference1, 2)) * 100.0f;
	fDdgeDepth = (fDdgeDepth > 1.5f) ? 1.0f : 0.0f;

	float3 fNormalDifference0 = f3NormalV1 - f3NormalV0;
	float3 fNormalDifference1 = f3NormalV3 - f3NormalV2;
	float fEdgeNormal = sqrt(dot(fNormalDifference0, fNormalDifference0) + dot(fNormalDifference1, fNormalDifference1));
	fEdgeNormal = (fEdgeNormal > 0.4f) ? 1.0f : 0.0f;

	float fEdge = max(fDdgeDepth, fEdgeNormal);
	float k = gtxtInputTextures[0][int2(input.position.xy)].w;
	float4 myColor = float4 (0, 0, 0, 1);
	if (k < 0.05f)
		myColor = float4(0, 0, 0, 1);
	else if (k < 0.3f)
		myColor = float4(0, 1, 0, 1);
	else if (k < 0.5f)
		myColor = float4(1, 0, 0, 1);
	else
		myColor = float4(0.2, 0.2, 0.2, 1);
	float4 f4EdgeColor = float4(myColor.xyz, myColor.w * fEdge);
	//float4 f4EdgeColor = float4(1.0f, 0,0,1.0f * fEdge);
	float4 f4Color = gtxtInputTextures[0].Sample(gssWrap, input.uv);
	return(AlphaBlend(f4EdgeColor, f4Color));
}

float4 PSScreenRectSamplingTextured(VS_SCREEN_RECT_TEXTURED_OUTPUT input) : SV_Target
{
	float4 cColor = float4(1.0f, 1.0f, 0.0f, 0.1f);

	if (gvDebugOptions.x == 10)
	{
		cColor = Outline(input);
	}
	else
	{
		if (input.screenNum == 6)
		{
			float fDepth = gtxtInputTextures[6].Load(uint3((uint)input.position.x, (uint)input.position.y, 0)).r;
			cColor = GetColorFromDepth(1.0f - fDepth);
		}
		else cColor = gtxtInputTextures[input.screenNum][int2(input.position.xy)];

	}
	//cColor = gtxtInputTextures[3].Sample(gssWrap, input.uv);
	//cColor = gtxtInputTextures[input.screenNum].Sample(gssWrap, input.uv);
	//cColor = LaplacianEdge(input.position);
	return(cColor);
}

float4 VSPostProcessing(uint nVertexID : SV_VertexID) : SV_POSITION
{
	if (nVertexID == 0) return(float4(-1.0f, +1.0f, 0.0f, 1.0f));
	if (nVertexID == 1) return(float4(+1.0f, +1.0f, 0.0f, 1.0f));
	if (nVertexID == 2) return(float4(+1.0f, -1.0f, 0.0f, 1.0f));

	if (nVertexID == 3) return(float4(-1.0f, +1.0f, 0.0f, 1.0f));
	if (nVertexID == 4) return(float4(+1.0f, -1.0f, 0.0f, 1.0f));
	if (nVertexID == 5) return(float4(-1.0f, -1.0f, 0.0f, 1.0f));

	return(float4(0, 0, 0, 0));
}

float4 PSPostProcessing(float4 position : SV_POSITION) : SV_Target
{
	return(float4(0.0f, 0.0f, 0.0f, 1.0f));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
Texture2D gtxtUITexture : register(t21);

struct VS_UI_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
};

struct VS_UI_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_UI_OUTPUT VSUI(VS_UI_INPUT input, uint nVertexID : SV_VertexID)
{
	VS_UI_OUTPUT output;
	output.position = float4(input.position,1.0f);
	output.uv = input.uv;
	//if (nVertexID == 0) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	//else if (nVertexID == 1) { output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 0.0f); }
	//else if (nVertexID == 2) { output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 1.0f); }
	//else if (nVertexID == 3) { output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 1.0f); }
	//else if (nVertexID == 4) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	//else if (nVertexID == 5) { output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 0.0f); }

	//output.position.xy *= float2(0.5f, 0.5f); // 위치를 화면 중심 기준으로 이동
	//output.position.xy -= float2(0.1f, 0.1f); // 위치를 [0, 1] 범위로 조정
	//output.position.xy *= float2(ScreenWidth, ScreenHeight); // 위치를 스크린 크기에 맞게 스케일 조정

	return output;
}

float4 PSUI(VS_UI_OUTPUT input) : SV_TARGET
{
	float4 Color = gtxtUITexture.Sample(gssWrap, input.uv);
	return Color;
}

cbuffer cbGaugeInfo : register(b3)
{
	float gfGauge : packoffset(c0.x);
	uint gnUIType : packoffset(c0.y);
};

#define DOOR_UI 1
#define VENT_UI 2
#define BOX_UI 3
#define POWER_UI 4

VS_UI_OUTPUT VSDoorUI(VS_UI_INPUT input)
{
	VS_UI_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return output;
}

float4 PSDoorUI(VS_UI_OUTPUT input) : SV_TARGET
{
	float4 Color = gtxtUITexture.Sample(gssWrap, input.uv);
	if (gnUIType == DOOR_UI) {
		if (input.uv.y - 1.0f > -gfGauge && Color.w < 0.1f) 
			Color = float4(0.0f, 168.0f / 255.0f, 243.0f / 255.0f, 1.0f);
	}
	else if (gnUIType == VENT_UI) {
		if (input.uv.y < 1.0f - gfGauge && input.uv.y > 0.2f) {
				Color = float4(0.2f, 0.2f, 0.2f, 1.0f);
		}
	}
	else if (gnUIType == BOX_UI) {
		if (input.uv.y - 1.0f > -gfGauge && Color.w < 0.1f)
			Color = float4(0.0f, 243.0f / 255.0f, 168.0f / 255.0f, 1.0f);
	}
	clip(Color.w - 0.1f);
	return Color;
}