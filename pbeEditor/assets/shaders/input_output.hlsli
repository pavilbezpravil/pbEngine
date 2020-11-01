struct VS_IN {
	float3 posL : POSITION0;
	float3 normalL : NORMAL0;
};

struct VS_OUT {
	float4 posH : SV_POSITION;
	float3 posW : POSITION0;
	float3 normalW : NORMAL0;
};

struct PS_OUT {
	float4 color0 : SV_TARGET0;
};
