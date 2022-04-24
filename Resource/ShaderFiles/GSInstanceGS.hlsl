#include "GSInstanceShaderHeader.hlsli"

//���_��
static const uint vnum = 24;

//�^�񒆂���̃I�t�Z�b�g
static const float4 offsetArray[vnum] =
{
    float4(-1.0f, -1.0f, -1.0f, 0), //�O�ʍ���
	float4(-1.0f, 1.0f, -1.0f, 0), //�O�ʍ���
	float4(1.0f, -1.0f, -1.0f, 0), //�O�ʉE��
	float4(1.0f, 1.0f, -1.0f, 0), //�O�ʉE��

	float4(1.0f, -1.0f, 1.0f, 0), //���ʉE��
	float4(1.0f, 1.0f, 1.0f, 0), //���ʉE��
	float4(-1.0f, -1.0f, 1.0f, 0), //���ʍ���
	float4(-1.0f, 1.0f, 1.0f, 0), //���ʍ���

	float4(1.0f, -1.0f, -1.0f, 0), //�E�ʍ���
	float4(1.0f, 1.0f, -1.0f, 0), //�E�ʍ���
	float4(1.0f, -1.0f, 1.0f, 0), //�E�ʉE��
	float4(1.0f, 1.0f, 1.0f, 0), //�E�ʉE��

	float4(-1.0f, -1.0f, 1.0f, 0), //���ʍ���
	float4(-1.0f, 1.0f, 1.0f, 0), //���ʍ���
	float4(-1.0f, -1.0f, -1.0f, 0), //���ʉE��
	float4(-1.0f, 1.0f, -1.0f, 0), //���ʉE��

	float4(-1.0f, 1.0f, -1.0f, 0), //��ʍ���
	float4(-1.0f, 1.0f, 1.0f, 0), //��ʍ���
	float4(1.0f, 1.0f, -1.0f, 0), //��ʉE��
	float4(1.0f, 1.0f, 1.0f, 0), //��ʉE��

	float4(-1.0f, -1.0f, 1.0f, 0), //���ʍ���
	float4(-1.0f, -1.0f, -1.0f, 0), //���ʍ���
	float4(1.0f, -1.0f, 1.0f, 0), //���ʉE��
	float4(1.0f, -1.0f, -1.0f, 0), //���ʉE��
};

//uv�̃I�t�Z�b�g
static const float2 offsetUVArray[vnum] =
{
    float2(0, 1), //�O�ʍ���
	float2(0, 0), //�O�ʍ���
	float2(1, 1), //�O�ʉE��
	float2(1, 0), //�O�ʉE��

	float2(0, 1), //���ʍ���
	float2(0, 0), //���ʍ���
	float2(1, 1), //���ʉE��
	float2(1, 0), //���ʉE��

	float2(0, 1), //�E�ʍ���
	float2(0, 0), //�E�ʍ���
	float2(1, 1), //�E�ʉE��
	float2(1, 0), //�E�ʉE��

	float2(0, 1), //���ʍ���
	float2(0, 0), //���ʍ���
	float2(1, 1), //���ʉE��
	float2(1, 0), //���ʉE��

	float2(0, 1), //��ʍ���
	float2(0, 0), //��ʍ���
	float2(1, 1), //��ʉE��
	float2(1, 0), //��ʉE��

	float2(0, 1), //���ʍ���
	float2(0, 0), //���ʍ���
	float2(1, 1), //���ʉE��
	float2(1, 0), //���ʉE��
};

[maxvertexcount(vnum)]
void main(
	point VSOutput input[1] : SV_POSITION,
	inout TriangleStream<GSOutput> output
)
{
    GSOutput element;
	//�S���_���܂킷
    for (uint i = 0; i < 4; i++)
    {
        GSOutput element;
        element.svpos = input[0].pos + offsetArray[i];

        element.svpos = mul(cBuffer[input[0].id].world, element.svpos);

        //element.uv = float2(noise(0.5f), noise(0.5f));
        //element.uv = float2(noise(i / 2.0f / vnum), noise(i / 2.0f / vnum));
        //element.uv = float2(0.5f, 0.5f);
        element.uv = offsetUVArray[i];

        element.id = input[0].id;

        output.Append(element);
    }
    output.RestartStrip();
    for (i = 4; i < 8; i++)
    {
        GSOutput element;
        element.svpos = input[0].pos + offsetArray[i];

        element.svpos = mul(cBuffer[input[0].id].world, element.svpos);

        //element.uv = float2(noise(0.5f), noise(0.5f));
        //element.uv = float2(noise(i / 2.0f / vnum), noise(i / 2.0f / vnum));
        //element.uv = float2(0.5f, 0.5f);
        element.uv = offsetUVArray[i];

        element.id = input[0].id;

        output.Append(element);
    }
    output.RestartStrip();
    for (i = 8; i < 12; i++)
    {
        GSOutput element;
        element.svpos = input[0].pos + offsetArray[i];

        element.svpos = mul(cBuffer[input[0].id].world, element.svpos);

        //element.uv = float2(noise(0.5f), noise(0.5f));
        //element.uv = float2(noise(i / 2.0f / vnum), noise(i / 2.0f / vnum));
        //element.uv = float2(0.5f, 0.5f);
        element.uv = offsetUVArray[i];

        element.id = input[0].id;

        output.Append(element);
    }
    output.RestartStrip();
    for (i = 12; i < 16; i++)
    {
        GSOutput element;
        element.svpos = input[0].pos + offsetArray[i];

        element.svpos = mul(cBuffer[input[0].id].world, element.svpos);

        //element.uv = float2(noise(0.5f), noise(0.5f));
        //element.uv = float2(noise(i / 2.0f / vnum), noise(i / 2.0f / vnum));
        //element.uv = float2(0.5f, 0.5f);
        element.uv = offsetUVArray[i];

        element.id = input[0].id;

        output.Append(element);
    }
    output.RestartStrip();
    for (i = 16; i < 20; i++)
    {
        GSOutput element;
        element.svpos = input[0].pos + offsetArray[i];

        element.svpos = mul(cBuffer[input[0].id].world, element.svpos);

        //element.uv = float2(noise(0.5f), noise(0.5f));
        //element.uv = float2(noise(i / 2.0f / vnum), noise(i / 2.0f / vnum));
        //element.uv = float2(0.5f, 0.5f);
        element.uv = offsetUVArray[i];

        element.id = input[0].id;

        output.Append(element);
    }
    output.RestartStrip();
    for (i = 20; i < 24; i++)
    {
        GSOutput element;
        element.svpos = input[0].pos + offsetArray[i];

        element.svpos = mul(cBuffer[input[0].id].world, element.svpos);

        //element.uv = float2(noise(0.5f), noise(0.5f));
        //element.uv = float2(noise(i / 2.0f / vnum), noise(i / 2.0f / vnum));
        //element.uv = float2(0.5f, 0.5f);
        element.uv = offsetUVArray[i];

        element.id = input[0].id;

        output.Append(element);
    }
    output.RestartStrip();

}



//�S���_���w�肵�ĕ`�悵�Ă݂