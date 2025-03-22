#include "../Common.hlsli"

static const float PI = 3.1415926f; // π

// フレネル項を計算
float3 FresnelSchlick(float3 F0, float dotVH)
{
    return F0 + (1.0 - F0) * pow(1.0 - dotVH, 5.0);
}

// ディストリビューション関数
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

// 幾何項
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

// 幾何項の融合
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// ピクセルシェーダー
float4 main(PS_IN input) : SV_Target
{

    // ライトの有無
    if (Light.Enable)
    {
          // 法線マップのサンプリング
        float3 normalMapSample = g_Normal.Sample(g_SamplerState, input.uv).rgb;
        float3 normal = normalize(input.normal * 2.0 - 1.0); // 入力法線を[-1,1]に変換
        normal = normalize(normalMapSample * 2.0 - 1.0); // 法線マップの変換

    // 法線をTBN空間に適用
        float3x3 TBN = float3x3(input.tangent, input.biNormal, input.normal);
        normal = normalize(mul(normal, TBN));

    // 視線ベクトル
        float3 V = normalize(eyePos - input.worldPos.xyz);

    // ライト方向
        float3 L = normalize(Light.Position - input.worldPos.xyz);

    // ハーフベクトル
        float3 H = normalize(V + L);

    // テクスチャから基底色、メタリック、粗さを取得
        float3 albedo = g_Texture.Sample(g_SamplerState, input.uv).rgb;
        float metallic = 0.4f;
        float roughness = 0.6f;

    // 環境反射率
        float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);

    // フレネル項
        float3 F = FresnelSchlick(F0, max(dot(H, V), 0.0));

    // ディストリビューション
        float D = DistributionGGX(normal, H, roughness);

    // 幾何項
        float G = GeometrySmith(normal, V, L, roughness);

    // Cook-Torrance BRDF
        float3 numerator = D * G * F;
        float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, L), 0.0) + 0.001; // 0割防止
        float3 specular = numerator / denominator;

    // 拡散反射項（Lambertian）
        float3 kS = F; // スペキュラ成分
        float3 kD = 1.0 - kS; // 拡散成分
        kD *= 1.0 - metallic; // メタリックによる補正

    // 照明計算
        float NdotL = max(dot(normal, L), 0.0);
        float3 Lo = (kD * albedo / PI + specular) * Light.Diffuse.rgb * NdotL;

    // 環境光（簡易的なIBL表現を追加可能）
        float3 ambient = Light.Ambient.rgb * albedo;

    // 出力色
        float3 color = ambient + Lo;

        return float4(color, 1.0);
    }
    else
    {
        float4 color = g_Texture.Sample(g_SamplerState, input.uv);
        return color;
    }
    
}
