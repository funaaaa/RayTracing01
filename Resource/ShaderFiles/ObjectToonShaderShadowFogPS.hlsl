#include "ObjectToonShadowShaderHeader.hlsli"

Texture2D<float4> tex : register(t0); //0�ԃX���b�g�ɐݒ肳��ăe�N�X�`��
SamplerState smp : register(s0); //0�ԃX���b�g�ɐݒ肳�ꂽ�T���v���[

//���� �e�N�X�`���̐F �t�H�O�̐F �t�H�O�̔Z��
float4 CaluFog(float distance, float4 color, float4 fogColor, float density)
{
    float e = 2.71828f;
    float fogFactor = pow(e, -distance * density);
    //�t�H�O��
    fogFactor = saturate(fogFactor);

    //�t�H�O�̐F
    float4 fog = fogFactor * color + (1 - fogFactor) * fogColor;

    return fog;
}

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
            shadeColor.rgb += outputColor / 4.0f;
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
    for (int i = 0; i < CIRCLESHADOW_NUM; ++i)
    {
        if (circleShadow[i].active == 1)
        {
    	    //�I�u�W�F�N�g�\�ʂ���CASTER�ւ̃x�N�g��
            //float3 casterv = circleShadow[i].casterpos - input.worldpos.xyz;
            ////���e�����ł̋���
            //float d = dot(casterv, circleShadow[i].dir);
            ////���������W��
            //float atten = saturate(1.0f / (circleShadow[i].atten.x +
            //circleShadow[i].atten.y * d +
            //circleShadow[i].atten.z * (d * d)));
            ////�������}�C�i�X�Ȃ�0�ɂ���
            //atten *= step(0, d);
            ////���z���C�g�̍��W
            //float3 lightpos = circleShadow[i].casterpos + circleShadow[i].dir * circleShadow[i].distanceCasterLight;
            ////�I�u�W�F�N�g�\�ʂ��烉�C�g�ւ̃x�N�g��(�P�ʃx�N�g��)
            //float3 lightv = normalize(lightpos - input.worldpos.xyz);
            ////�p�x����
            //float cos = dot(lightv, circleShadow[i].dir);
            ////�����J�n�p�x���猸���I���p�x�ɂ����Č���
            ////�����J�n�p�x�̓�����1�{ �����I���p�x�̊O����0�x�̋P�x
            //float angleatten = smoothstep(circleShadow[i].factorAngleCos.y, circleShadow[i].factorAngleCos.x, cos);
            ////�p�x��������Z
            //atten *= angleatten;
            ////���ׂĂ����Z����
            //shadeColor.rgb -= atten;

            float dis = distance(circleShadow[i].casterpos, input.worldpos.xyz);

            if (dis <= circleShadow[i].dir.x)
            {
                shadeColor.rgb = float3(0, 0, 0);

            }

        }
    }

    //�Ԑ���
    float buff = 0.35f;
    float buff2 = 0.65f;
    float buff3 = 1.0f;
    if (shadeColor.r < buff)
    {
        shadeColor.r = buff;
    }
    else if (shadeColor.r < buff2)
    {
        shadeColor.r = buff2;
    }
    else
    {
        shadeColor.r = buff3;

    }

    //�ΐ���
    if (shadeColor.g < buff)
    {
        shadeColor.g = buff;
    }
    else if (shadeColor.g < buff2)
    {
        shadeColor.g = buff2;
    }
    else
    {
        shadeColor.g = buff3;
    }

    //����
    if (shadeColor.b < buff)
    {
        shadeColor.b = buff;
    }
    else if (shadeColor.b < buff2)
    {
        shadeColor.b = buff2;
    }
    else
    {
        shadeColor.b = buff3;
    }

	//�e�N�X�`���̐F
    float4 texcolor = tex.Sample(smp, input.uv);
    float4 endColor = (shadeColor * texcolor) * color;

    float4 fogColor = float4(0.00f,0.00f, 0.00f, 1.0f);
    float mitudo = 0.0003;

    return CaluFog(input.svpos.a, endColor, fogColor, mitudo);

}