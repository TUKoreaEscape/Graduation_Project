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
    matrix					gmtxInverseView : packoffset(c12);
	float3					gvCameraPosition : packoffset(c16);
	float3					gf3CameraDirection : packoffset(c17);

};

cbuffer cbGameObjectInfo : register(b0)
{
	matrix					gmtxGameObject : packoffset(c0);
	MATERIAL				gMaterial : packoffset(c4);
	uint					gnTexturesMask : packoffset(c8.x);
	int						gnObjectType : packoffset(c8.y);
};

cbuffer cbFrameworkInfo : register(b9)
{
    float gfCurrentTime : packoffset(c0.x);
    float gfElapsedTime : packoffset(c0.y);
    float gfSecondsPerFirework : packoffset(c0.z);
    int gnFlareParticlesToEmit : packoffset(c0.w);
	float3 gf3Gravity : packoffset(c1.x);
	int gnMaxFlareType2Particles : packoffset(c1.w);
};

#include "Light.hlsl"

struct CB_TOOBJECTSPACE
{
	matrix mtxToTexture;
	float4 f4Position;
};

cbuffer cbToLightSpace : register(b6)
{
	CB_TOOBJECTSPACE gcbToLightSpaces[MAX_LIGHTS];
};

Texture2D gtxtInputTextures[6] : register(t14); //Color, NormalW, Texture, Position, NormalV, Depth // 3과 4는 Illumination, ObjectID+zDepth 인데 이거 두개는 쓸모없어서 삭제함

static float gfLaplacians[9] = { -1.0f, -1.0f, -1.0f, -1.0f, 8.0f, -1.0f, -1.0f, -1.0f, -1.0f };
static int2 gnOffsets[9] = { { -1,-1 }, { 0,-1 }, { 1,-1 }, { -1,0 }, { 0,0 }, { 1,0 }, { -1,1 }, { 0,1 }, { 1,1 } };

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

float gSampleRadius = 0.5f;
float gIntensity = 1.0f;
float gScale = 1.0f;
float gBias = 0.025f;

struct SSAO_PS_INPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};

struct SSAO_PS_OUTPUT
{
	float4 color : COLOR0;
};

//float3 getPosition(float2 uv)
//{
//	return gtxtInputTextures[3].Sample(gssWrap, uv).xyz;
//}

//float3 getNormal(float2 uv)
//{
//	return normalize(gtxtInputTextures[1].Sample(gssWrap, uv).xyz * 2.0f - 1.0f);
//}

float3 getPosition(in float2 uv)
{
	return float3(uv, gtxtInputTextures[5][int2(uv)].r);
}

float3 getNormal(in float2 uv)
{
	return normalize(gtxtInputTextures[1][int2(uv)] * 2.0f - 1.0f);
}

float3 randomNormal(float2 tex)
{
	float noiseX = (frac(sin(dot(tex, float2(12.9898f, 78.233f) * 1.0f)) * 43758.5453f));
	float noiseY = (frac(sin(dot(tex, float2(94.3225f, 22.817f) * 2.0f)) * 23574.3521f));
	float noiseZ = (frac(sin(dot(tex, float2(39.129f, 10.311f) * 3.0f)) * 93878.1986f));
	return normalize(float3(noiseX, noiseY, noiseZ));
}

float2 getRandom(in float2 uv)
{
	float gSampleRadius = 0.01f;
	return normalize(randomNormal(uv) * 2.0f - 1.0f) * gSampleRadius;
}

float doAmbientOcclusion(in float2 tcoord, in float2 uv, in float3 p, in float3 cnorm)
{

	float gSampleRadius = 0.05f;
	float gIntensity = 0.07f;
	float gScale = 0.6f;
	float gBias = 0.005f;

	float3 diff = getPosition(tcoord + uv) - p;
	float3 v = normalize(diff);
	float d = length(diff) * gScale;
	return max(0.0f, dot(cnorm, v) - gBias) * (1.0f / (1.0f + d)) * gIntensity;
}

//밝게
float4 Adjust_Brightness(float4 vColor, float fBrightness)
{
	return float4(vColor.rgb += fBrightness, vColor.a);
}
// 대비
float4 Adjust_Contrast(float4 vColor, float fContrast)
{
	float t = 0.5f - fContrast * 0.5f;
	return float4(vColor.rgb * fContrast + t, vColor.a);
}


//float2 getRandom(float2 uv)
//{
//	float random_size = 64;
//	float g_sample_rad = 1.0f;
//	float g_intensity = 1.0f;
//	float g_scale = 1.0f;
//	float g_bias = 0.0001f;
//	float g_screen_size = 144000;
//	return normalize(gtxtInputTextures[0].Sample(gssWrap, g_screen_size * uv / random_size).xy * 2.0f - 1.0f);
//}

//float doAmbientOcclusion(float2 tcoord, float2 uv, float3 p, float3 cnorm)
//{
//	float random_size = 64;
//	float g_sample_rad = 1.0f;
//	float g_intensity = 1.0f;
//	float g_scale = 1.0f;
//	float g_bias = 0.0001f;
//	float g_screen_size = 144000;
//
//	float3 diff = getPosition(tcoord + uv) - p;
//	//if (diff.z>0) return 1.0f;
//	const float3 v = normalize(diff);
//	const float d = length(diff) * g_scale;
//	return max(0.0, dot(cnorm, v) - g_bias) * (1.0 / (1.0 + d)) * g_intensity;
//}

float3 getPositionBack(in float2 uv)
{
	return gtxtInputTextures[3].Sample(gssWrap, uv).xyz;
}

