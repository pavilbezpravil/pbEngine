struct VS_IN {
	#ifdef VS_IN_POS_L
		float3 posL : POSITION0;
	#endif
	#ifdef VS_IN_NORMAL_L
		float3 normalL : NORMAL0;
	#endif
	#ifdef VS_IN_COLOR0
		float4 color0 : COLOR0;
	#endif
};

struct VS_OUT {
	float4 posH : SV_POSITION;
	#ifdef VS_OUT_POS_W
		float3 posW : POSITION0;
	#endif
	#ifdef VS_OUT_NORMAL_W
		float3 normalW : NORMAL0;
	#endif
	#ifdef VS_OUT_COLOR0
		float4 color0 : COLOR0;
	#endif
};

struct PS_OUT {
	#ifdef PS_OUT_COLOR0
		float4 color0 : SV_TARGET0;
	#endif
	#ifdef PS_OUT_COLOR1
		float4 color1 : SV_TARGET1;
	#endif
};
