#pragma once

#include	<d3d11.h>
#include	<DirectXMath.h>
#include	<SimpleMath.h>
#include	<io.h>
#include	<string>
#include	<vector>
#include	"NonCopyable.h"
#include	"AssimpPerse.h"
#include	"Application.h"

// 外部ライブラリ
#pragma comment(lib,"directxtk.lib")
#pragma comment(lib,"d3d11.lib")

#define GBUFFER_NUM	(5)	// Gバッファの数


// 3D頂点データ
struct VERTEX_3D
{
	DirectX::SimpleMath::Vector3	Position;
	DirectX::SimpleMath::Vector3	Normal;
	DirectX::SimpleMath::Color		Diffuse;
	DirectX::SimpleMath::Vector2	TexCoord;
	DirectX::SimpleMath::Vector3	Tangent;
};

// マテリアル
struct MATERIAL
{
	DirectX::SimpleMath::Color	Ambient;
	DirectX::SimpleMath::Color	Diffuse;
	DirectX::SimpleMath::Color	Specular;
	DirectX::SimpleMath::Color	Emission;
	float Shiness;
	float Alpha;
	float Dummy[2]{};
};

// 行列
struct MatrixInfo
{
	DirectX::SimpleMath::Matrix Mat;		// 現フレームの行列
	DirectX::SimpleMath::Matrix InvMat;		// 現フレームの逆行列
	DirectX::SimpleMath::Matrix PrevMat;	// 前フレームの行列
};



// 平行光源
struct LIGHT
{
	BOOL							Enable;
	BOOL							Dummy[3];
	DirectX::SimpleMath::Vector3	Position;
	float							Dummy2[1];
	DirectX::SimpleMath::Vector4	Direction;
	DirectX::SimpleMath::Color		Diffuse;
	DirectX::SimpleMath::Color		Ambient;

};

// カメラの位置を定数バッファに渡すための構造体
struct CAMERAPARAM
{
	DirectX::SimpleMath::Vector3 Position;	// カメラの位置
	float Padding;							// 16バイトアラインメントを確保するためのパディング

	float			nearClip;		// ニアクリップ
	float			farClip;		// ファークリップ 
	unsigned int	renderSizeX;	// レンダリングサイズ（X）
	unsigned int	renderSizeY;	// レンダリングサイズ（Y）
};

// テクスチャパラメータ
struct TEXPARAM
{
	int		UseNormalMap = 0;	// 法線マップを使用するかどうか
	int		TextureType = 0;	// Gバッファで使用するテクスチャタイプ
	float	Padding2[2];
};

// メッシュ情報（マテリアル毎にサブセットが存在する）
struct SUBSET {
	std::string		MtrlName;						// マテリアル名
	unsigned int	IndexNum = 0;					// インデックス数
	unsigned int	VertexNum = 0;					// 頂点数
	unsigned int	IndexBase = 0;					// 開始インデックス
	unsigned int	VertexBase = 0;					// 頂点ベース
	unsigned int	MaterialIdx = 0;				// マテリアルインデックス
};

// カリングモード
enum CULLMODE
{
	BACK = 0,
	FRONT,
	NONE,
};

// 深度ステンシル切り替え
enum DEPTHSTENCIL
{
	DS_DEFAULT,			// デフォルト
	DS_CASCADESHADOW,	// カスケードシャドウ
};

// レンダーターゲットを切り替える際の深度バッファの種類
enum class DEPTHTYPE
{
	DEFAULT,	// デフォルト深度バッファ（フォワードレンダリング）
	DEFERRED,	// ディファードレンダリング用深度バッファ
	DLSS,		// DLSSスーパーサンプリング用深度バッファ
	NONE,		// なし（nullptr）
};

// 出力するテクスチャタイプ
enum class TEXTURETYPE
{
	COLOR,
	NORMAL,
	WORLD,
	DEPTH,
	MVECTOR,
	OUTPUT,
};

class Camera;

// レンダラークラス
class Renderer : NonCopyable
{
private:

