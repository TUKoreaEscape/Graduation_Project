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

#include "Light.hlsl"

struct CB_TOOBJECTSPACE
{
    matrix mtxToTexture;
    float4 f4Position;
};

cbuffer cbDebug : register(b2)
{
    int4 gvDebugOptions : packoffset(c0);
};

cbuffer cbToLightSpace : register(b6)
{
    CB_TOOBJECTSPACE gcbToLightSpaces[MAX_LIGHTS];
};

Texture2D gtxtInputTextures[7] : register(t14); //Albedo, Normal, Specular, Emission, Position, NormalV, Depth // 3과 4는 Illumination, ObjectID+zDepth 인데 이거 두개는 쓸모없어서 삭제함

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

const float random_size = 4096;
const float g_sample_rad = 1.0f;
const float g_intensity = 100.0f;
const float g_scale = 1.8f;
const float g_bias = 0.000001f;
const float g_screen_size = 144000;

struct SSAO_PS_INPUT
{
    float2 uv : TEXCOORD0;
};

struct SSAO_PS_OUTPUT
{
    float4 color : COLOR0;
};

float3 getPosition(float2 uv)
{
    return gtxtInputTextures[3].Sample(gssWrap, uv).xyz;
}

float3 getNormal(float2 uv)
{
    return normalize(gtxtInputTextures[1].Sample(gssWrap, uv).xyz * 2.0f - 1.0f);
}

float2 getRandom(float2 uv)
{
    float random_size = 64;
    float g_sample_rad = 1.0f;
    float g_intensity = 1.0f;
    float g_scale = 1.0f;
    float g_bias = 0.0001f;
    float g_screen_size = 144000;
    return normalize(gtxtInputTextures[0].Sample(gssWrap, g_screen_size * uv / random_size).xy * 2.0f - 1.0f);
}