float doAmbientOcclusionBack(in float2 tcoord, in float2 uv, in float3 p, in float3 cnorm)
{
	float random_size = 64;
	float g_sample_rad = 1.0f;
	float g_intensity = 1.0f;
	float g_scale = 1.0f;
	float g_bias = 0.0001f;
	float g_screen_size = 144000;
	float3 diff = getPositionBack(tcoord + uv) - p;
	const float3 v = normalize(diff);
	const float d = length(diff) * g_scale;
	return max(0.0, dot(cnorm, v) - g_bias) * (1.0 / (1.0 + d));
}

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

	float4 uvs[MAX_LIGHTS] : TEXCOORD1;
};

struct PS_MULTIPLE_RENDER_TARGETS_OUTPUT
{
	float4 f4Scene : SV_TARGET0; //Swap Chain Back Buffer

	float4 f4Color : SV_TARGET1;
	float4 f4Normal : SV_TARGET2;
	float4 f4Albedo : SV_TARGET3;
	float4 f4Position : SV_TARGET4;
	float4 f4CameraNormal : SV_TARGET5;
};

VS_STANDARD_OUTPUT VSStandard(VS_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	float4 positionW = mul(float4(input.position, 1.0f), gmtxGameObject);
	output.positionW = positionW.xyz;
	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.normalV = mul(float4(output.normalW, 1.0f), gmtxView).xyz;
	output.tangentW = mul(input.tangent, (float3x3)gmtxGameObject);
	output.bitangentW = mul(input.bitangent, (float3x3)gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (gcbToLightSpaces[i].f4Position.w != 0.0f)
			output.uvs[i] = mul(positionW, gcbToLightSpaces[i].mtxToTexture);
	}

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
	float4 cSpecularColor = float4(0.0f, 0.0f, 1.0f, 0.0f);
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
	float4 cIllumination = Lighting(input.positionW, normalW, false, input.uvs);
	//output.f4Illumination = cIllumination;

	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	output.f4Albedo = cAlbedoColor;

	//output.f4Scene = output.f4Color = lerp(output.f4Illumination, output.f4Texture, 0.7f);
	//output.f4Scene = output.f4Color = output.f4Albedo * cIllumination + cEmissionColor;

	output.f4Normal = float4(normalW.xyz * 0.5f + 0.5f, input.position.z);
	cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
	cSpecularColor = cSpecularColor * gMaterial.m_cSpecular;
	//output.f4Specular = gMaterial.m_cSpecular;
    output.f4Position = (input.positionW, 1.0f);
	output.f4CameraNormal = float4(input.normalV.xyz * 0.5f + 0.5f, input.position.z);

	//output.f2ObjectIDzDepth.x = (float)1.0f;
	//output.f2ObjectIDzDepth.y = 1.0f - input.position.z;

	if (cColor.w < 0.524f)
		discard;
	
	SSAO_PS_OUTPUT o = (SSAO_PS_OUTPUT)0;

	o.color.rgb = 1.0f;
	const float2 vec[4] = { float2(1,0),float2(-1,0), float2(0,1),float2(0,-1) };

	float3 p = getPosition(input.uv);
	float3 n = getNormal(input.uv);
	float2 rand = getRandom(input.uv);
	if (p.x == 0)  output.f4Color = float4(1, 0, 0, 1.0f);
	if (n.x == 0)  output.f4Color = float4(0, 1, 0, 1.0f);
	if (rand.x == 0)  output.f4Color = float4(0, 0, 1, 1.0f);

	float ao = 0.0f;
	int iterations = 4;
	for (int j = 0; j < iterations; ++j)
	{
		ao += doAmbientOcclusion(input.uv, rand * 0.25f, p, n);
		ao += doAmbientOcclusion(input.uv, rand * 0.5f, p, n);
		ao += doAmbientOcclusion(input.uv, rand * 0.75f, p, n);
		ao += doAmbientOcclusion(input.uv, rand, p, n);
	}
	ao /= (float)iterations * 4.0f;

	output.f4Scene = output.f4Color = output.f4Albedo * cIllumination - float4(ao, ao, ao, 1.0f) + cEmissionColor;
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

	//output.f4Illumination = cIllumination;

	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	output.f4Albedo = cAlbedoColor;

	//output.f4Scene = output.f4Color = lerp(output.f4Illumination, output.f4Texture, 0.7f);
	output.f4Scene = output.f4Color = output.f4Albedo * cIllumination;

	output.f4Normal = float4(normalW.xyz * 0.5f + 0.5f, input.position.z);
	cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
	cSpecularColor = cSpecularColor * gMaterial.m_cSpecular;
	//output.f4Specular = gMaterial.m_cSpecular;
    output.f4Position = (input.positionW, 1.0f);
	output.f4CameraNormal = float4(input.normalV.xyz * 0.5f + 0.5f, input.position.z);

	//output.f2ObjectIDzDepth.x = (float)1.0f;
	//output.f2ObjectIDzDepth.y = 1.0f - input.position.z;

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
	float4 positionW = mul(float4(input.position, 1.0f), mtxVertexToBoneWorld);
	output.positionW = positionW.xyz;
	output.normalW = mul(input.normal, (float3x3)mtxVertexToBoneWorld).xyz;
	output.normalV = mul(float4(output.normalW, 1.0f), gmtxView).xyz;
	output.tangentW = mul(input.tangent, (float3x3)mtxVertexToBoneWorld).xyz;
	output.bitangentW = mul(input.bitangent, (float3x3)mtxVertexToBoneWorld).xyz;

	//	output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;

	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	for (int j = 0; j < MAX_LIGHTS; j++)
	{
		if (gcbToLightSpaces[j].f4Position.w != 0.0f)
			output.uvs[j] = mul(positionW, gcbToLightSpaces[j].mtxToTexture);
	}

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

	float4 uvs[MAX_LIGHTS] : TEXCOORD2;
};

VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
	VS_TERRAIN_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.positionW = input.position;
	float4 positionW = float4(input.position, 1.0f);
	output.normalW = float3(0, 1, 0);
	output.normalV = float3(0, 1, 0);
	output.tangentW = float3(0, 0, 0);
	output.bitangentW = float3(0, 0, 0);
	output.color = input.color;
	output.uv0 = input.uv0;
	output.uv1 = input.uv1;

	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (gcbToLightSpaces[i].f4Position.w != 0.0f)
			output.uvs[i] = mul(positionW, gcbToLightSpaces[i].mtxToTexture);
	}

	return(output);
}

PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSTerrain(VS_TERRAIN_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID)
{
	PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;

	float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gssWrap, input.uv0);
	//float4 cDetailTexColor = gtxtTerrainDetailTexture.Sample(gssWrap, input.uv1);
	//	float4 cColor = saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));
	//float4 cColor = input.color * saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));

	float4 cColor = cBaseTexColor;

	float3 normalW = normalize(input.normalW);

	float4 cIllumination = Lighting(input.positionW, normalize(input.normalW), true, input.uvs);
	//output.f4Illumination = cIllumination;

	//float3 uvw = float3(input.uv0, nPrimitiveID / 2);
	output.f4Albedo = cColor;

	//output.f4Scene = output.f4Color = output.f4Albedo * cIllumination;
    output.f4Position = (input.positionW, 1.0f);
	output.f4Normal = float4(normalW.xyz * 0.5f + 0.5f, input.position.z);
	//output.f4Specular = gMaterial.m_cSpecular;
	output.f4CameraNormal = float4(input.normalV.xyz * 0.5f + 0.5f, input.position.z);

	//output.f2ObjectIDzDepth.x = (float)1.0f;
	//output.f2ObjectIDzDepth.y = 1.0f - input.position.z;

	SSAO_PS_OUTPUT o = (SSAO_PS_OUTPUT)0;

	o.color.rgb = 1.0f;
	const float2 vec[4] = { float2(1,0),float2(-1,0), float2(0,1),float2(0,-1) };

	float3 p = getPosition(input.uv0);
	float3 n = getNormal(input.uv0);
	float2 rand = getRandom(input.uv0);
	if (p.x == 0)  output.f4Color = float4(1, 0, 0, 1.0f);
	if (n.x == 0)  output.f4Color = float4(0, 1, 0, 1.0f);
	if (rand.x == 0)  output.f4Color = float4(0, 0, 1, 1.0f);

	float ao = 0.0f;
	int iterations = 4;
	for (int j = 0; j < iterations; ++j)
	{
		ao += doAmbientOcclusion(input.uv0, rand * 0.25f, p, n);
		ao += doAmbientOcclusion(input.uv0, rand * 0.5f, p, n);
		ao += doAmbientOcclusion(input.uv0, rand * 0.75f, p, n);
		ao += doAmbientOcclusion(input.uv0, rand, p, n);
	}
	ao /= (float)iterations * 4.0f;

	output.f4Scene = output.f4Color = output.f4Albedo * cIllumination - float4(ao, ao, ao, 1.0f);
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
	//output.f4Illumination = cIllumination;

	output.f4Albedo = cColor;

	output.f4Scene = output.f4Color = output.f4Albedo * gcGlobalAmbientLight;

	output.f4Normal = float4(normalW.xyz * 0.5f + 0.5f, input.position.z);
	//output.f4Specular = gMaterial.m_cSpecular;
    output.f4Position = (input.position, 1.0f);
	output.f4CameraNormal = float4(input.normalV.xyz * 0.5f + 0.5f, input.position.z);

	//output.f2ObjectIDzDepth.x = (float)1.0f;
	//output.f2ObjectIDzDepth.y = 1.0f - input.position.z;
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

	float4 uvs[MAX_LIGHTS] : TEXCOORD1;
};

VS_WALL_OUTPUT VSWall(VS_WALL_INPUT input)
{
	VS_WALL_OUTPUT output;

	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.normalV = mul(float4(output.normalW, 1.0f), gmtxView).xyz;
	float4 positionW = mul(float4(input.position, 1.0f), gmtxGameObject);
	output.positionW = positionW.xyz;
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.tangentW = float3(0, 0, 0);
	output.bitangentW = float3(0, 0, 0);
	output.uv = input.uv;
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (gcbToLightSpaces[i].f4Position.w != 0.0f)
			output.uvs[i] = mul(positionW, gcbToLightSpaces[i].mtxToTexture);
	}
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

    float4 cColor = cAlbedoColor + cSpecularColor + cMetallicColor + cEmissionColor;
	float3 normalW = normalize(input.normalW);

	float4 cIllumination = Lighting(input.positionW, normalize(input.normalW), true, input.uvs);
	//output.f4Illumination = cIllumination;

	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	output.f4Albedo = cColor;

	//output.f4Scene = output.f4Color = lerp(output.f4Illumination, output.f4Texture, 0.7f);
	//output.f4Scene = output.f4Color = output.f4Albedo * cIllumination;

	output.f4Normal = float4(normalW.xyz * 0.5f + 0.5f, input.position.z);
	cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
	cSpecularColor = cSpecularColor * gMaterial.m_cSpecular;
	//output.f4Specular = gMaterial.m_cSpecular;
    output.f4Position = (input.positionW, 1.0f);
	output.f4CameraNormal = float4(input.normalV.xyz * 0.5f + 0.5f, input.position.z);

	//output.f2ObjectIDzDepth.x = (float)1.0f;
	//output.f2ObjectIDzDepth.y = 1.0f - input.position.z;
	
	SSAO_PS_OUTPUT o = (SSAO_PS_OUTPUT)0;

	o.color.rgb = 1.0f;
	const float2 vec[4] = { float2(1,0),float2(-1,0), float2(0,1),float2(0,-1) };

	float3 p = getPosition(input.uv);
	float3 n = getNormal(input.uv);
	float2 rand = getRandom(input.uv);
	if (p.x == 0)  output.f4Color = float4(1, 0, 0, 1.0f);
	if (n.x == 0)  output.f4Color = float4(0, 1, 0, 1.0f);
	if (rand.x == 0)  output.f4Color = float4(0, 0, 1, 1.0f);

	float ao = 0.0f;
	int iterations = 4;
	for (int j = 0; j < iterations; ++j)
	{
		ao += doAmbientOcclusion(input.uv, rand * 0.25f, p, n);
		ao += doAmbientOcclusion(input.uv, rand * 0.5f, p, n);
		ao += doAmbientOcclusion(input.uv, rand * 0.75f, p, n);
		ao += doAmbientOcclusion(input.uv, rand, p, n);
	}
	ao /= (float)iterations * 4.0f;

    output.f4Scene = output.f4Color = output.f4Albedo * cIllumination - float4(ao, ao, ao, 1.0f);
    output.f4Color.w = gnObjectType / 10.0f;
    return (output);
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

	float4 uvs[MAX_LIGHTS] : TEXCOORD1;
};

