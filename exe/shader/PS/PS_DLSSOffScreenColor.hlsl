#include "../Common.hlsli"

static const float PI = 3.1415926f;


PS_OUT_2 main(PS_IN input) : SV_TARGET
{
    PS_OUT_2 output;
	
    float4 color = (0, 0, 0, 1);
    
    // テクスチャからアルベド(色)情報をサンプリング
    color = g_Texture.Sample(g_SamplerState, input.uv);

	// テクスチャから法線ベクトルをサンプリングし正規化
    float3 N = normalize(g_Normal.Sample(g_SamplerState, input.uv).rgb);
	
	// テクスチャからワールド座標をサンプリング
    float3 worldPos = g_World.Sample(g_SamplerState, input.uv).rgb;
	
	// ピクセルの1点に対して、全ての点光源との明るさを計算する
    float3 L = normalize(-Light.Direction.xyz); // ライトの方向を正規化
    float diffuse = saturate(dot(N, L)); // 拡散反射光の計算(法線ベクトルとライトの方向の内積)
    
    color *= diffuse * Light.Diffuse / PI + Light.Ambient; //カラーに拡散反射光と環境光を適用
	
    output.OutputColor = color;
	
	return output;
}