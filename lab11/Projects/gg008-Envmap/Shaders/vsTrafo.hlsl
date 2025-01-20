#include "trafo.hlsli"
#include "perObject.hlsli"

VsosTrafo vsTrafo(IaosTrafo input)
{
	VsosTrafo output = (VsosTrafo)0;
	output.pos = mul(input.pos,
		modelViewProjMatrix);
	output.worldPos = mul(input.pos,
		modelMatrix);
	output.normal = mul(modelMatrixInverse,
		float4(input.normal.xyz, 0.0)).xyz;
	output.tex = input.tex;
	return output;
}
