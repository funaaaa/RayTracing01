
// 入力情報
RWTexture2D<float4> InputImg : register(u0);

// 出力先UAV  
RWTexture2D<float4> OutputImg : register(u1);

float4 GetPixelColor(int x, int y, int2 texSize)
{
    x = clamp(x, 0, texSize.x);
    y = clamp(y, 0, texSize.y);

    return InputImg[uint2(x, y)];
}

[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    
    // バイリニアフィルタをかける
    uint2 basepos = uint2(DTid.x / 1, DTid.y / 1);
    float2 texSize = float2(1280 / 1.0f, 720 / 1.0f);
    float4 color = GetPixelColor(basepos.x, basepos.y, texSize);

    // 加重平均を取る
    //color /= 4.0f;
    OutputImg[DTid.xy] = color;
    
}