VS_TEXTURED_LIGHTING_OUTPUT VSTexturedLighting(VS_TEXTURED_LIGHTING_INPUT input)
{
	VS_TEXTURED_LIGHTING_OUTPUT output;

	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.normalV = mul(float4(output.normalW, 1.0f), gmtxView).xyz;
	float4 positionW = mul(float4(input.position, 1.0f), gmtxGameObject);
	output.positionW = (float3) positionW;
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.tangentW = mul(input.tangent, (float3x3)gmtxGameObject);
	output.bitangentW = mul(input.bitangent, (float3x3)gmtxGameObject);
	output.uv = input.uv;
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (gcbToLightSpaces[i].f4Position.w != 0.0f)
			output.uvs[i] = mul(positionW, gcbToLightSpaces[i].mtxToTexture);
	}
	return(output);
}

float4 PSTexturedLighting(VS_TEXTURED_LIGHTING_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET0
{
	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	float4 cColor = gtxtAlbedoTexture.Sample(gssWrap, uvw);
	input.normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, normalize(input.normalW), true, input.uvs);
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

	//float4 cIllumination = Lighting(input.positionW, normalW);

	float4 cIllumination = Lighting(input.positionW, normalize(input.normalW), true, input.uvs);
	clip(cIllumination.w - 0.15f);
	//return(lerp(cColor, cIllumination, 0.5f));
	//output.f4Illumination = cIllumination;

	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	//output.f4Texture = gtxtAlbedoTexture.Sample(gssWrap, uvw);// Texture2DArray gtxtTextureArray : register(t0); 정의해야함   또 원래는 gtxtTextureArray를 사용하고있었는데 임시로 알베도로 바꿔놓음
	output.f4Albedo = cColor;
	input.normalW = normalize(input.normalW);
	//output.f4Illumination = Lighting(input.positionW, input.normalW);

	//output.f4Scene = output.f4Color = output.f4Illumination * output.f4Texture;
	//output.f4Scene = output.f4Color = lerp(output.f4Illumination, output.f4Texture, 0.7f);
	//output.f4Scene = output.f4Color = output.f4Albedo * cIllumination + cEmissionColor;
	//output.f4Normal = float4(input.normalW.xyz * 0.5f + 0.5f, input.position.z);
	//output.f4CameraNormal = float4(input.normalV.xyz * 0.5f + 0.5f, input.position.z);

	output.f4Normal = float4(normalW.xyz * 0.5f + 0.5f, input.position.z);
	cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
	cSpecularColor = cSpecularColor * gMaterial.m_cSpecular;
	//output.f4Specular = gMaterial.m_cSpecular;
    output.f4Position = (input.positionW, 1.0f);
	output.f4CameraNormal = float4(input.normalV.xyz * 0.5f + 0.5f, input.position.z);

	//output.f2ObjectIDzDepth.x = (float)1.0f;
	//output.f2ObjectIDzDepth.y = 1.0f - input.position.z;
	//output.f4Scene = output.f4Texture;
	//output.f4Scene = float4(1,1,1,1);
	
	SSAO_PS_OUTPUT o = (SSAO_PS_OUTPUT)0;

	o.color.rgb = 1.0f;
	const float2 vec[4] = { float2(1,0),float2(-1,0), float2(0,1),float2(0,-1) };

	float3 p = getPosition(input.uv);
	float3 n = getNormal(input.uv);
	float2 rand = getRandom(input.uv);
	if (p.x == 0)  output.f4Color = float4(1, 0, 0, 1.0f);
	if (n.x == 0)  output.f4Color = float4(0, 1, 0, 1.0f);
	if (rand.x == 0)  output.f4Color = float4(0, 0, 1, 1.0f);

	float ao = 0.0f;
	int iterations = 4;
	for (int j = 0; j < iterations; ++j)
	{
		ao += doAmbientOcclusion(input.uv, rand * 0.25f, p, n);
		ao += doAmbientOcclusion(input.uv, rand * 0.5f, p, n);
		ao += doAmbientOcclusion(input.uv, rand * 0.75f, p, n);
		ao += doAmbientOcclusion(input.uv, rand, p, n);
	}
	ao /= (float)iterations * 4.0f;

	output.f4Scene = output.f4Color = output.f4Albedo * cIllumination - float4(ao, ao, ao, 1.0f) + cEmissionColor;
    if (gnObjectType == 10)
    {
        output.f4Scene = output.f4Color = output.f4Albedo;
        output.f4Color.w = 0;
    }
	else 
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

	if (nVertexID == 0) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	else if (nVertexID == 1) { output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 0.0f); }
	else if (nVertexID == 2) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	else if (nVertexID == 3) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	else if (nVertexID == 4) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	else if (nVertexID == 5) { output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 1.0f); }

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
			float3 f3Depth = gtxtInputTextures[5][int2(position.xy) + gnOffsets[input]].xyz * 2.0f - 1.0f;
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

	float3 f3NormalV0 = gtxtInputTextures[4].Sample(gssWrap, f2BottomLeftUV).rgb;
	float3 f3NormalV1 = gtxtInputTextures[4].Sample(gssWrap, f2TopRightUV).rgb;
	float3 f3NormalV2 = gtxtInputTextures[4].Sample(gssWrap, f2BottomRightUV).rgb;
	float3 f3NormalV3 = gtxtInputTextures[4].Sample(gssWrap, f2TopLeftUV).rgb;

	float fDepth0 = gtxtInputTextures[5].Sample(gssWrap, f2BottomLeftUV).r;
	float fDepth1 = gtxtInputTextures[5].Sample(gssWrap, f2TopRightUV).r;
	float fDepth2 = gtxtInputTextures[5].Sample(gssWrap, f2BottomRightUV).r;
	float fDepth3 = gtxtInputTextures[5].Sample(gssWrap, f2TopLeftUV).r;

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
		myColor = float4(0, 0.6f, 0, 1);
	else if (k < 0.5f)
		myColor = float4(0.6f, 0, 0, 1);
	else
		myColor = float4(0.2, 0.2, 0.2, 1);
	float4 f4EdgeColor = float4(myColor.xyz, myColor.w * fEdge * 3.0);
	//float4 f4EdgeColor = float4(1.0f, 0,0,1.0f * fEdge);
	float4 f4Color = gtxtInputTextures[0].Sample(gssWrap, input.uv);

	//float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
	//float weights[5] = { 0.227027f, 0.1945946f, 0.1216216f, 0.054054f, 0.0162162f };
	//float texelSize = 0.2f;
	//float value = 2;
	//for (int i = -value; i <= value; ++i)
	//{
	//	for (int j = -value; j <= value; ++j)
	//	{
	//		color += gtxtInputTextures[0].Sample(gssWrap, input.uv + float2(i, j) * texelSize) * weights[i + 2] * weights[j + 2];
	//	}
	//}

	float3 result = float3(0, 0, 0);
	float totalWeight = 0;
	float blurRadius = 0;
	if(gtxtInputTextures[5].Sample(gssWrap,input.uv).r<0.12f) blurRadius = 0.003;
	else blurRadius = 0.0002;
	// 블러의 크기를 설정
	int blurSize = 6;

	// 가우시안 블러 마스크 생성
	float weights[13] = { 0.0561, 0.1353, 0.278, 0.4868, 0.7261, 0.9231,
	1, 0.9231, 0.7261, 0.4868, 0.278, 0.1353, 0.0561 };

	// 블러를 적용할 주변 픽셀들을 순회하며 블러 적용
	for (int i = -blurSize; i <= blurSize; ++i)
	{
		for (int j = -blurSize; j <= blurSize; ++j)
		{
			float2 offset = float2(i, j) * blurRadius;
			float3 color = gtxtInputTextures[0].Sample(gssWrap,input.uv + offset);

			float weight = weights[i + blurSize] * weights[j + blurSize];
			result += color * weight;
			totalWeight += weight;
		}
	}

	// 결과를 가중치로 나누어 정규화
	result /= totalWeight;
	float4 color = float4(result, 1.0f);
	//if(ao>0.5) return(AlphaBlend(f4EdgeColor, f4Color)-ao);
	return(AlphaBlend(f4EdgeColor, color));
}

