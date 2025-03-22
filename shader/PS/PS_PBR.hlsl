#include "../Common.hlsli"

static const float PI = 3.1415926f; // ��

// �t���l�������v�Z
float3 FresnelSchlick(float3 F0, float dotVH)
{
    return F0 + (1.0 - F0) * pow(1.0 - dotVH, 5.0);
}

// �f�B�X�g���r���[�V�����֐�
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

// �􉽍�
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

// �􉽍��̗Z��
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// �s�N�Z���V�F�[�_�[
float4 main(PS_IN input) : SV_Target
{

    // ���C�g�̗L��
    if (Light.Enable)
    {
          // �@���}�b�v�̃T���v�����O
        float3 normalMapSample = g_Normal.Sample(g_SamplerState, input.uv).rgb;
        float3 normal = normalize(input.normal * 2.0 - 1.0); // ���͖@����[-1,1]�ɕϊ�
        normal = normalize(normalMapSample * 2.0 - 1.0); // �@���}�b�v�̕ϊ�

    // �@����TBN��ԂɓK�p
        float3x3 TBN = float3x3(input.tangent, input.biNormal, input.normal);
        normal = normalize(mul(normal, TBN));

    // �����x�N�g��
        float3 V = normalize(eyePos - input.worldPos.xyz);

    // ���C�g����
        float3 L = normalize(Light.Position - input.worldPos.xyz);

    // �n�[�t�x�N�g��
        float3 H = normalize(V + L);

    // �e�N�X�`��������F�A���^���b�N�A�e�����擾
        float3 albedo = g_Texture.Sample(g_SamplerState, input.uv).rgb;
        float metallic = 0.4f;
        float roughness = 0.6f;

    // �����˗�
        float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);

    // �t���l����
        float3 F = FresnelSchlick(F0, max(dot(H, V), 0.0));

    // �f�B�X�g���r���[�V����
        float D = DistributionGGX(normal, H, roughness);

    // �􉽍�
        float G = GeometrySmith(normal, V, L, roughness);

    // Cook-Torrance BRDF
        float3 numerator = D * G * F;
        float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, L), 0.0) + 0.001; // 0���h�~
        float3 specular = numerator / denominator;

    // �g�U���ˍ��iLambertian�j
        float3 kS = F; // �X�y�L��������
        float3 kD = 1.0 - kS; // �g�U����
        kD *= 1.0 - metallic; // ���^���b�N�ɂ��␳

    // �Ɩ��v�Z
        float NdotL = max(dot(normal, L), 0.0);
        float3 Lo = (kD * albedo / PI + specular) * Light.Diffuse.rgb * NdotL;

    // �����i�ȈՓI��IBL�\����ǉ��\�j
        float3 ambient = Light.Ambient.rgb * albedo;

    // �o�͐F
        float3 color = ambient + Lo;

        return float4(color, 1.0);
    }
    else
    {
        float4 color = g_Texture.Sample(g_SamplerState, input.uv);
        return color;
    }
    
}
