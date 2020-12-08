#define VS_IN_POS_L
#define VS_IN_NORMAL_L

#define VS_OUT_POS_W
#define VS_OUT_NORMAL_W

#ifdef COLOR_PASS
	#define PS_OUT_COLOR0
#endif

#include "input_output.hlsli"
#include "pbr.hlsli"

cbuffer cbPass : register(b0) {
	float4x4 gVP;
	float4x4 gWorldToShadowMap;
	float3 gCamPos;
	int gNumLights;
}

cbuffer cbModel : register(b1) {
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

float GetShadowFactor(float3 posW, float3 normalW, float3 L) {
	float3 shadowMapNDC = mul(gWorldToShadowMap, float4(posW, 1)).xyz; 
	float2 shadowMapUV = shadowMapNDC.xy;
	float pixelShadowMapDepth = shadowMapNDC.z;
	if (any(shadowMapUV < 0) || any(shadowMapUV > 1) || pixelShadowMapDepth > 1) {
		return 1;
	}

	float bias = max(0.007 * (1 - dot(normalW, L)), 0.0015f);
	return gShadowMap.Sample(gPointSampler, shadowMapUV) > pixelShadowMapDepth - bias;
}

static const int LIGHT_TYPE_DIRECT = 0;
static const int LIGHT_TYPE_POINT = 1;
static const int LIGHT_TYPE_SPOT = 2;

struct Light {
    float3 position;
	float radius;
    float3 color;
	float cutOff;
	int type;
	float3 up;
};

StructuredBuffer<Light> gLights : register(t1);

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
		for(int i = 0; i < gNumLights; ++i) {
			Light light = gLights[i];
			
			float3 L = normalize(-light.position);
			float attenuation = 1.0;

			if (light.type == LIGHT_TYPE_DIRECT) {
				L = normalize(-light.position);
				attenuation = 1.0 * GetShadowFactor(posW, normalW, L);
			} else if (light.type == LIGHT_TYPE_POINT) {
				L = normalize(light.position - posW);
				float distance = length(light.position - posW);
				attenuation = 1.0 / (distance * distance);
				attenuation *= smoothstep(1, 0, distance / light.radius);
			} else if (light.type == LIGHT_TYPE_SPOT) {
				L = normalize(light.position - posW);
				float distance = length(light.position - posW);
				attenuation = 1.0 / (distance * distance);
				attenuation *= smoothstep(1, 0, distance / light.radius);

				float3 lightDirection = normalize(light.up);
				float cutOffValue = dot(lightDirection, -L);
				float cutOffAttenuation = smoothstep(0, 1, saturate((cutOffValue - light.cutOff) / (1.0 - light.cutOff) * 3.0));

				attenuation *= cutOffAttenuation;
			}

			float3 radiance = light.color * attenuation;

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
