#include "ObjectToonShaderHeader.hlsli"

Texture2D<float4> tex : register(t0); //0�ԃX���b�g�ɐݒ肳��ăe�N�X�`��
Texture2D<float4> palette : register(t1); //1�ԃX���b�g�ɐݒ肳��ăe�N�X�`��
SamplerState smp : register(s0); //0�ԃX���b�g�ɐݒ肳�ꂽ�T���v���[

float4 main(VSOutput input) : SV_TARGET
{
	//return float4(1,1,1,1);

    float4 shadeColor = float4(0, 0, 0, 1); //�ŏI�I�ɕ`�悷��e�̐F

	//���s����
    for (int i = 0; i < DIRECTIONLIGHT_NUM; ++i)
    {
        if (dirLights[i].active == 1)
        {
			//�����ˌ��̌v�Z
            float3 ambient = mAmbient;
			//�g�U���ˌ��̌v�Z
            float3 diffuse = dot(-dirLights[i].lightv, input.normal) * mDiffuse;
			//����x
            const float shininess = 4.0f;
			//���_���王�_�ւ̕����x�N�g��
            float3 eyedir = normalize(eye - input.worldpos.xyz);
			//���ˌ��x�N�g��
            float3 reflect = normalize(dirLights[i].lightv + 2 * dot(-dirLights[i].lightv, input.normal) * input.normal);
			//���ʔ��ˌ�
            float3 specular = pow(saturate(dot(reflect, eyedir)), shininess) * mSpecular;
			//���ׂĉ��Z�����F
            float3 color = (ambient + diffuse + specular) * dirLights[i].lightcolor;
            float3 outputColor = color;
            shadeColor.rgb += outputColor;
        }
    }
	//�_����
    for (int i = 0; i < POINTLIGHT_NUM; ++i)
    {
        if (pointLights[i].active == 1)
        {
			//���C�g�̃x�N�g��
            float3 lightv = pointLights[i].lightpos - input.worldpos.xyz;
			//�x�N�g���̒���
            float d = length(lightv);
			//���K�����ĒP�ʃx�N�g���ɂ���
            lightv = normalize(lightv);
			//���������W��
            float atten = 1.0f / (pointLights[i].lightatten.x +
				pointLights[i].lightatten.y * d +
				pointLights[i].lightatten.z * (d * d));
            atten *= 1000;
			//���C�g�Ɍ������x�N�g���Ɩ@���̓���
            float3 dotlightnormal = dot(lightv, input.normal);
			//���ˌ��x�N�g��
            float3 reflect = normalize(-lightv + 2 * dotlightnormal * input.normal);
			//�g�U���ˌ�
            float3 diffuse = dotlightnormal * mDiffuse;
			//����x
            const float shininess = 4.0f;
			//���_���王�_�ւ̕����x�N�g��
            float3 eyedir = normalize(eye - input.worldpos.xyz);
			//���ʔ��ˌ�
            float3 specular = pow(saturate(dot(reflect, eyedir)), shininess) * mSpecular;
			//���ׂĉ��Z�����F
            float3 outputColor = saturate(atten * (diffuse + specular) * pointLights[i].lightcolor);
            shadeColor.rgb += outputColor;
        }
    }
	//�X�|�b�g���C�g
   // for (int i = 0; i < SPOTLIGHT_NUM; ++i)
   // {
   //     if (spotlights[i].active == 1)
   //     {
			////���C�g�ւ̕����x�N�g��
   //         float3 lightv = spotlights[i].lightpos - input.worldpos.xyz;
   //         float d = length(lightv);
   //         lightv = normalize(lightv);
			////���������W��
   //         float atten = saturate(1.0f / (spotlights[i].lightatten.x +
			//	spotlights[i].lightatten.y * d +
			//	spotlights[i].lightatten.z * d * d));
			////�p�x����
   //         float cos = dot(lightv, spotlights[i].lightv);
			////�����J�n�p�x����A�����I���p�x�ɂ����Č���
			////�����J�n�p�x�̓�����1�{ �����I���p�x�̊O����0�{�̋P�x
   //         float angleatten = smoothstep(spotlights[i].lightfactoranglecos.y, spotlights[i].lightfactoranglecos.x, -cos);
			////�p�x��������Z
   //         atten *= angleatten;
			////���C�g�Ɍ������x�N�g���Ɩ@���̓���
   //         float3 dotlightnormal = dot(lightv, input.normal);
			////���_���王�_�ւ̕����x�N�g��
   //         float3 eyedir = normalize(eye - input.worldpos.xyz);
			////����x
   //         const float shininess = 4.0f;
			////���ˌ��x�N�g��
   //         float3 reflect = normalize(-lightv + 2 * dotlightnormal * input.normal);
			////�g�U���ˌ�
   //         float3 diffuse = dotlightnormal * mDiffuse;
			////���ʔ��ˌ�
   //         float3 specular = pow(saturate(dot(reflect, eyedir)), shininess) * mSpecular;
			////���ׂĉ��Z
   //         float3 outputColor = atten * (diffuse + specular) * spotlights[i].lightcolor;
   //         shadeColor.rgb += outputColor;
   //     }
    //}
    
    //�ۉe�̏���
    //for (int i = 0; i < CIRCLESHADOW_NUM; ++i)
    //{
    //    if (circleShadow[i].active == 1)
    //    {
    //	//���C�g�ւ̕����x�N�g��
    //        float3 lightv = circleShadow[i].casterpos - input.worldpos.xyz;
    //        lightv.g += circleShadow[i].distanceCasterLight;
    //        float d = length(lightv);
    //        lightv = normalize(lightv);
    //	//���������W��
    //        float atten = saturate(1.0f / (circleShadow[i].atten.x +
    //		circleShadow[i].atten.y * d +
    //		circleShadow[i].atten.z * d * d));
    //	//�p�x����
    //        float cos = dot(lightv, circleShadow[i].dir);
    //	//�����J�n�p�x����A�����I���p�x�ɂ����Č���
    //	//�����J�n�p�x�̓�����1�{ �����I���p�x�̊O����0�{�̋P�x
    //        float angleatten = smoothstep(circleShadow[i].factorAngleCos.y, circleShadow[i].factorAngleCos.x, -cos);
    //	//�p�x��������Z
    //        atten *= angleatten;
    //	//���C�g�Ɍ������x�N�g���Ɩ@���̓���
    //        float3 dotlightnormal = dot(lightv, input.normal);
    //	//���_���王�_�ւ̕����x�N�g��
    //        float3 eyedir = normalize(eye - input.worldpos.xyz);
    //	//����x
    //        const float shininess = 4.0f;
    //	//���ˌ��x�N�g��
    //        float3 reflect = normalize(-lightv + 2 * dotlightnormal * input.normal);
    //	//�g�U���ˌ�
    //        float3 diffuse = dotlightnormal * mDiffuse;
    //	//���ʔ��ˌ�
    //        float3 specular = pow(saturate(dot(reflect, eyedir)), shininess) * mSpecular;
    //	//���ׂĉ��Z
    //        float3 outputColor = atten * (diffuse + specular) * spotlights[i].lightcolor;
    //        shadeColor.rgb -= outputColor;
    //    }
    //}
    
    //�e�̐F��RGB�̕��ς����߂�
    float shadeColorU = (shadeColor.r + shadeColor.g + shadeColor.b) / 3.0f;
    
    //�ŏI�I�ȃ��C�e�B���O�̐F����u�̒l�����߂�
    float4 paletteColor = palette.Sample(smp, float2(shadeColorU, 0.5f));

	//�e�N�X�`���̐F
    float4 texcolor = tex.Sample(smp, input.uv);
    return (paletteColor * texcolor) * color;

}