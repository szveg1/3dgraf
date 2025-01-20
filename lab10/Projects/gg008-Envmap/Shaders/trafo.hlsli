struct IaosTrafo
{
	float4 pos	: POSITION;
	float3 normal 	: NORMAL;
	float2 tex 	: TEXCOORD;
};

struct VsosTrafo
{
	float4 pos 		: SV_POSITION;
	float4 worldPos 	: WORLDPOS;
	float3 normal 		: NORMAL;
	float2 tex 		: TEXCOORD;
};
