// ワールド変換行列
cbuffer WorldBuffer : register(b0)
{
    matrix World;
    matrix InvWorld;
    matrix PrevWorld;
}

// ビュー変換行列
cbuffer ViewBuffer : register(b1)
{
    matrix View;
    matrix InvView;
    matrix PrevView;
}

// プロジェクション変換行列
cbuffer ProjectionBuffer : register(b2)
{
    matrix Proj;
    matrix InvProj;
    matrix PrevProj;
}

// マテリアル情報
struct MATERIAL
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emission;
    float Shininess;
    float Alpha;
    float2 Dummy;
};

// マテリアル定数
cbuffer MaterialBuffer : register(b3)
{
    MATERIAL Material;
}

// ライト情報
struct LIGHT
{
    bool Enable;        // 使用するか否か
    bool3 Dummy;        // PADDING
    float3 Position;    // ライトの座標
    float Dummy2;       // PADDING
    float4 Direction;   // 方向
    float4 Diffuse;     // 拡散反射用の光の強さ
    float4 Ambient;     // 環境光用の光の強さ
};

// ライト定数
cbuffer LightBuffer : register(b4)
{
    LIGHT Light;
};

// カメラ用の定数バッファー
cbuffer CameraParam : register(b5)
{
    float3 eyePos;  // カメラの位置
    float padding;  // パディング
    
    float           nearClip;       // ニアクリップ
    float           farClip;        // ファークリップ 
    unsigned int    renderSizeX;    // レンダリングサイズ（X）
    unsigned int    renderSizeY;    // レンダリングサイズ（Y）
};

// テクスチャ用パラメータ
cbuffer TextureParam : register(b6)
{
    int UseNormalMap;  // 法線マップを使用するかどうか
    int TextureType;    // ディファードレンダリングで使用するテクスチャタイプ
    float2 padding2;    // パディング用
}

Texture2D g_Texture : register(t0);     // アルベド
Texture2D g_Normal : register(t1);      // 法線
Texture2D g_World : register(t2);       // ワールド座標
Texture2D g_Depth : register(t3);       // 深度
Texture2D g_MVector : register(t4);     // モーションベクトル

SamplerState g_SamplerState : register(s0);

struct VS_IN
{
    float4 pos : POSITION0;
    float4 normal : NORMAL0;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    float3 tangent : TANGENT0;
};

struct PS_IN
{
    float4 pos : SV_POSITION0;          // プロジェクション変換後座標
    float4 prevPos : POSITION0;         // 前フレームのプロジェクション変換後座標
    float4 worldPos : POSITION1;        // ワールド変換後座標
    float4 prevWorldPos : POSITION2;    // 前フレームのワールド変換後座標
    float4 viewPos : POSITION3;         // ビュー変換後座標
    float4 prevViewPos : POSITION4;     // 前フレームのビュー変換後座標

    
    float3 normal : NORMAL0;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    float3 tangent : TANGENT0;
    float3 biNormal : BINORMAL;
};

// Gバッファ用出力結果
struct PS_OUT
{
    float4 Albedo : SV_TARGET0;    // 第1のレンダーターゲット（RTV 0）への出力
    float4 Normal : SV_TARGET1;    // 第2のレンダーターゲット（RTV 1）への出力
    float4 World : SV_TARGET2;     // 第3のレンダーターゲット（RTV 2）への出力
    float4 Depth : SV_TARGET3;     // 第4のレンダーターゲット（RTV 3）への出力
    float4 MVector : SV_TARGET4;   // 第5のレンダーターゲット（RTV 4）への出力
};

// ディファードレンダリングの最終出力結果を
// 保存するためのレンダリング先
struct PS_OUT_2
{
    float4 OutputColor : SV_TARGET5; // 第6のレンダーターゲット（RTV 5）への出力
};