	// パラメータ
	static D3D_FEATURE_LEVEL		m_FeatureLevel;
	static IDXGIFactory*			m_Factory;
	static IDXGIAdapter*			m_Adapter;

	// 基本機能
	static ID3D11Device*			m_Device;
	static ID3D11DeviceContext*		m_DeviceContext;
	static IDXGISwapChain*			m_SwapChain;

	// デフォルトのレンダリングリソース
	static ID3D11RenderTargetView*	m_DefaultRTV;
	static ID3D11DepthStencilView*	m_DefaultDSV;

	// 定数バッファ
	static ID3D11Buffer*			m_WorldBuffer;
	static ID3D11Buffer*			m_ViewBuffer;
	static ID3D11Buffer*			m_ProjectionBuffer;
	static ID3D11Buffer*			m_MaterialBuffer;
	static ID3D11Buffer*			m_LightBuffer;
	static ID3D11Buffer*			m_CameraBuffer;
	static ID3D11Buffer*			m_TexParamBuffer;

	// 深度ステンシルステート
	static ID3D11DepthStencilState* m_DepthStateEnable;
	static ID3D11DepthStencilState* m_DepthStateDisable;

	// ラスタライザーステート
	static ID3D11RasterizerState* m_RsCullFront;	// カリング（表を描画しない）
	static ID3D11RasterizerState* m_RsCullBack;		// カリング（裏を描画しない）
	static ID3D11RasterizerState* m_RsCullNone;		// カリング（カリングなし）

	static bool				m_IsDisableNvidia;		// NVIDIA_GPUが使用可能か？
	static DirectX::XMUINT2	m_InputRenderSize;		// アップスケール前のレンダーターゲットサイズ（ウィンドウサイズ）
	static DirectX::XMUINT2	m_OutputRenderSize;		// アップスケール後のレンダーターゲットサイズ（3840x2160）（4K）


public:

	// 描画
	static void Init();
	static void Uninit();
	static void Begin();
	static void End();

	// 深度値の可否
	static void SetDepthEnable(bool Enable);

	// 各種リソースのセッタ
	static void SetWorldViewProjection2D();
	static void SetWorldMatrix(MatrixInfo* WorldMatrix);
	static void SetViewMatrix(MatrixInfo* ViewMatrix);
	static void SetProjectionMatrix(MatrixInfo* ProjectionMatrix);
	static void SetMaterial(MATERIAL Material);
	static void SetLight(LIGHT Light);
	static void SetCamera(Camera camera);
	static void SetTextureParam(TEXPARAM param);

	static void SetSamplerState(float LodBias = 1.0f);
	static void SetViewPort(DirectX::XMUINT2 renderSize = 
		{ Application::GetWidth(),Application::GetHeight() });

	// ゲッタ
	static ID3D11Device*			GetDevice()			{ return m_Device; }
	static ID3D11DeviceContext*		GetDeviceContext()	{ return m_DeviceContext; }
	static bool						GetIsAbleNVIDIA()	{ return m_IsDisableNvidia; }
	static IDXGIAdapter*			GetAdapter()		{ return m_Adapter; }
	static ID3D11RenderTargetView*	GetDefaultRTV()		{ return m_DefaultRTV; }

	// レンダリングサイズのセッタ・ゲッタ
	static DirectX::XMUINT2		GetInputRenderSize()	{ return m_InputRenderSize; }
	static DirectX::XMUINT2		GetOutputRenderSize()	{ return m_OutputRenderSize; }
	static void					SetInputRenderSize(DirectX::XMUINT2 inputSize)	
														{ m_InputRenderSize = inputSize; }

	// シェーダー作成
	static void					CreateVertexShader(ID3D11VertexShader** VertexShader, ID3D11InputLayout** VertexLayout, const char* FileName);
	static void					CreatePixelShader(ID3D11PixelShader** PixelShader, const char* FileName);

	// カリングモードを変更
	static void					SetCullMode(CULLMODE type);

	// デフォルトのレンダーターゲットをクリア・セット
	static void					ClearDefaultRenderTarget(bool isDSV);

};