float4 PSScreenRectSamplingTextured(VS_SCREEN_RECT_TEXTURED_OUTPUT input) : SV_Target
{
	float4 cColor = float4(1.0f, 1.0f, 0.0f, 0.1f);

	float vDepthInfo = GetColorFromDepth(1.0f-gtxtInputTextures[5].Load(uint3((uint)input.position.x, (uint)input.position.y, 0)).r);
	//cColor = color;
	cColor = Outline(input);
	float4 LinkColor = Adjust_Brightness(cColor, -0.1 * (1.f - vDepthInfo));
	if (vDepthInfo > 0.5f)
		LinkColor = Adjust_Contrast(LinkColor, 3);

	cColor = lerp(LinkColor, cColor, vDepthInfo);

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
	output.position = float4(input.position, 1.0f);
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
	//Color *= 0.2f;
	clip(Color.w - 0.1f);
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
#define BLOCKED_UI 5
#define INGAME_UI 6
#define TAGGER_UI 7
#define  PROGRESS_BAR_UI 8

VS_UI_OUTPUT VSDoorUI(VS_UI_INPUT input)
{
	VS_UI_OUTPUT output;

	if (gnUIType == INGAME_UI || gnUIType == PROGRESS_BAR_UI) {
		output.position = mul(float4(input.position, 1.0f), gmtxGameObject);
	}
	else {
		output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	}
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
	else if (gnUIType == BLOCKED_UI) {
		if (input.uv.y > 1.0f - gfGauge && Color.w < 0.1f) {
			Color = float4(1.0f, 0.0f, 0.0f, 1.0f);
		}
	}
	else if (gnUIType == INGAME_UI) {
		if (gfGauge <= 0.0f) {
			Color *= 0.5f;
			//if (Color.w < 0.1f)
				//Color = float4(0.3f, 0.3f, 0.3f, 1.0f);
		}
	}
	else if (gnUIType == TAGGER_UI) {
		if (input.uv.y - 1.0f > -gfGauge && Color.w < 0.1f)
			Color = float4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else if (gnUIType == PROGRESS_BAR_UI)
	{
		if (input.uv.x > gfGauge)
		{
			Color.w = 0;
		}
	}
	clip(Color.w - 0.1f);
	return Color;
}

VS_UI_OUTPUT VSMinimapUI(VS_UI_INPUT input)
{
	VS_UI_OUTPUT output;
	output.position = float4(input.position, 1.0f);
	output.uv = input.uv;
	return output;
}

float4 PSMinimapUI(VS_UI_OUTPUT input) : SV_TARGET
{
	float4 Color = gtxtUITexture.Sample(gssWrap, input.uv);
	float2 newvalue = float2((10 * gvCameraPosition.x + 600) / 1600, (5.625 * -gvCameraPosition.z + 450) / 900);
	if (newvalue.x < input.uv.x + 0.014 && newvalue.x > input.uv.x + 0.006 && newvalue.y < input.uv.y + 0.007 && newvalue.y > input.uv.y - 0.007)
	{
		Color = float4(1, 0, 1, 1);
	}
	else
	{
		Color.w = 0.8f;
	}
	//clip(Color.w - 0.1f);
	return Color;
}


struct VS_SHADOW_MAP_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;

	float4 uvs[MAX_LIGHTS] : TEXCOORD0;
};

VS_SHADOW_MAP_OUTPUT VSShadowMapShadow(VS_STANDARD_INPUT input)
{
	VS_SHADOW_MAP_OUTPUT output = (VS_SHADOW_MAP_OUTPUT)0;

	float4 positionW = mul(float4(input.position, 1.0f), gmtxGameObject);
	output.positionW = positionW.xyz;
	output.position = mul(mul(positionW, gmtxView), gmtxProjection);
	output.normalW = mul(float4(input.normal, 0.0f), gmtxGameObject).xyz;

	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (gcbToLightSpaces[i].f4Position.w != 0.0f)
			output.uvs[i] = mul(positionW, gcbToLightSpaces[i].mtxToTexture);
	}

	return (output);
}

