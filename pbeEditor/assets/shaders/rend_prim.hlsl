#define VS_IN_POS_L
#define VS_IN_COLOR0

#define VS_OUT_COLOR0

#define PS_OUT_COLOR0

#include "input_output.hlsli"

cbuffer cbPass : register(b0) {
	float4x4 gVP;
}

VS_OUT mainVS(in VS_IN input) {
	VS_OUT output;

	float3 posW = input.posL;

	output.posH = mul(gVP, float4(posW, 1));
	output.color0 = input.color0;

	return output;
}

PS_OUT mainPS(VS_OUT input) {
	PS_OUT output;

	output.color0 = input.color0;

	return output;
}
