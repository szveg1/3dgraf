Texture2D<float> inputTexture;
RWBuffer<float> outputData : register(u0);

[numthreads(64, 1, 1)]
void csReduceHorizontal(uint tid : SV_GroupIndex, uint3 groupIdx : SV_GroupID)
{
    // Load two elements from the texture and sum them
    float value1 = inputTexture.Load(int3(tid * 2, groupIdx.x, 0));
    float value2 = inputTexture.Load(int3(tid * 2 + 1, groupIdx.x, 0));
    float sum = value1 + value2;

    // Write the sum to the output buffer for debugging
    if (tid == 0)
    {
        outputData[groupIdx.x] = sum / 128.0;
    }
}