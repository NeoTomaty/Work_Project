#pragma once
#include <DirectXMath.h>
#include <SimpleMath.h>
#include <memory>
#include "Shader.h"
#include "DirectInput.h"
#include "debugui.h"
#include "camera.h"
#include "Field.h"
#include "SceneDemo.h"
#include "StaticMesh.h"
#include "StaticMeshRenderer.h"
#include "Renderer.h"
#include "Application.h"
#include "Polygon2D.h"


#define GBUFFER_NUM	(5)	// Gバッファの数（Albedo, Normal, WorldPos, Depth, MVector）

// ON/OFF列挙
enum Enable
{
	DISABLE,
	ENABLE,
};

// レンダリングモード列挙
enum class RenderMode {
	NONE,		// なし（フォワードレンダリング）
	DEFERRED,	// ディファードレンダリング
	DLSS,		// DLSSスーパーサンプリング
};

// バッファモード列挙
enum class BufferMode {
	DEFAULT,
	COLOR,
	NORMAL,
	WORLD,
	DEPTH,
	MVECTOR,
};

// アンチエイリアシングモード列挙
enum class  AntiAliasingMode {
	NONE,
	//TEMPORAL,
	DLSS,
};


// シーンデモクラス
class SceneDemo
{
public:		// メンバ関数

	// シーン描画
	void SceneUpdate();		// 更新
	void SceneInit();		// 初期化
	void SceneDraw();		// 描画
	void SceneDispose();	// 終了

private:

	// 各種更新
	void UpdateDLSS();		// DLSSの更新処理
	void UpdateLight();		// ライトの更新処理

	// リソース生成
	void CreateRenderResource();								// フォワードレンダリング
	void CreateDeferredResource();								// ディファードレンダリング
	void CreateDLSSResource(DirectX::XMUINT2 renderSize =		// DLSS
		{ Application::GetWidth(),Application::GetHeight() });

	// リソースのセット
	void SetDeferredGBufferRenderTarget();	// GBuffer用のレンダーターゲット
	void SetDeferredShaderResource();		// ディファードレンダリング用のシェーダーリソース
	void SetDLSSRenderTarget();				// DLSS用のレンダーターゲット
	void SetDLSSShaderResource();			// DLSS用のシェーダーリソース
	void SetOffScreenRenderTarget();		// オフスクリーン先のレンダーターゲット
	void SetOffScreenResource();			// オフスクリーンのシェーダーリソース
	void SetDLSSOutputResource();			// アップスケールの出力結果

private:	// メンバ変数
	//-------------------------------------
	// シェーダー
	//-------------------------------------
	Shader m_ShaderPBR;					// PBR表現
	Shader m_ShaderSkysphere;			// skysphere用
	Shader m_ShaderDeferredRendering;	// ディファードレンダリング
	Shader m_ShaderGBuffer;				// GBuffer出力
	Shader m_ShaderDLSSInput;			// DLSSに渡す入力（オフスクリーン）
	Shader m_ShaderDLSSOutput;			// DLSSアップスケール後の出力


	//-------------------------------------
	// オブジェクト
	//-------------------------------------
	Camera		m_Camera;	// カメラ
	Field		m_Field;	// フィールド
	Polygon2D	m_Screen;	// スクリーンクアッド

	StaticMesh			m_Skysphere;	// SkySphere
	StaticMeshRenderer	m_MRSkysphere;	// レンダラー

	StaticMesh			m_Cottage;		// Cottage
	StaticMeshRenderer	m_MRCottage;	// レンダラー

	StaticMesh			m_Pole;		// Pole
	StaticMeshRenderer	m_MRPole;	// レンダラー

	//-------------------------------------
	// レンダーターゲット（RTV）
	//-------------------------------------
	ID3D11RenderTargetView*		m_DeferredGBufferRTVs[GBUFFER_NUM] = { nullptr };	// Gバッファ（ディファードレンダリング）
	ID3D11RenderTargetView*		m_DLSSGBufferRTVs[GBUFFER_NUM] = { nullptr };		// Gバッファ（DLSS）
	ID3D11RenderTargetView*		m_OffScreenRTV = nullptr;							// オフスクリーン（DLSS入力画像）

	//-------------------------------------
	// シェーダーリソース（SRV）
	//-------------------------------------
	ID3D11ShaderResourceView*	m_DeferredGBufferSRVs[GBUFFER_NUM] = { nullptr };	// Gバッファ（ディファードレンダリング）
	ID3D11ShaderResourceView*	m_DLSSGBufferSRVs[GBUFFER_NUM] = { nullptr };		// Gバッファ（DLSS）
	ID3D11ShaderResourceView*	m_OffScreenSRV = nullptr;							// オフスクリーン（DLSS入力画像）
	ID3D11ShaderResourceView*	m_DLSSOutputSRV = nullptr;							// DLSS出力値


	ID3D11ShaderResourceView*	m_DepthStencilSRV = nullptr;						// 深度情報
	ID3D11ShaderResourceView*	m_DLSSDepthSRV = nullptr;							// DLSS深度情報

	//-------------------------------------
	// 深度ステンシル（DSV）
	//-------------------------------------
	ID3D11DepthStencilView*		m_DepthStencilView = nullptr;						// 深度情報
	ID3D11DepthStencilView*		m_DLSSDepthDSV = nullptr;							// DLSS深度情報

	//-------------------------------------
	// 書き込みリソース（UAV）
	//-------------------------------------
	ID3D11UnorderedAccessView*	m_DLSSOutputUAV = nullptr;							// DLSS出力値

	//-------------------------------------
	// バッファリソース（Texture）
	//-------------------------------------
	ID3D11Texture2D*			m_DeferredGBufferTexs[GBUFFER_NUM]= { nullptr };	// Gバッファ（ディファードレンダリング）
	ID3D11Texture2D*			m_DLSSGBufferTexs[GBUFFER_NUM]	= { nullptr };		// Gバッファ（DLSS）
	ID3D11Texture2D*			m_OffScreenTex = nullptr;							// オフスクリーン（＝DLSS入力値）
	ID3D11Texture2D*			m_DLSSOutputTex = nullptr;							// DLSS出力値

	ID3D11Texture2D*			m_DepthStencilBuffer = nullptr;						// 深度情報
	ID3D11Texture2D*			m_DLSSDepthTex = nullptr;							// DLSS深度情報

};


