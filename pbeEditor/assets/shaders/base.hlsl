#include "input_output.hlsli"

cbuffer cbPass : register(b0) {
	float4x4 gMVP;
}

cbuffer cbModel : register(b1) {
	float4x4 gTransform;
}

float3 PosToWorld(float3 posL) {
	return mul(gTransform, float4(posL, 1)).xyz;
}

VS_OUT mainVS(in VS_IN input) {
	VS_OUT output;

	float3 posW = PosToWorld(input.posL.xyz);
	float3 normalW = input.normalL.xyz;

	output.posH = mul(gMVP, float4(posW, 1));
	output.posW = posW;
	output.normalW = normalW;

	return output;
}

PS_OUT mainPS(VS_OUT input) {
	PS_OUT output;

	float3 normalW = normalize(input.normalW);

	const float3 L = normalize(float3(1.2f, 1, 0.3f));
	float3 diffuse = float3(0, 1, 1);
	float nDotL = max(dot(L, normalW), 0.1f);
	output.color0 = float4(diffuse * nDotL, 1);

	return output;
}
