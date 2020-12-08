#include "math.hlsli"

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

struct Surface {
	float3 posW;
	float3 normalW;
	float3 albedo;
	float roughness;
	float metallic;
	float3 F0;
};

float3 SurfacePBRShade(in Surface surf, float3 V, float3 L, float3 radiance) {
	float3 H = normalize(V + L);

	// cook-torrance brdf
	float NDF = DistributionGGX(surf.normalW, H, surf.roughness);
	float G = GeometrySmith(surf.normalW, V, L, surf.roughness);
	float3 F = fresnelSchlick(max(dot(H, V), 0.0), surf.F0);

	float3 kS = F;
	float3 kD = float3(1, 1, 1) - kS;
	kD *= 1.0 - surf.metallic;

	float3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(surf.normalW, V), 0.0) * max(dot(surf.normalW, L), 0.0);
	float3 specular = numerator / max(denominator, 0.001);

	// add to outgoing radiance Lo
	float NdotL = max(dot(surf.normalW, L), 0.0);
	return (kD * surf.albedo / PI + specular) * radiance * NdotL;
}
