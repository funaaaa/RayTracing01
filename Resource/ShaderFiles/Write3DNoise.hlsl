
// 出力先UAV
RWTexture3D<float4> OutputImg : register(u0);

[numthreads(8, 8, 4)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    
    // 色を保存。
    OutputImg[DTid.xyz] = float4(1,1,1,1);
    
}