float4 PSShadowMapShadow(VS_SHADOW_MAP_OUTPUT input) : SV_TARGET
{
	float4 cIllumination = Lighting(input.positionW, normalize(input.normalW), true, input.uvs);

	//cIllumination = saturate(gtxtDepthTextures[3].SampleLevel(gssProjector, f3uvw.xy, 0).r);

	return (cIllumination);
}

struct PS_DEPTH_OUTPUT
{
	float fzPosition : SV_Target;
	float fDepth : SV_Depth;
};

PS_DEPTH_OUTPUT PSDepthWriteShader(VS_STANDARD_OUTPUT input)
{
	PS_DEPTH_OUTPUT output;

	output.fzPosition = input.position.z;
	output.fDepth = input.position.z;

	return (output);
}

struct VS_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_TEXTURED_OUTPUT VSTextureToViewport(uint nVertexID : SV_VertexID)
{
	VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;

	if (nVertexID == 0)
	{
		output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 0.0f);
	}
	if (nVertexID == 1)
	{
		output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 0.0f);
	}
	if (nVertexID == 2)
	{
		output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 1.0f);
	}
	if (nVertexID == 3)
	{
		output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 0.0f);
	}
	if (nVertexID == 4)
	{
		output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 1.0f);
	}
	if (nVertexID == 5)
	{
		output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 1.0f);
	}

	return (output);
}

float4 GetColorFromDepth2(float fDepth)
{
	float4 cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	if (fDepth < 0.00625f)
		cColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
	else if (fDepth < 0.0125f)
		cColor = float4(0.0f, 1.0f, 0.0f, 1.0f);
	else if (fDepth < 0.025f)
		cColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
	else if (fDepth < 0.05f)
		cColor = float4(1.0f, 1.0f, 0.0f, 1.0f);
	else if (fDepth < 0.075f)
		cColor = float4(0.0f, 1.0f, 1.0f, 1.0f);
	else if (fDepth < 0.1f)
		cColor = float4(1.0f, 0.5f, 0.5f, 1.0f);
	else if (fDepth < 0.4f)
		cColor = float4(0.5f, 1.0f, 1.0f, 1.0f);
	else if (fDepth < 0.6f)
		cColor = float4(1.0f, 0.0f, 1.0f, 1.0f);
	else if (fDepth < 0.8f)
		cColor = float4(0.5f, 0.5f, 1.0f, 1.0f);
	else if (fDepth < 0.9f)
		cColor = float4(0.5f, 1.0f, 0.5f, 1.0f);
	else if (fDepth < 0.95f)
		cColor = float4(0.5f, 0.0f, 0.5f, 1.0f);
	else if (fDepth < 0.99f)
		cColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	else if (fDepth < 0.999f)
		cColor = float4(1.0f, 0.0f, 1.0f, 1.0f);
	else if (fDepth == 1.0f)
		cColor = float4(0.5f, 0.5f, 0.5f, 1.0f);
	else if (fDepth > 1.0f)
		cColor = float4(0.0f, 0.0f, 0.5f, 1.0f);
	else
		cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	return (cColor);
}

SamplerState gssBorder : register(s3);

