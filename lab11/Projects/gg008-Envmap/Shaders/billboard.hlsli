struct IaosBillboard {
    float3 pos : POSITION;
    float lifespan : LIFESPAN;
    float age : AGE;
};
typedef IaosBillboard VsosBillboard;
struct GsosBillboard {
    float4 pos : SV_Position;
    float2 tex : TEXCOORD;
    float opacity : OPACITY;
};
cbuffer perSwapChain{
  float4 billboardSize;
}