float doAmbientOcclusion(float2 tcoord, float2 uv, float3 p, float3 cnorm)
{
    float random_size = 64;
    float g_sample_rad = 1.0f;
    float g_intensity = 1.0f;
    float g_scale = 1.0f;
    float g_bias = 0.0001f;
    float g_screen_size = 144000;

    float3 diff = getPosition(tcoord + uv) - p;
    //if (diff.z>0) return 1.0f;
    const float3 v = normalize(diff);
    const float d = length(diff) * g_scale;
    return max(0.0, dot(cnorm, v) - g_bias) * (1.0 / (1.0 + d)) * g_intensity;
}

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
    float4 f4Albedo : SV_TARGET0;
    float f4Normal : SV_TARGET1;
    float4 f4Specular : SV_TARGET2;
    float4 f4Emission : SV_TARGET3;
    
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

    float3 uvw = float3(input.uv, nPrimitiveID / 2);
    output.f4Albedo = cAlbedoColor;
    output.f4Normal = float4(normalW.xyz * 0.5f + 0.5f, input.position.z);
    output.f4Specular = cSpecularColor;
    output.f4Emission = cEmissionColor + gMaterial.m_cEmissive;
    output.f4Position = (input.positionW, 1.0f);
    output.f4CameraNormal = float4(input.normalV.xyz * 0.5f + 0.5f, input.position.z);

    if (cColor.w < 0.524f)
        discard;
    output.f4Albedo.w = gnObjectType / 10.0f;

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
    float3 uvw = float3(input.uv, nPrimitiveID / 2);
    output.f4Albedo = cAlbedoColor;
    output.f4Normal = float4(normalW.xyz * 0.5f + 0.5f, input.position.z);
    output.f4Specular = cSpecularColor;
    output.f4Emission = cEmissionColor + gMaterial.m_cEmissive;
    output.f4Position = (input.positionW, 1.0f);
    output.f4CameraNormal = float4(input.normalV.xyz * 0.5f + 0.5f, input.position.z);

    clip(cColor.a - 0.15f);

    output.f4Albedo.w = gnObjectType / 10.0f;
    return(output);
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
    
    float4 cColor = input.color * cBaseTexColor;

    float3 normalW = normalize(input.normalW);

    output.f4Albedo = cColor;
    output.f4Normal = float4(normalW.xyz * 0.5f + 0.5f, input.position.z);
    output.f4Specular = gMaterial.m_cSpecular;
    output.f4Emission = gMaterial.m_cEmissive;
    output.f4Position = float4(input.positionW, 1.0f);
    output.f4CameraNormal = float4(input.normalV.xyz * 0.5f + 0.5f, input.position.z);

    output.f4Albedo.w = gnObjectType / 10.0f;
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
    output.f4Normal = float4(normalW.xyz * 0.5f + 0.5f, input.position.z);
    output.f4Specular = gMaterial.m_cSpecular;
    output.f4Emission = gMaterial.m_cEmissive;
    output.f4Position = (input.positionL, 1.0f);
    output.f4CameraNormal = float4(input.normalV.xyz * 0.5f + 0.5f, input.position.z);

    output.f4Albedo.w = gnObjectType / 10.0f;
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

    output.f4Albedo = cAlbedoColor;
    output.f4Normal = float4(normalW.xyz * 0.5f + 0.5f, input.position.z);
    output.f4Specular = cSpecularColor;
    output.f4Emission = cEmissionColor + gMaterial.m_cEmissive;
    output.f4Position = (input.positionW, 1.0f);
    output.f4CameraNormal = float4(input.normalV.xyz * 0.5f + 0.5f, input.position.z);

    output.f4Albedo.w = gnObjectType / 10.0f;
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
    }
    else
    {
        cEmissionColor = gMaterial.m_cEmissive;
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

    output.f4Albedo = cAlbedoColor;
    output.f4Normal = float4(normalW.xyz * 0.5f + 0.5f, input.position.z);
    output.f4Specular = cSpecularColor;
    output.f4Emission = cEmissionColor;
    output.f4Position = (input.positionW, 1.0f);
    output.f4CameraNormal = float4(input.normalV.xyz * 0.5f + 0.5f, input.position.z);

    output.f4Albedo.w = gnObjectType / 10.0f;

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
        myColor = float4(0, 0.6f, 0, 1);
    else if (k < 0.5f)
        myColor = float4(0.6f, 0, 0, 1);
    else
        myColor = float4(0.2, 0.2, 0.2, 1);
    float4 f4EdgeColor = float4(myColor.xyz, myColor.w * fEdge);
    //float4 f4EdgeColor = float4(1.0f, 0,0,1.0f * fEdge);
    float4 f4Color = gtxtInputTextures[0].Sample(gssWrap, input.uv);
    float4 f4Specular = gtxtInputTextures[2].Sample(gssWrap, input.uv);
    float3 f3PositionW = gtxtInputTextures[4].Sample(gssWrap, input.uv).xyz;
    float3 f3NormalW = gtxtInputTextures[1].Sample(gssWrap, input.uv).xyz;
    float4 f4Emission = gtxtInputTextures[3].Sample(gssWrap, input.uv);
    f3NormalW = (f3NormalW - 0.5f) * 2.0f;
    
    float4 uvs[MAX_LIGHTS];
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (gcbToLightSpaces[i].f4Position.w != 0.0f)
            uvs[i] = mul(float4(f3PositionW, 1.0f), gcbToLightSpaces[i].mtxToTexture);
    }
    
    float4 cIllumination = Lighting(f3PositionW, f3NormalW, true, uvs, f4Color.xyz, f4Specular.xyz, f4Specular.w) + f4Emission;
    f4Color = cIllumination * f4Color;
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
        if (input.screenNum == 5)
        {
            float fDepth = gtxtInputTextures[5].Load(uint3((uint)input.position.x, (uint)input.position.y, 0)).r;
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
    if (gvDebugOptions.x == 60)
    {
        float2 newvalue = float2((10 * gvCameraPosition.x + 600) / 1600, (5.625 * -gvCameraPosition.z + 450) / 900);
        //float2 value = input.uv - newvalue;
        //float d = length(value);
        if (newvalue.x < input.uv.x + 0.013 && newvalue.x > input.uv.x + 0.007 && newvalue.y < input.uv.y + 0.005 && newvalue.y > input.uv.y - 0.005)
        {
            Color = float4(1, 0, 1, 1);
        }
    }
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
