RWBuffer<float> inputData : register(u0);
RWBuffer<float> result : register(u1);

#define groupSize 64
groupshared float sdata[groupSize];


[numthreads(groupSize, 1, 1)]
void csReduceVertical(uint threadIdx : SV_GroupIndex, uint3 groupIdx : SV_GroupID)
{
    unsigned int tid = threadIdx.x;
    unsigned int i = groupIdx.x * groupSize + threadIdx.x;
    sdata[tid] = inputData[i];
    GroupMemoryBarrierWithGroupSync();
    
    if (tid < 32)
        sdata[tid] += sdata[tid + 32];
    if (tid < 16)
        sdata[tid] += sdata[tid + 16];
    if (tid < 8)
        sdata[tid] += sdata[tid + 8];
    if (tid < 4)
        sdata[tid] += sdata[tid + 4];
    if (tid < 2)
        sdata[tid] += sdata[tid + 2];
    if (tid < 1)
        sdata[tid] += sdata[tid + 1];
    
    if (tid == 0)
    {
        result[groupIdx.x] = sdata[0]; // Store the final result
    }
}