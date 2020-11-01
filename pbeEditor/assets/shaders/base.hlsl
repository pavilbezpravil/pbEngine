#ifdef COLOR_PASS
	#define PS_OUT_COLOR0
#endif

#include "input_output.hlsli"

cbuffer cbPass : register(b0) {
	float4x4 gVP;
	float4x4 gWorldToShadowMap;
}

cbuffer cbDirectionLight : register(b1) {
	float3 gDirection;
	float3 gColor;
}

cbuffer cbModel : register(b2) {
	float4x4 gTransform;
	float4x4 gNormalTransform;
}

Texture2D<float> gShadowMap : register(t0);
SamplerState gPointSampler : register(s0);

float3 PosToWorld(float3 posL) {
	return mul(gTransform, float4(posL, 1)).xyz;
}

float3 NormalToWorld(float3 normalL) {
	return mul(gNormalTransform, float4(normalL, 1)).xyz;
}

VS_OUT mainVS(in VS_IN input) {
	VS_OUT output;

	float3 posW = PosToWorld(input.posL);
	float3 normalW = NormalToWorld(input.normalL);

	output.posH = mul(gVP, float4(posW, 1));
	output.posW = posW;
	output.normalW = normalW;

	return output;
}

float GetShadowFactor(float3 posW, float3 normalW) {
	float3 shadowMapNDC = mul(gWorldToShadowMap, float4(posW, 1)).xyz; 
	float2 shadowMapUV = shadowMapNDC.xy;
	float pixelShadowMapDepth = shadowMapNDC.z;
	shadowMapUV *= float2(0.5, -0.5); // todo: make it in c++
	shadowMapUV = (shadowMapUV + 0.5);
	if (any(shadowMapUV < 0) || any(shadowMapUV > 1) || pixelShadowMapDepth > 1) {
		return 1;
	}

	float bias = max(0.007 * (1 - dot(normalW, -gDirection)), 0.0015f);
	return 1 - (gShadowMap.Sample(gPointSampler, shadowMapUV) < pixelShadowMapDepth - bias);
}

PS_OUT mainPS(VS_OUT input) {
	PS_OUT output;

	#ifdef COLOR_PASS
		float3 normalW = normalize(input.normalW);

		const float3 L = normalize(-gDirection);
		float3 diffuse = float3(177, 227, 199) / 256.f;
		float nDotL = max(dot(L, normalW), 0);
		float ambient = 0.15f;
		output.color0 = float4(diffuse * (nDotL * GetShadowFactor(input.posW, input.normalW) + ambient), 1);

		// output.color0.rgb = GetShadowFactor(input.posW, input.normalW);
	#endif

	return output;
}
