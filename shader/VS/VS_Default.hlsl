#include "../Common.hlsli"


PS_IN main( VS_IN input )
{
    PS_IN output;
	
    float4 pos = float4(input.pos.xyz, 1.0f); // 頂点座標（x,y,z）を取得し、wを1.0fにする
    
    // 現在フレームの頂点座標
    output.pos = mul(pos, World);                   // ワールド行列と乗算
    output.worldPos = float4(output.pos.xyz, 1.0f); // ワールド空間での頂点位置を保存
    output.pos = mul(output.pos, View);             // ビュー行列と乗算
    output.viewPos = output.pos;                    // ビュー空間での頂点位置を保存
    output.pos = mul(output.pos, Proj);             // プロジェクション行列と乗算（クリップ空間）
    
    // 前フレームの頂点座標
    output.prevPos = mul(pos, PrevWorld);                   // 前フレームのワールド行列と乗算
    output.prevWorldPos = float4(output.prevPos.xyz, 1.0f); // 前フレームのワールド空間での頂点位置を保存
    output.prevPos = mul(output.prevPos, PrevView);         // 前フレームのビュー行列と乗算
    output.prevViewPos = output.prevPos;                    // 前フレームのビュー空間での頂点位置を保存
    output.prevPos = mul(output.prevPos, PrevProj);         // 前フレームのプロジェクション行列と乗算（クリップ空間）

	// 法線ベクトルをワールド行列で変換
    output.normal = mul(input.normal.xyz, (float3x3) World);
	
	// 頂点データのUV座標ピクセルシェーダへの出力データとして設定
    output.uv = input.uv;
    
    
    return output;
}