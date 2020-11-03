#ifdef COLOR_PASS
	#define PS_OUT_COLOR0
#endif

#include "input_output.hlsli"

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

static const float PI = 3.14159265359;

float3 fresnelSchlick(float cosTheta, float3 F0) {
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(float3 N, float3 H, float roughness) {
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;
	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;
	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;
	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness) {
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	return ggx1 * ggx2;
}


PS_OUT mainPS(VS_OUT input) {
	PS_OUT output;

	#ifdef COLOR_PASS
		float3 albedo = float3(177, 227, 199) / 256.f;
		float metallic = 0;
		float roughness = 0.3;
		float ao = 0.1;

		float3 WorldPos = input.posW;

		float3 N = normalize(input.normalW);
		float3 V = normalize(gCamPos - WorldPos);
		float3 F0 = 0.04;
		F0 = lerp(F0, albedo, metallic);
		// reflectance equation
		float3 Lo = 0.0;

		for(int i = 0; i < 1; ++i) {
			// calculate per-light radiance
			// float3 L = normalize(lightPositions[i] - WorldPos);
			float3 L = normalize(-gDirection);
			float3 H = normalize(V + L);
			// float distance = length(lightPositions[i] - WorldPos);
			// float attenuation = 1.0 / (distance * distance);
			float attenuation = 1.0 * GetShadowFactor(input.posW, input.normalW);
			// float3 radiance = lightColors[i] * attenuation;
			float3 radiance = gColor * attenuation;
			// cook-torrance brdf
			float NDF = DistributionGGX(N, H, roughness);
			float G = GeometrySmith(N, V, L, roughness);
			float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
			float3 kS = F;
			float3 kD = float3(1, 1, 1) - kS;
			kD *= 1.0 - metallic;
			float3 numerator = NDF * G * F;
			float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
			float3 specular = numerator / max(denominator, 0.001);
			// add to outgoing radiance Lo
			float NdotL = max(dot(N, L), 0.0);
			Lo += (kD * albedo / PI + specular) * radiance * NdotL;
		}
		float3 ambient = 0.03 * albedo * ao;
		float3 color = ambient + Lo;
		color = color / (color + 1.0);
		color = pow(color, 1.0 / 2.2);

		output.color0 = float4(color, 1);

		// output.color0.rgb = GetShadowFactor(input.posW, input.normalW);
	#endif

	return output;
}
