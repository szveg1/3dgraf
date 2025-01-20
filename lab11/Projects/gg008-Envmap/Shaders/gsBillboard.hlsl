#include "billboard.hlsli"
#include "perObject.hlsli"

[maxvertexcount(4)]
void gsBillboard(point VsosBillboard input[1], inout TriangleStream<GsosBillboard> stream) {
    float4 hndcPos = mul(float4(input[0].pos, 1), modelViewProjMatrix);
    GsosBillboard output;
    
    float s = input[0].age / input[0].lifespan;
    
    output.opacity = 1 - abs(s * 2 - 1);
    
    output.pos = hndcPos;
    output.pos.x += billboardSize.x * s;
    output.pos.y += billboardSize.y * s;
    output.tex = float2(1, 0);
    stream.Append(output);
    output.pos = hndcPos;
    output.pos.x += billboardSize.x * s;
    output.pos.y -= billboardSize.y * s;
    output.tex = float2(1, 1);
    stream.Append(output);
    output.pos = hndcPos;
    output.pos.x -= billboardSize.x * s;
    output.pos.y += billboardSize.y * s;
    output.tex = float2(0, 0);
    stream.Append(output);
    output.pos = hndcPos;
    output.pos.x -= billboardSize.x * s;
    output.pos.y -= billboardSize.y * s;
    output.tex = float2(0, 1);
    stream.Append(output);
}