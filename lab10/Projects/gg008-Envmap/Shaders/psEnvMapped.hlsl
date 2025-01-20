#include "trafo.hlsli"
#include "perFrame.hlsli"

TextureCube envTexture;
Texture2D kd;
SamplerState ss;


float4 psEnvMapped(VsosTrafo input) : SV_Target{
	
	float3 reflDir = reflect(normalize(input.worldPos.xyz - eyePos.xyz), normalize(input.normal));
	return envTexture.Sample(ss, reflDir) * 0.5+
		kd.Sample(ss, input.tex) * 0.5;
	}