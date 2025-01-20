#include "billboard.hlsli"

Texture2D billboardTexture;
SamplerState ss;

float4 psFire(GsosBillboard input) : SV_Target
{
    float4 color = billboardTexture.Sample(ss, input.tex.xy);
    color.rgb = float3(color.a, pow(color.a, 4), pow(color.a, 10));
    color.a *= input.opacity;
    return color;
}