float4 PSTextureToViewport(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float4 cColor = gtxtDepthTextures[0].SampleLevel(gssBorder, input.uv, 0).r * 1.0f;

	cColor = GetColorFromDepth2(cColor.r);

	return (cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define PARTICLE_TYPE_EMITTER		0
#define PARTICLE_TYPE_SHELL			1
#define PARTICLE_TYPE_FLARE01		2
#define PARTICLE_TYPE_FLARE02		3
#define PARTICLE_TYPE_FLARE03		4

#define SHELL_PARTICLE_LIFETIME		3.0f
#define FLARE01_PARTICLE_LIFETIME	1.5f
#define FLARE02_PARTICLE_LIFETIME	0.5f
#define FLARE03_PARTICLE_LIFETIME	1.0f

Texture2D<float4> gtxtParticleTexture : register(t31);
//Texture1D<float4> gtxtRandom : register(t32);
Buffer<float4> gRandomBuffer : register(t32);
Buffer<float4> gRandomSphereBuffer : register(t33);

SamplerState gMirrorSamplerState : register(s4);
SamplerState gPointSamplerState : register(s5);

struct VS_PARTICLE_INPUT
{
    float3 position : POSITION;
    float3 velocity : VELOCITY;
    float lifetime : LIFETIME;
    uint type : PARTICLETYPE;
};

VS_PARTICLE_INPUT VSParticleStreamOutput(VS_PARTICLE_INPUT input)
{
    return (input);
}

float3 GetParticleColor(float fAge, float fLifetime)
{
    float3 cColor = float3(1.0f, 1.0f, 1.0f);

    if (fAge == 0.0f)
        cColor = float3(0.0f, 1.0f, 0.0f);
    else if (fLifetime == 0.0f)
        cColor = float3(1.0f, 1.0f, 0.0f);
    else
    {
        float t = fAge / fLifetime;
        cColor = lerp(float3(1.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 1.0f), t * 1.0f);
    }

    return (cColor);
}

void GetBillboardCorners(float3 position, float2 size, out float4 pf4Positions[4])
{
    float3 f3Up = float3(0.0f, 1.0f, 0.0f);
    float3 f3Look = normalize(gvCameraPosition - position);
    float3 f3Right = normalize(cross(f3Up, f3Look));

    pf4Positions[0] = float4(position + size.x * f3Right - size.y * f3Up, 1.0f);
    pf4Positions[1] = float4(position + size.x * f3Right + size.y * f3Up, 1.0f);
    pf4Positions[2] = float4(position - size.x * f3Right - size.y * f3Up, 1.0f);
    pf4Positions[3] = float4(position - size.x * f3Right + size.y * f3Up, 1.0f);
}

void GetPositions(float3 position, float2 f2Size, out float3 pf3Positions[8])
{
    float3 f3Right = float3(1.0f, 0.0f, 0.0f);
    float3 f3Up = float3(0.0f, 1.0f, 0.0f);
    float3 f3Look = float3(0.0f, 0.0f, 1.0f);

    float3 f3Extent = normalize(float3(1.0f, 1.0f, 1.0f));

    pf3Positions[0] = position + float3(-f2Size.x, 0.0f, -f2Size.y);
    pf3Positions[1] = position + float3(-f2Size.x, 0.0f, +f2Size.y);
    pf3Positions[2] = position + float3(+f2Size.x, 0.0f, -f2Size.y);
    pf3Positions[3] = position + float3(+f2Size.x, 0.0f, +f2Size.y);
    pf3Positions[4] = position + float3(-f2Size.x, 0.0f, 0.0f);
    pf3Positions[5] = position + float3(+f2Size.x, 0.0f, 0.0f);
    pf3Positions[6] = position + float3(0.0f, 0.0f, +f2Size.y);
    pf3Positions[7] = position + float3(0.0f, 0.0f, -f2Size.y);
}

float4 RandomDirection(float fOffset)
{
    int u = uint(gfCurrentTime + fOffset + frac(gfCurrentTime) * 1000.0f) % 1024;
    return (normalize(gRandomBuffer.Load(u)));
}

float4 RandomDirectionOnSphere(float fOffset)
{
    int u = uint(gfCurrentTime + fOffset + frac(gfCurrentTime) * 1000.0f) % 256;
    return (normalize(gRandomSphereBuffer.Load(u)));
}

void OutputParticleToStream(VS_PARTICLE_INPUT input, inout PointStream<VS_PARTICLE_INPUT> output)
{
    input.position += input.velocity * gfElapsedTime;
    input.velocity += gf3Gravity * gfElapsedTime;
    input.lifetime -= gfElapsedTime;

    output.Append(input);
}

void EmmitParticles(VS_PARTICLE_INPUT input, inout PointStream<VS_PARTICLE_INPUT> output)
{
    float4 f4Random = RandomDirection(input.type);
    if (input.lifetime <= 0.0f)
    {
        VS_PARTICLE_INPUT particle = input;

        particle.type = PARTICLE_TYPE_SHELL;
        particle.position = input.position + (input.velocity * gfElapsedTime);
        particle.velocity.xy = input.velocity.xy + (f4Random.xy * 16.0f);
        particle.lifetime = SHELL_PARTICLE_LIFETIME + (f4Random.y * 0.5f);

        output.Append(particle);

        input.lifetime = gfSecondsPerFirework * 0.2f + (f4Random.x * 0.4f);
    }
    else
    {
        input.lifetime -= gfElapsedTime;
    }

    output.Append(input);
}

void ShellParticles(VS_PARTICLE_INPUT input, inout PointStream<VS_PARTICLE_INPUT> output)
{
    if (input.lifetime <= 0.0f)
    {
        VS_PARTICLE_INPUT particle = input;
        float4 f4Random = float4(0.0f, 0.0f, 0.0f, 0.0f);

        particle.type = PARTICLE_TYPE_FLARE01;
        particle.position = input.position + (input.velocity * gfElapsedTime * 2.0f);
        particle.lifetime = FLARE01_PARTICLE_LIFETIME;

        for (int i = 0; i < gnFlareParticlesToEmit; i++)
        {
            f4Random = RandomDirection(input.type + i);
            particle.velocity.xy = input.velocity.xy + (f4Random.xy * 18.0f);

            output.Append(particle);
        }

        particle.type = PARTICLE_TYPE_FLARE02;
        particle.position = input.position + (input.velocity * gfElapsedTime);
        for (int j = 0; j < abs(f4Random.x) * gnMaxFlareType2Particles; j++)
        {
            f4Random = RandomDirection(input.type + j);
            particle.velocity.xy = input.velocity.xy + (f4Random.xy * 10.0f);
            particle.lifetime = FLARE02_PARTICLE_LIFETIME + (f4Random.x * 0.4f);

            output.Append(particle);
        }
    }
    else
    {
        OutputParticleToStream(input, output);
    }
}

void OutputEmberParticles(VS_PARTICLE_INPUT input, inout PointStream<VS_PARTICLE_INPUT> output)
{
    if (input.lifetime > 0.0f)
    {
        OutputParticleToStream(input, output);
    }
}

void GenerateEmberParticles(VS_PARTICLE_INPUT input, inout PointStream<VS_PARTICLE_INPUT> output)
{
    if (input.lifetime <= 0.0f)
    {
        VS_PARTICLE_INPUT particle = input;

        particle.type = PARTICLE_TYPE_FLARE03;
        particle.position = input.position + (input.velocity * gfElapsedTime);
        particle.lifetime = FLARE03_PARTICLE_LIFETIME;
        for (int i = 0; i < 64; i++)
        {
            float4 f4Random = RandomDirectionOnSphere(input.type + i);
            particle.velocity.xy = input.velocity.xy + (f4Random.xy * 25.0f);

            output.Append(particle);
        }
    }
    else
    {
        OutputParticleToStream(input, output);
    }
}

[maxvertexcount(128)]
void GSParticleStreamOutput(point VS_PARTICLE_INPUT input[1], inout PointStream<VS_PARTICLE_INPUT> output)
{
    VS_PARTICLE_INPUT particle = input[0];

    if (particle.type == PARTICLE_TYPE_EMITTER)
        EmmitParticles(particle, output);
    else if (particle.type == PARTICLE_TYPE_SHELL)
        ShellParticles(particle, output);
    else if ((particle.type == PARTICLE_TYPE_FLARE01) || (particle.type == PARTICLE_TYPE_FLARE03))
        OutputEmberParticles(particle, output);
    else if (particle.type == PARTICLE_TYPE_FLARE02)
        GenerateEmberParticles(particle, output);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_PARTICLE_DRAW_OUTPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
    float size : SCALE;
    uint type : PARTICLETYPE;
};

struct GS_PARTICLE_DRAW_OUTPUT
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXTURE;
    uint type : PARTICLETYPE;
};

VS_PARTICLE_DRAW_OUTPUT VSParticleDraw(VS_PARTICLE_INPUT input)
{
    VS_PARTICLE_DRAW_OUTPUT output = (VS_PARTICLE_DRAW_OUTPUT) 0;

    output.position = input.position;
    output.size = 2.5f;
    output.type = input.type;

    if (input.type == PARTICLE_TYPE_EMITTER)
    {
        output.color = float4(1.0f, 0.1f, 0.1f, 1.0f);
        output.size = 1.0f;
    }
    else if (input.type == PARTICLE_TYPE_SHELL)
    {
        output.color = float4(0.1f, 0.0f, 1.0f, 1.0f);
        output.size = 1.0f;
    }
    else if (input.type == PARTICLE_TYPE_FLARE01)
    {
        output.color = float4(1.0f, 1.0f, 0.1f, 1.0f);
        output.color *= (input.lifetime / FLARE01_PARTICLE_LIFETIME);
    }
    else if (input.type == PARTICLE_TYPE_FLARE02)
        output.color = float4(1.0f, 0.1f, 1.0f, 1.0f);
    else if (input.type == PARTICLE_TYPE_FLARE03)
    {
        output.color = float4(1.0f, 0.1f, 1.0f, 1.0f);
        output.color *= (input.lifetime / FLARE03_PARTICLE_LIFETIME);
    }

    return (output);
}

static float3 gf3Positions[4] = { float3(-1.0f, +1.0f, 0.5f), float3(+1.0f, +1.0f, 0.5f), float3(-1.0f, -1.0f, 0.5f), float3(+1.0f, -1.0f, 0.5f) };
static float2 gf2QuadUVs[4] = { float2(0.0f, 0.0f), float2(1.0f, 0.0f), float2(0.0f, 1.0f), float2(1.0f, 1.0f) };

[maxvertexcount(4)]
void GSParticleDraw(point VS_PARTICLE_DRAW_OUTPUT input[1], inout TriangleStream<GS_PARTICLE_DRAW_OUTPUT> outputStream)
{
    GS_PARTICLE_DRAW_OUTPUT output = (GS_PARTICLE_DRAW_OUTPUT) 0;

    output.type = input[0].type;
    output.color = input[0].color;
    for (int i = 0; i < 4; i++)
    {
        float3 positionW = mul(gf3Positions[i] * input[0].size, (float3x3) gmtxInverseView) + input[0].position;
        output.position = mul(mul(float4(positionW, 1.0f), gmtxView), gmtxProjection);
        output.uv = gf2QuadUVs[i];

        outputStream.Append(output);
    }
    outputStream.RestartStrip();
}

float4 PSParticleDraw(GS_PARTICLE_DRAW_OUTPUT input) : SV_TARGET
{
    float4 cColor = gtxtParticleTexture.Sample(gssWrap, input.uv);
    cColor *= input.color;

    return (cColor);
}
