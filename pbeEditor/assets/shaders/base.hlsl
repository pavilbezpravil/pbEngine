#include "input_output.hlsli"

cbuffer cbPass : register(b0) {
	float4x4 gMVP;
}

float4 PosToWorldH(float3 posL) {
	return mul(gMVP, float4(posL, 1));
}

// float4 NormalToWorld(float3 normalL) {
// 	float4x4 normalMVP = 
// 	return mul(gMVP, float4(posL, 1));
// }

VS_OUT mainVS(in VS_IN input) {
	VS_OUT output;

	float4 posH = PosToWorldH(input.posL.xyz);
	float3 posW = posH.xyz;
	float3 normalW = input.normalL.xyz;

	output.posH = posH;
	output.posW = posW;
	output.normalW = normalW;

	return output;
}

PS_OUT mainPS(VS_OUT input) {
	PS_OUT output;

	float3 normalW = normalize(input.normalW);

	const float3 L = normalize(float3(1.2f, 1, 0.3f));
	float3 diffuse = float3(1, 0, 0);
	float nDotL = max(dot(L, normalW), 0.1f);
	output.color0 = float4(diffuse * nDotL, 1);

	return output;
}
