#include "ObjectInstanceShaderHeader.hlsli"

Texture2D<float4> tex : register(t0); //0�ԃX���b�g�ɐݒ肳��ăe�N�X�`��
SamplerState smp : register(s0); //0�ԃX���b�g�ɐݒ肳�ꂽ�T���v���[

float4 main(VSOutput input) : SV_TARGET
{
	//return float4(1,1,1,1);

	//�e�N�X�`���̐F
    float4 texcolor = tex.Sample(smp, input.uv);
    return texcolor * cbuff0Data[input.instanceID].color;

}