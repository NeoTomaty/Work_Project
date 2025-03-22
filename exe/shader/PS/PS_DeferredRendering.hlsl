#include "../Common.hlsli"

static const float PI = 3.1415926f;


float4 main(PS_IN input) : SV_TARGET
{
    float4 color = float4(0, 0, 0, 1);
    
    // DEFAULT出力
    if (TextureType == 0)
    {
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

    }
    // Color出力
    else if (TextureType == 1)
    {
        // カラーテクスチャをサンプリング
        color = g_Texture.Sample(g_SamplerState, input.uv);
    }
    // 法線出力
    else if (TextureType == 2)
    {
        color = g_Normal.Sample(g_SamplerState, input.uv);
        
    }
    // ワールド座標出力
    else if (TextureType == 3)
    {
        // カラーテクスチャをサンプリング
        color = g_World.Sample(g_SamplerState, input.uv);
    }
    // 深度情報出力
    else if (TextureType == 4)
    {
        // 深度テクスチャをサンプリング
        color = g_Depth.Sample(g_SamplerState, input.uv);
    }
        // 速度バッファ情報出力
    else if (TextureType == 5)
    {
        // 速度バッファテクスチャをサンプリング
        color = g_MVector.Sample(g_SamplerState, input.uv);
    }

    return color;
}