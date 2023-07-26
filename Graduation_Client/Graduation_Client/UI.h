#pragma once
#include "stdafx.h"
#include "Input.h"

extern RECT_FLOAT idRect;
extern RECT_FLOAT passwordRect;

extern RECT_FLOAT logininfoRect[5];

extern RECT_FLOAT roominfoRect[6];

extern RECT_FLOAT roomPageRect[2];

extern RECT_FLOAT pageNumRect;

extern RECT_FLOAT waitingRoomRect[3];

extern RECT_FLOAT customizingRect[10];

extern RECT_FLOAT endingRect;

extern RECT_FLOAT powerRect;

extern RECT_FLOAT chatBoxRect;

void UpdateRectSize(HWND hWnd);

/*
// ±íÀÌÅ¸°Ù
texture		g_DepthTexture;
sampler DepthSampler = sampler_state
{
	texture = g_DepthTexture;
};

// ³ë¸»Å¸°Ù
texture		g_NormalTexture;
sampler NormalSampler = sampler_state
{
	texture = g_NormalTexture;
};

float		g_fPixelX, g_fPixelY;
float		g_fFar = 500.f;
float		g_fNear = 0.01f;

float		g_fBloomStrength = 2.f;
vector		g_vBlendColor = vector(0.4784313, 0.541176, 1, 0.3);

float		g_LinkTime;

////OUTLINE//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//variables for customising the effect
vector g_OutlineColor = float4(0.f, 0.f, 0.f, 1.f);
float g_NormalMult = 10.f;
float g_NormalBias = 1.f;
float g_DepthMult = 10.f;
float g_DepthBias = 1.f;

void		Compare(inout float depthOutline, inout float normalOutline, float baseDepth, float3 baseNormal, float2 uv, float2 offset)
{
	float4 vNeighborDepth = tex2D(DepthSampler, uv + float2(g_fPixelX, g_fPixelY) * offset);
	float3	vNeighborNormal = tex2D(NormalSampler, uv + float2(g_fPixelX, g_fPixelY) * offset).xyz;
	float	fNeighborDepth = vNeighborDepth.y * g_fFar;

	float	fDepthDifference = baseDepth - fNeighborDepth;
	depthOutline = depthOutline + fDepthDifference;

	float3	vNormalDifference = baseNormal - vNeighborNormal;
	vNormalDifference = vNormalDifference.x + vNormalDifference.y + vNormalDifference.z;
	normalOutline = normalOutline + vNormalDifference.r;
}

float Compute_OutLine(vector vNormal, float fDepth, float2 vTexUV)
{
	float	fDepthDifference = 0.f;
	float	fNormalDifference = 0.f;

	Compare(fDepthDifference, fNormalDifference, fDepth, vNormal.xyz, vTexUV, float2(1, 0));
	Compare(fDepthDifference, fNormalDifference, fDepth, vNormal.xyz, vTexUV, float2(0, 1));
	Compare(fDepthDifference, fNormalDifference, fDepth, vNormal.xyz, vTexUV, float2(0, -1));
	Compare(fDepthDifference, fNormalDifference, fDepth, vNormal.xyz, vTexUV, float2(-1, 0));

	fDepthDifference = fDepthDifference * g_DepthMult;
	fDepthDifference = saturate(fDepthDifference);
	fDepthDifference = pow(fDepthDifference, g_DepthBias);

	fNormalDifference = fNormalDifference * g_NormalMult;
	fNormalDifference = saturate(fNormalDifference);
	fNormalDifference = pow(fNormalDifference, g_NormalBias);

	return (fNormalDifference + fDepthDifference);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////DOF///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float		g_fFocusDistance =50.f;
float		g_fFocusRange = 50.f;

vector DistanceDOF(vector ColorFocus, vector ColorBlurred, float Depth)
{
	float BlurFactor = saturate((Depth - g_fFocusDistance) / g_fFocusRange);
	return lerp(ColorFocus, ColorBlurred, BlurFactor);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////COLOR GRADING/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ¹à±â
vector Adjust_Brightness(vector vColor, float fBrightness)
{
	return vector(vColor.rgb += fBrightness, vColor.a);
}
// ´ëºñ
vector Adjust_Contrast(vector vColor, float fContrast)
{
	float t = 0.5f - fContrast * 0.5f;
	return vector(vColor.rgb * fContrast + t, vColor.a);
}
// ¿À¹ö·¹ÀÌ
vector Blend_Overlay(vector Target, vector Blend)
{
	return vector(((Target.rgb > 0.5) * (1 - (1 - 2 * (Target.rgb - 0.5)) * (1 - Blend.rgb)) + (Target.rgb <= 0.5) * ((2 * Target.rgb) * Blend.rgb)) * Blend.a + Target.rgb * (1 - Blend.a), Target.a);
}
// ¹à°Ô ÇÏ±â
vector Blend_Lighten(vector Target, vector Blend)
{
	return max(Target, Blend);
}
// ½ºÅ©¸°
vector Blend_Screen(vector Target, vector Blend)
{
	return vector((1 - (1 - Target.rgb) * (1 - Blend.rgb)) * Blend.a + Target.rgb * (1 - Blend.a), Target.a);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PS_IN
{
	float2 vTexUV : TEXCOORD0;
};

struct PS_OUT
{
	float4 vPostProcessing : COLOR;
};

PS_OUT PS_ALL_ON(PS_IN In)
{
	PS_OUT Out = (PS_OUT)0;

	vector vDepthInfo = tex2D(DepthSampler, In.vTexUV);
	vector vNormalInfo = tex2D(NormalSampler, In.vTexUV);
	vector vNormal = vector(vNormalInfo.xyz * 2.f - 1.f, 0.f);
	float fDepth = vDepthInfo.y * g_fFar;				// ViewZ
	float fWorldZ = vDepthInfo.x * fDepth;

	SOBEL OUTLINE 
float	fOutLine = 0.f;
if (vNormalInfo.a > 0.1)
fOutLine = Compute_OutLine(vNormal, fDepth, In.vTexUV);

vector ColorBlurred = tex2D(BlurSampler, In.vTexUV);
vector Color = tex2D(BlendSampler, In.vTexUV);
Color = DistanceDOF(Color, ColorBlurred, fDepth);

/BLOOM 
Color = (Color * g_fBloomStrength + ColorBlurred) / (g_fBloomStrength + 1.f) + (tex2D(LuminanceSampler, In.vTexUV));

 FINAL COLOR 
Color = lerp(Color, g_OutlineColor, fOutLine);

COLOR GRADING 
Color = Blend_Overlay(Color, g_vBlendColor);

Out.vPostProcessing = Color;

return Out;
}

PS_OUT PS_ALL_OFF(PS_IN In)
{
	PS_OUT Out = (PS_OUT)0;

	Out.vPostProcessing = tex2D(BlendSampler, In.vTexUV);

	return Out;
}

PS_OUT PS_DOF_OFF(PS_IN In)
{
	PS_OUT Out = (PS_OUT)0;

	vector vDepthInfo = tex2D(DepthSampler, In.vTexUV);
	vector vNormalInfo = tex2D(NormalSampler, In.vTexUV);
	vector vNormal = vector(vNormalInfo.xyz * 2.f - 1.f, 0.f);
	float fDepth = vDepthInfo.y * g_fFar;				// ViewZ
	float fWorldZ = vDepthInfo.x * fDepth;

	SOBEL OUTLINE 
	float	fOutLine = 0.f;
	if (vNormalInfo.a > 0.1)
		fOutLine = Compute_OutLine(vNormal, fDepth, In.vTexUV);
	vector Color = tex2D(BlendSampler, In.vTexUV);

	 BLOOM 
	Color = (Color * g_fBloomStrength + tex2D(BlurSampler, In.vTexUV)) / (g_fBloomStrength + 1.f) + (tex2D(LuminanceSampler, In.vTexUV));

	 FINAL COLOR 
	Color = lerp(Color, g_OutlineColor, fOutLine);

	 COLOR GRADING 
	Color = Blend_Overlay(Color, g_vBlendColor);

	Out.vPostProcessing = Color;

	return Out;
}

PS_OUT PS_LINK_ATTACK(PS_IN In)
{
	PS_OUT Out = (PS_OUT)0;

	vector vDepthInfo = tex2D(DepthSampler, In.vTexUV);
	vector vNormalInfo = tex2D(NormalSampler, In.vTexUV);
	vector vNormal = vector(vNormalInfo.xyz * 2.f - 1.f, 0.f);
	float fDepth = vDepthInfo.y * g_fFar;				// ViewZ
	float fWorldZ = vDepthInfo.x * fDepth;

	/* SOBEL OUTLINE 
	float	fOutLine = 0.f;
	if (vNormalInfo.a > 0.1)
		fOutLine = Compute_OutLine(vNormal, fDepth, In.vTexUV);

	vector ColorBlurred = tex2D(BlurSampler, In.vTexUV);
	vector Color = tex2D(BlendSampler, In.vTexUV);

	/* BLOOM 
	Color = (Color * g_fBloomStrength + ColorBlurred) / (g_fBloomStrength + 1.f) + (tex2D(LuminanceSampler, In.vTexUV));

	/* FINAL COLOR 
	Color = lerp(Color, g_OutlineColor, fOutLine);

	/* COLOR GRADING 
	Color = Blend_Overlay(Color, g_vBlendColor);

	/* LINK ATTACK 
	vector LinkColor = Adjust_Brightness(Color, -0.1 * (1.f - vDepthInfo.z));
	if (vDepthInfo.z < 0.5f)
		LinkColor = Adjust_Contrast(LinkColor, 1);

	Out.vPostProcessing = lerp(Color, LinkColor, g_LinkTime);

	return Out;
}
*/