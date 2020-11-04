static const float PI = 3.14159265359;

#ifdef COLOR_PASS
	#define PS_OUT_COLOR0
#endif

#include "input_output.hlsli"
#include "pbr.hlsli"

cbuffer cbPass : register(b0) {
	float4x4 gVP;
	float4x4 gWorldToShadowMap;
	float3 gCamPos;
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
	if (any(shadowMapUV < 0) || any(shadowMapUV > 1) || pixelShadowMapDepth > 1) {
		return 1;
	}

	float bias = max(0.007 * (1 - dot(normalW, -gDirection)), 0.0015f);
	return gShadowMap.Sample(gPointSampler, shadowMapUV) > pixelShadowMapDepth - bias;
}

PS_OUT mainPS(VS_OUT input) {
	PS_OUT output;

	#ifdef COLOR_PASS
		float3 posW = input.posW;
		float3 normalW = normalize(input.normalW);

		float3 V = normalize(gCamPos - posW);

		Surface surf;
		surf.posW = posW;
		surf.normalW = normalW;
		surf.albedo = float3(177, 227, 199) / 256.f;
		surf.metallic = 0;
		surf.roughness = 0.1;

		float3 F0 = 0.04;
		F0 = lerp(F0, surf.albedo, surf.metallic);
		surf.F0 = F0;
		
		// reflectance equation
		float3 Lo = 0.0;
		for(int i = 0; i < 1; ++i) {
			// calculate per-light radiance
			// float3 L = normalize(lightPositions[i] - WorldPos);
			float3 L = normalize(-gDirection);
			// float distance = length(lightPositions[i] - WorldPos);
			// float attenuation = 1.0 / (distance * distance);
			float attenuation = 1.0 * GetShadowFactor(posW, normalW);
			float3 radiance = gColor * attenuation;

			Lo += SurfacePBRShade(surf, V, L, radiance);
		}

		const float ao = 0.5;
		float3 ambient = 0.03 * surf.albedo * ao;

		float3 color = ambient + Lo;

		color = color / (color + 1.0);
		color = pow(color, 1.0 / 2.2);

		output.color0 = float4(color, 1);

		// output.color0.rgb = GetShadowFactor(input.posW, input.normalW);
	#endif

	return output;
}
