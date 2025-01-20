#include "quad.hlsli"

Texture2D showTexture;
SamplerState ss;
Buffer<float> toneData;

float4 psShow(VsosQuad input) : SV_Target
{
    float2 texCoord = float2(1 - input.tex.x, 1 - input.tex.y);
    
    float reductionResult = toneData.Load(0);
    
    float3 texColor = showTexture.Sample(ss, texCoord).rgb / reductionResult ;
    
    return float4(texColor, reductionResult);
}