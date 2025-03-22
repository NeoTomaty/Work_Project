#include "SceneDemo.h"
#include "DLSSManager.h"
#include <map>

using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace std;

// クオリティリスト
std::vector<PERF_QUALITY_ITEM> PERF_QUALITY_LIST =
{
	{NVSDK_NGX_PerfQuality_Value_MaxPerf,          "Performance", false, false},	// レンダリング倍率2.0倍
	{NVSDK_NGX_PerfQuality_Value_Balanced,         "Balanced"   , false, false},	// レンダリング倍率1.7倍
	{NVSDK_NGX_PerfQuality_Value_MaxQuality,       "Quality"    , false, false},	// レンダリング倍率1.5倍
	{NVSDK_NGX_PerfQuality_Value_UltraPerformance, "UltraPerf"  , false, false},	// レンダリング倍率3.0倍
	{NVSDK_NGX_PerfQuality_Value_DLAA,             "DLAA"       , false, false},	// アンチエイリアシング
};

std::vector<RENDER_PRESET_ITEM>       RENDER_PRESET_LIST =
{
	{NVSDK_NGX_DLSS_Hint_Render_Preset_Default, "Default"},
	{NVSDK_NGX_DLSS_Hint_Render_Preset_A,       "Render Preset A"},
	{NVSDK_NGX_DLSS_Hint_Render_Preset_B,       "Render Preset B"},
	{NVSDK_NGX_DLSS_Hint_Render_Preset_C,       "Render Preset C"},
	{NVSDK_NGX_DLSS_Hint_Render_Preset_D,       "Render Preset D"},
	{NVSDK_NGX_DLSS_Hint_Render_Preset_E,       "Render Preset E"},
	{NVSDK_NGX_DLSS_Hint_Render_Preset_F,       "Render Preset F"},
};

// 動的に変化するパラメータ（ImGuiで変更）
static LIGHT	pointLight{};					// ポイントライト
static float	lightLength = 50.0f;			// 原点からライトまでの距離
static float	radY = 45;						// ライトの回転量Y
static float	radXZ = 135;					// ライトの回転量XZ
static TEXPARAM	texParam{};						// テクスチャ情報
static int renderPresetIndex = 0;				// 現在選択されているプリセットのインデックス
static int perfQualityIndex = 0;				// 現在選択されているパフォーマンス品質のインデックス
static int bufferMode = 0;						// 現在選択されているバッファモード
static bool isResetResource = false;			// 蓄積されたパラメータを破棄するか
static bool isInitDLSS = false;					// DLSS機能の初期化が出来ているか？
static bool isUseDLSSFeature = false;			// DLSSを使用するかどうか
RenderMode g_RenderMode = RenderMode::DLSS;		// レンダリングモード
AntiAliasingMode g_AntiAliasingMode				// アンチエイリアシングモード
					= AntiAliasingMode::DLSS;

// DLSSの推奨設定（画質モード別）
map<NVSDK_NGX_PerfQuality_Value, DlssRecommendedSettings> g_RecommendedSettingsMap;

// 前フレームのクォリティモード
NVSDK_NGX_PerfQuality_Value PrevQuality = 
PERF_QUALITY_LIST[perfQualityIndex].PerfQuality;
// 現フレームのクォリティモード
NVSDK_NGX_PerfQuality_Value PerfQualityMode =
PERF_QUALITY_LIST[perfQualityIndex].PerfQuality;



// DLSSデバッグ
void DebugDLSS() {

	// ディファードレンダリング前提なので
	// レンダリングモードがDEFERRED以外の場合は使用できないようにする
	if (g_RenderMode == RenderMode::DLSS)
	{
		ImGui::Begin("DLSSSetting");

		auto input = Renderer::GetInputRenderSize();
		auto output = Renderer::GetOutputRenderSize();

		// 現在のレンダリングサイズを表示
		ImGui::Text("Input Render Size: %d x %d", input.x, input.y);
		ImGui::Text("Output Render Size: %d x %d", output.x, output.y);

		// DLSSで蓄積したリソースを初期化のON/OFF
		if (ImGui::Checkbox("Use DLSS", &isUseDLSSFeature)) {
			// チェック状態が変更されたときに呼び出される
			if (isUseDLSSFeature) {
				printf("Use DLSS Enabled\n");
			}
			else {
				printf("Use DLSS Disabled\n");
			}
		}

		// DLSSで蓄積したリソースを初期化のON/OFF
		if (ImGui::Checkbox("Reset Resource", &isResetResource)) {
			// チェック状態が変更されたときに呼び出される
			if (isResetResource) {
				printf("Reset Enabled\n");
			}
			else {
				printf("Reset Disabled\n");
			}
		}

		// アンチエイリアシングモードの選択
		const char* aaModes[] = { "NONE", "DLSS" };
		int currentAAMode = static_cast<int>(g_AntiAliasingMode); // 現在のモードを整数値に変換
		if (ImGui::Combo("Anti-Aliasing Mode", &currentAAMode, aaModes, IM_ARRAYSIZE(aaModes))) {
			g_AntiAliasingMode = static_cast<AntiAliasingMode>(currentAAMode); // 選択されたモードを直接適用
			printf("Selected Anti-Aliasing Mode: %s\n", aaModes[currentAAMode]);
		}

		// パフォーマンス品質の選択
		std::vector<const char*> perfQualityItems;

		for (const auto& quality : PERF_QUALITY_LIST) {
			perfQualityItems.push_back(quality.PerfQualityText);
		}

		// アンチエイリアシングモードがDLSSの時
		if (g_AntiAliasingMode == AntiAliasingMode::DLSS)
		{
			if (ImGui::Combo("DLSS Performance Quality", &perfQualityIndex, perfQualityItems.data(), static_cast<int>(perfQualityItems.size()))) {
				// パフォーマンス品質が変更された場合に呼び出される
				printf("Selected Performance Quality: %s\n", perfQualityItems[perfQualityIndex]);
			}
		}

		ImGui::End();
	}
}
// ライティングデバッグ
void DebugLight()
{
	ImGui::Begin("LightParameter");

	// ライトの有無
	bool enable = (pointLight.Enable == TRUE); // BOOL -> bool に変換
	// チェックボックスの値を更新
	if (ImGui::Checkbox("Light Enabled", &enable))
	{
		pointLight.Enable = (enable ? TRUE : FALSE); // bool -> BOOL に変換
	}

	// ライト方向の表示
	ImGui::Text("Direction: (%.2f, %.2f, %.2f)",
		pointLight.Direction.x,
		pointLight.Direction.y,
		pointLight.Direction.z);

	// XZ回転とY回転
	float radXZDegrees = DirectX::XMConvertToDegrees(radXZ);	// XZ回転（度数法に変換）
	if (ImGui::SliderFloat("Rotation XZ (Yaw)", &radXZDegrees, 0.0f, 360.0f)) {
		radXZ = DirectX::XMConvertToRadians(radXZDegrees);		// 度数法からラジアンに戻す
	}
	float radYDegrees = DirectX::XMConvertToDegrees(radY);		// Y回転（度数法に変換）
	if (ImGui::SliderFloat("Rotation Y (Pitch)", &radYDegrees, 0.0f, 90.0f)) {
		radY = DirectX::XMConvertToRadians(radYDegrees);		// 度数法からラジアンに戻す
	}

	// ライトの位置
	ImGui::Text("Light Position: (%.2f, %.2f, %.2f)", pointLight.Position.x, pointLight.Position.y, pointLight.Position.z);

	// 距離 (length) の調整
	ImGui::SliderFloat("Light Distance (Length)", &lightLength, 10.0f, 100.0f);


	// ディフューズ
	static float diffuse[3] = { pointLight.Diffuse.x, pointLight.Diffuse.y, pointLight.Diffuse.z };
	if (ImGui::ColorEdit3("Diffuse", diffuse))
	{
		pointLight.Diffuse = Color(diffuse[0], diffuse[1], diffuse[2], 1.0f);
	}

	// アンビエント
	static float ambient[3] = { pointLight.Ambient.x, pointLight.Ambient.y, pointLight.Ambient.z };
	if (ImGui::ColorEdit3("Ambient", ambient))
	{
		pointLight.Ambient = Color(ambient[0], ambient[1], ambient[2], 1.0f);
	}

	ImGui::End();
}
// レンダリングデバッグ
void DebugRender()
{
	ImGui::Begin("RenderingSetting");

	// カリングモード
	static int cullmode = CULLMODE::BACK; // 現在選択されているカリング

	// 表示する選択肢のリスト
	const char* items[] = { "BACK", "FRONT", "NONE" };

	// Comboボックスの描画
	if (ImGui::Combo("Select CullMode", &cullmode, items, IM_ARRAYSIZE(items))) {
		// 選択された項目が変更された場合に呼び出される
		printf("Selected option: %s\n", items[cullmode]);
		Renderer::SetCullMode(static_cast<CULLMODE>(cullmode));
	}

	// レンダリングモード選択用の静的変数
	static int renderMode = static_cast<int>(g_RenderMode);

	// レンダリングモードの選択肢（文字列配列）
	const char* renderItems[] = { "NONE", "DEFERRED", "DEFERRED+DLSS"};

	// Comboボックスでレンダリングモードを選択
	if (ImGui::Combo("Select RenderMode", &renderMode, renderItems, IM_ARRAYSIZE(renderItems))) {
		// ユーザーが選択を変更した場合に呼び出される
		printf("Selected RenderMode: %s\n", renderItems[renderMode]);

		// グローバル変数を更新
		g_RenderMode = static_cast<RenderMode>(renderMode);
	}

	// DEFERREDモードの場合に追加の選択項目を表示
	if (g_RenderMode == RenderMode::DEFERRED) {

		// バッファ選択肢（文字列配列）
		const char* bufferItems[] = { "DEFAULT", "COLOR", "NORMAL", "WORLD", "DEPTH", "MVECTOR"};

		// Comboボックスでバッファを選択
		if (ImGui::Combo("Select Buffer", &bufferMode, bufferItems, IM_ARRAYSIZE(bufferItems))) {
			// 選択されたバッファに応じて処理
			printf("Selected Buffer: %s\n", bufferItems[bufferMode]);
		}
	}

	ImGui::End();
}



// シーンの初期化
void SceneDemo::SceneInit()
{
	// カメラ初期化
	m_Camera.Init();

	// スクリーン初期化
	m_Screen.Init();

	// フィールド初期化
	m_Field.Init();

	// ライトの初期化
	{
		pointLight.Enable = true;
		pointLight.Position = Vector3(100.0f, 100.0f, 100.0f);	// DirectionalLightの場合は未使用
		pointLight.Direction = Vector4(0.5f, -1.0f, 0.8f, 0.0f);
		pointLight.Direction.Normalize();
		pointLight.Ambient = Color(0.5f, 0.5f, 0.5f, 1.0f);
		pointLight.Diffuse = Color(1.5f, 1.5f, 1.5f, 1.0f);
		Renderer::SetLight(pointLight);
	}

	// レンダリングリソースの生成
	CreateRenderResource();

	// Imgui用デバッグ関数呼び出し
	DebugUI::RedistDebugFunction(DebugLight);
	DebugUI::RedistDebugFunction(DebugDLSS);
	DebugUI::RedistDebugFunction(DebugRender);

	// モデルファイルパス
	std::vector<std::string> filename =
	{
		"assets/model/Cottage/cottage.fbx",
		"assets/model/SkySphere.fbx",
		"assets/model/Cube.fbx",
	};
	// テクスチャファイルパス
	std::vector<std::string> texfilename =
	{
		"assets/model/Cottage/cottage_diffuse.png",
		"assets/model/Cottage/cottage_normal.png",
		"assets/texture/bg.jpg",
	};
	// シェーダーファイルパス
	std::vector<std::string> shaderfile =
	{
		"shader/VS/VS_PBR.hlsl",				// 0
		"shader/PS/PS_PBR.hlsl",				// 1

		"shader/VS/VS_Default.hlsl",			// 2
		"shader/PS/PS_Default.hlsl",			// 3

		"shader/PS/PS_GBuffer.hlsl",			// 4
		"shader/PS/PS_DeferredRendering.hlsl",	// 5
		"shader/PS/PS_DLSSOffScreenColor.hlsl",	// 6	
		"shader/PS/PS_DLSSOutput.hlsl",			// 7
	};

	// シェーダー
	{
		// フォワードレンダリングモード
		m_ShaderPBR.Create(shaderfile[2], shaderfile[1]);				// PBR
		// ディファードレンダリングモード
		m_ShaderGBuffer.Create(shaderfile[2], shaderfile[4]);			// Gバッファ出力
		m_ShaderDeferredRendering.Create(shaderfile[2], shaderfile[5]);	// ディファードレンダリング
		// DLSSモード
		m_ShaderDLSSInput.Create(shaderfile[2], shaderfile[6]);			// DLSSの入力値（オフスクリーン）
		m_ShaderDLSSOutput.Create(shaderfile[2], shaderfile[7]);		// DLSSの出力値
		// その他・モデル個別用
		m_ShaderSkysphere.Create(shaderfile[2], shaderfile[3]);			// skybox

	}

	// SkyBox読み込み
	{
		std::string f = filename[1];

		// モデル読み込み
		m_Skysphere.Load(f);

		// テクスチャ読み込み
		f = texfilename[2];
		m_Skysphere.LoadTex(f, DIFFUSE);

		// レンダラーにモデルを渡す
		m_MRSkysphere.Init(m_Skysphere);

		Vector3 pos = Vector3(0.0f, 0.0f, 0.0f);
		Vector3 rotate = Vector3(0.0f, 0.0f, 0.0f);
		Vector3 scale = Vector3(1.0f, 1.0f, 1.0f);
		m_MRSkysphere.SetPosition(pos);
		m_MRSkysphere.SetRotation(rotate);
		m_MRSkysphere.SetScale(scale);
	}

	// Cottage読み込み
	{
		std::string f = filename[0];

		// モデル読み込み
		m_Cottage.Load(f);

		// テクスチャ読み込み
		f = texfilename[0];
		m_Cottage.LoadTex(f, TexType::DIFFUSE);
		f = texfilename[1];
		m_Cottage.LoadTex(f, TexType::NORMAL);

		// レンダラーにモデルを渡す
		m_MRCottage.Init(m_Cottage, true);

		Vector3 pos = Vector3(0.0f, 0.0f, 0.0f);
		Vector3 rotate = Vector3(0.0f, 45.0f, 0.0f);
		Vector3 scale = Vector3(1.0f, 1.0f, 1.0f);
		m_MRCottage.SetPosition(pos);
		m_MRCottage.SetRotation(rotate);
		m_MRCottage.SetScale(scale);
	}


}
// シーンの更新
void SceneDemo::SceneUpdate()
{
	// 各種オブジェクト更新処理
	m_Camera.Update();
	m_MRCottage.Update();
	m_Screen.Update();

	// ライトの更新処理
	UpdateLight();

	// DLSSモードで使用していたら
	if (g_RenderMode == RenderMode::DLSS &&
		g_AntiAliasingMode == AntiAliasingMode::DLSS)
		UpdateDLSS();	// DLSSアップスケーリングの更新処理

}
// シーンの描画
void SceneDemo::SceneDraw()
{
	// Applicationに格納されている描画カウントをリセット
	Application::ResetDrawInfo();


	// レンダリングモードに応じて描画を変える
	switch (g_RenderMode)
	{
	case RenderMode::NONE:
		// 通常のレンダリング（フォワードレンダリング）
	{
		// バッファリソースのセット
		Renderer::SetSamplerState();
		Renderer::ClearDefaultRenderTarget(true);	// デフォルトレンダーターゲットを指定
		Renderer::SetViewPort();					// ビューポートを指定
		Renderer::SetDepthEnable(true);				// 深度ステンシルステートをON
		Renderer::SetCamera(m_Camera);				// 3D（透視投影）カメラセット
		m_Camera.Draw();							// バインド

		// オブジェクトの描画
		m_ShaderSkysphere.SetGPU();					// シェーダーをセット（skysphere）
		m_Skysphere.SetTexture();					// テクスチャをセット
		m_MRSkysphere.Draw();						// skysphereの描画

		m_ShaderPBR.SetGPU();						// シェーダーをセット（PBR）
		m_Field.Draw();								// フィールドの描画
		m_Cottage.SetTexture();						// テクスチャセット
		m_MRCottage.Draw();							// モデルの描画

	}
	break;
	case RenderMode::DEFERRED:
		// ディファードレンダリング
	{

		// バッファリソースのセット
		Renderer::SetSamplerState();
		texParam.TextureType = bufferMode;			// 表示するテクスチャタイプを設定
		Renderer::SetViewPort();					// ビューポートを指定
		Renderer::SetDepthEnable(true);				// 深度ステンシルステートをON
		Renderer::SetCamera(m_Camera);				// 3D（透視投影）カメラセット
		m_Camera.Draw();							// バインド

		auto context = Renderer::GetDeviceContext();
		auto defaultRTV = Renderer::GetDefaultRTV();

		//---------------------------------------
		// ディファードレンダリング
		//---------------------------------------
		SetDeferredGBufferRenderTarget();			// ディファードレンダリング用レンダーターゲットを指定

		// オブジェクトの描画
		m_ShaderGBuffer.SetGPU();					// シェーダーをセット（GBuffer）

		m_Skysphere.SetTexture();					// テクスチャをセット
		m_MRSkysphere.Draw();						// skysphereの描画

		texParam.UseNormalMap = Enable::ENABLE;		// 法線マップ使用
		Renderer::SetTextureParam(texParam);		// バインド
		m_Field.Draw();								// フィールドの描画
		m_Cottage.SetTexture();						// テクスチャをセット
		m_MRCottage.Draw();							// モデルの描画


		//---------------------------------------
		// ディファードシェーディング
		//---------------------------------------
		
		Renderer::ClearDefaultRenderTarget(false);	// デフォルトのレンダーターゲットをセット（深度バッファなし）
		SetDeferredShaderResource();				// GBufferの情報をテクスチャとしてバインド
		m_ShaderDeferredRendering.SetGPU();			// シェーダーのセット（ディファードレンダリング）
		m_Screen.Draw();							// フルスクリーンクアッドの描画


		//========================================================
		// 問題点１
		// ディファードレンダリングとフォワードレンダリングの
		// 両方を用いた描画だと上手く描画されない
		// →深度バッファに問題がある？
		//========================================================


		//↓半透明オブジェクトや異なるライティングを行うモデルのため↓
		//---------------------------------------
		// フォワードレンダリング
		//---------------------------------------
		
		// フォワードレンダリングは深度バッファを有効化
		//context->OMSetRenderTargets(1, &defaultRTV, m_DepthStencilView);

		// 深度バッファはそのまま使用する (必要ならクリア)
		// context->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

		//// オブジェクトの描画
		//m_ShaderSkysphere.SetGPU();				// シェーダーをセット（skysphere）
		//m_Skysphere.SetTexture();					// テクスチャをセット
		//m_MRSkysphere.Draw();						// skysphereの描画



	}
		break;
	case RenderMode::DLSS:
		// DLSSアップスケール
		{

		Renderer::SetViewPort(Renderer::GetInputRenderSize());	// ビューポートを指定
		Renderer::SetDepthEnable(true);							// 深度ステンシルステートをON
		Renderer::SetCamera(m_Camera);							// 3D（透視投影）カメラセット
		m_Camera.Draw();										// バインド

		//----------------------------------------------
		// ディファードレンダリング
		//----------------------------------------------
		SetDLSSRenderTarget();						// DLSS用レンダーターゲットを指定

		// GBuffer用シェーダーをセット
		m_ShaderGBuffer.SetGPU();

		// オブジェクトの描画
		texParam.UseNormalMap = Enable::DISABLE;	// 法線マップ未使用
		Renderer::SetTextureParam(texParam);		// バインド
		m_Skysphere.SetTexture();					// テクスチャをセット
		m_MRSkysphere.Draw();						// skysphereの描画

		texParam.UseNormalMap = Enable::ENABLE;		// 法線マップ使用
		Renderer::SetTextureParam(texParam);		// バインド
		m_Field.Draw();								// フィールドの描画
		m_Cottage.SetTexture();						// テクスチャをセット
		m_MRCottage.Draw();							// モデルの描画


		//-----------------------------------------------------------------------------
		// オフスクリーンレンダリング（DLSSに渡す入力値を取得）
		// ※オフスクリーンのレンダーターゲットはSetDLSSRenderTarget()で指定しておく
		//-----------------------------------------------------------------------------
		SetOffScreenRenderTarget();		// オフスクリーン用のレンダーターゲットをセット

		SetDLSSShaderResource();		// Gバッファをバインド
		m_ShaderDLSSInput.SetGPU();		// オフスクリーンレンダリング用シェーダーをセット
		m_Screen.Draw();				// オフスクリーンレンダリング


		//------------------------------------------------
		// レンダリング結果をアップスケーリング（DLSS）
		//------------------------------------------------

		// Gバッファから得られたリソースを取得
		ID3D11Texture2D* PreResolveColor = m_OffScreenTex;
		ID3D11Texture2D* ResolvedColor = m_DLSSOutputTex;
		ID3D11Texture2D* MotionVectors = m_DLSSGBufferTexs[4];
		ID3D11Texture2D* Depth = m_DLSSDepthTex;
		ID3D11Texture2D* Exposure = nullptr;

		// アップスケールの成功フラグ
		bool isEvaluate = false;

		// DLSSスーパーサンプリング
		// ココでDLSSサンプリングを行う
		if (g_AntiAliasingMode == AntiAliasingMode::DLSS)
		{
			// DLSS機能が初期化されていて
			// DLSSを使用するフラグが経っている場合
			if (isInitDLSS && isUseDLSSFeature)
			{
				bool ResetScene = isResetResource;


				//==============================================================================
				// 問題点２
				// DLAA以外のアップスケールの処理が行われない
				// →DLAAが成功していることから渡しているリソースに以上はないことがわかる
				// →レンダリングサイズが原因？あるいは足りないリソースがある？
				//==============================================================================

				// DLSSスーパーサンプリング
				if (DLSSManager::GetInstance().EvaluateSuperSampling(
					PreResolveColor,		// 入力
					ResolvedColor,			// 出力
					MotionVectors,			// モーションベクトル
					Depth,					// 深度
					Exposure,				// 露出
					ResetScene,				// リセットフラグ
					true,					// 拡張APIフラグ
					{ 0.0f, 0.0f },			// ジッターオフセット
					{ 1.0f, 1.0f }))		// モーションベクトルスケール
				{
					std::cout << "アップスケールに成功しました" << std::endl;
					isEvaluate = true;
				}
				else
				{
					// スーパーサンプリングに失敗した場合は
					// 入力前のリソースを使用する
					std::cerr << "アップスケールに失敗しました" << std::endl;
					std::cout << "アップスケール前の画像を使用します" << std::endl;
					isEvaluate = false;
				}
			}
			else
			{
				// DLSS機能の初期化がされていなかったら
				if (!isInitDLSS)
				{
					std::cerr << "DLSS機能が初期化されていません" << std::endl;
					std::cerr << "アップスケール前の画像を使用します" << std::endl;
					isEvaluate = false;
				}
			}
		}

		//---------------------------------
		// アップスケール結果を描画
		//---------------------------------
		Renderer::ClearDefaultRenderTarget(true);	// レンダーターゲットを元に戻す
		Renderer::SetViewPort();					// ビューポートをウィンドウサイズに戻す
	
		// スーパーサンプリングの可否に応じてバインドするリソースを変更
		if (isEvaluate) {
			SetDLSSOutputResource();				// アウトプットリソース
		}
		else {
			SetOffScreenResource();					// オフスクリーンリソース
		}

		m_ShaderDLSSOutput.SetGPU();				// 出力用シェーダーをセット
		m_Screen.Draw();							// 結果をフルスクリーンクアッドに描画

		}
		break;
	}

}
// シーンの終了
void SceneDemo::SceneDispose()
{
	// 解放 
	
	// G-Buffer
	for (int i = 0; i < GBUFFER_NUM; ++i) {
		if (m_DeferredGBufferTexs[i]) {
			m_DeferredGBufferTexs[i]->Release();
			m_DeferredGBufferTexs[i] = nullptr;
		}
		if (m_DeferredGBufferRTVs[i]) {
			m_DeferredGBufferRTVs[i]->Release();
			m_DeferredGBufferRTVs[i] = nullptr;
		}
		if (m_DeferredGBufferSRVs[i]) {
			m_DeferredGBufferSRVs[i]->Release();
			m_DeferredGBufferSRVs[i] = nullptr;
		}

		if (m_DLSSGBufferTexs[i]) {
			m_DLSSGBufferTexs[i]->Release();
			m_DLSSGBufferTexs[i] = nullptr;
		}
		if (m_DLSSGBufferRTVs[i]) {
			m_DLSSGBufferRTVs[i]->Release();
			m_DLSSGBufferRTVs[i] = nullptr;
		}
		if (m_DLSSGBufferSRVs[i]) {
			m_DLSSGBufferSRVs[i]->Release();
			m_DLSSGBufferSRVs[i] = nullptr;
		}
	}

	// DLSS深度テクスチャ
	m_DLSSDepthTex->Release();
	m_DLSSDepthTex = nullptr;
	m_DLSSDepthDSV->Release();
	m_DLSSDepthDSV = nullptr;
	m_DLSSDepthSRV->Release();
	m_DLSSDepthSRV = nullptr;

	// DLSS出力テクスチャ
	m_DLSSOutputTex->Release();
	m_DLSSOutputTex = nullptr;
	m_DLSSOutputUAV->Release();
	m_DLSSOutputUAV = nullptr;
	m_DLSSOutputSRV->Release();
	m_DLSSOutputSRV = nullptr;

	// DLSS入力テクスチャ
	m_OffScreenTex->Release();
	m_OffScreenTex = nullptr;
	m_OffScreenRTV->Release();
	m_OffScreenRTV = nullptr;
	m_OffScreenSRV->Release();
	m_OffScreenSRV = nullptr;

	// ディファードレンダリング
	m_DepthStencilSRV->Release();
	m_DepthStencilSRV = nullptr;
	m_DepthStencilView->Release();
	m_DepthStencilView = nullptr;
	m_DepthStencilBuffer->Release();
	m_DepthStencilBuffer = nullptr;

}



// DLSSの更新処理
void SceneDemo::UpdateDLSS()
{
	//-------------------------------
	// レンダリングサイズ関連
	//-------------------------------

	// レンダリングサイズ（アップスケール前）
	XMUINT2 inputRenderTargetSize = Renderer::GetInputRenderSize();
	// 最終出力レンダリングサイズ（アップスケール後）
	XMUINT2 outputRenderTargetSize = Renderer::GetOutputRenderSize();
	// 直前（前フレーム）の推奨レンダリングサイズ
	static XMUINT2 inputLastSize = { 0, 0 };	// 入力
	static XMUINT2 outputLastSize = { 0, 0 };	// 出力 
	// サンプラーステートに渡すLodバイアス
	float lodBias = 0.f;
	// DLSS作成時に設定したレンダリングサイズ
	XMUINT2 dlssCreationTimeRenderSize = inputRenderTargetSize;

	//-------------------------------
	// クオリティモードの設定
	//-------------------------------

	// 現在のクオリティモード（UIから設定可能にしている）
	PrevQuality = PerfQualityMode;
	PerfQualityMode = PERF_QUALITY_LIST[perfQualityIndex].PerfQuality;

	//------------------------------------------------
	// DLSSの推奨サイズ設定（レンダリングサイズ）のクエリ
	// 最終レンダリングサイズ（output）から
	// 推奨レンダリングサイズ（input）を求める
	//------------------------------------------------

	// アンチエイリアシングモードがDLSSの場合
	if (g_AntiAliasingMode == AntiAliasingMode::DLSS)
	{
		// 最終出力レンダリングサイズが前フレームから更新されていない場合は
		// 最適レンダリングサイズの取得クエリは省略出来る
		if (outputLastSize.x != outputRenderTargetSize.x ||
			outputLastSize.y != outputRenderTargetSize.y)
		{
			// 各パフォーマンスモードについて最適設定のクエリ
			for (PERF_QUALITY_ITEM& item : PERF_QUALITY_LIST)
			{
				// 推奨レンダリングサイズのクエリ
				DLSSManager::GetInstance().QueryOptimalSettings(
					outputRenderTargetSize,
					item.PerfQuality,
					&g_RecommendedSettingsMap[item.PerfQuality]);

				// 推奨設定が有効か確認
				bool isRecommendedSettingValid = g_RecommendedSettingsMap[item.PerfQuality].m_ngxRecommendedOptimalRenderSize.x > 0;
				item.PerfQualityAllowed = isRecommendedSettingValid;

				if (isRecommendedSettingValid)
				{
					std::cout << "true isvalid" << endl;
				}
				else
				{
					std::cout << "false isvalid" << endl;
				}

				// 推奨サイズのデバッグ表示
				std::cout << "Recommended Optimal Render Size: ("
					<< g_RecommendedSettingsMap[item.PerfQuality].m_ngxRecommendedOptimalRenderSize.x << ", "
					<< g_RecommendedSettingsMap[item.PerfQuality].m_ngxRecommendedOptimalRenderSize.y << ")" << std::endl;
				std::cout << "Dynamic Maximum Render Size: ("
					<< g_RecommendedSettingsMap[item.PerfQuality].m_ngxDynamicMaximumRenderSize.x << ", "
					<< g_RecommendedSettingsMap[item.PerfQuality].m_ngxDynamicMaximumRenderSize.y << ")" << std::endl;
				std::cout << "Dynamic Minimum Render Size: ("
					<< g_RecommendedSettingsMap[item.PerfQuality].m_ngxDynamicMinimumRenderSize.x << ", "
					<< g_RecommendedSettingsMap[item.PerfQuality].m_ngxDynamicMinimumRenderSize.y << ")" << std::endl;

				// 動的設定が可能か確認
				bool isDynamicSettingAllowed =
					(g_RecommendedSettingsMap[item.PerfQuality].m_ngxDynamicMaximumRenderSize.x != g_RecommendedSettingsMap[item.PerfQuality].m_ngxDynamicMinimumRenderSize.x) ||
					(g_RecommendedSettingsMap[item.PerfQuality].m_ngxDynamicMaximumRenderSize.y != g_RecommendedSettingsMap[item.PerfQuality].m_ngxDynamicMinimumRenderSize.y);
				item.PerfQualityDynamicAllowed = isDynamicSettingAllowed;

				if (isDynamicSettingAllowed)
				{
					std::cout << "true dynamic allowed" << endl;
				}
				else
				{
					std::cout << "false dynamic allowd" << endl;
				}
			}

			// 前フレームのレンダリングサイズに
			// 現在のDLSS最終レンダリングサイズを保存しておく
			outputLastSize = outputRenderTargetSize;

			std::cout << "DLSS最適設定完了" << endl;
		}
	}


	//--------------------------------------------------------------------
	// レンダリングサイズが前フレームと比較して変更されていた場合の処理
	// （主にパフォーマンスモードに依存してサイズが変わる）
	//--------------------------------------------------------------------

	// テクスチャのLOD（Level of Detail）のX軸方向のサイズを初期化
	float texLodXDimension = { 0 };

	// 推奨されている最適なレンダリングサイズをDLSS初期化時のレンダリングサイズに設定
	dlssCreationTimeRenderSize = g_RecommendedSettingsMap[PerfQualityMode].m_ngxRecommendedOptimalRenderSize;

	// DLSS作成時のレンダリングサイズをアップスケール前のレンダリングサイズに設定
	inputRenderTargetSize = dlssCreationTimeRenderSize;

	// Lodバイアスをレンダリングサイズから計算
	texLodXDimension = inputRenderTargetSize.x;
	float ratio = (float)inputRenderTargetSize.x / (float)outputRenderTargetSize.x;
	lodBias = (std::log2f(ratio)) - 1.0f;

	// 求めたLodバイアスでサンプラーをセット
	Renderer::SetSamplerState(lodBias);

	//-------------------------------
	// DLSSの初期化
	//-------------------------------
	// レンダリングサイズが前フレームと異なる場合
	if (inputRenderTargetSize.x != inputLastSize.x ||
		inputRenderTargetSize.y != inputLastSize.y)
	{

		std::cout << "Lod Bias: " << lodBias << std::endl;

		// HDRや深度反転の設定（必要に応じて変更）
		bool isHDR = false;
		bool depthInverted = false;

		// DLSSオプション（例: Quality設定）
		NVSDK_NGX_PerfQuality_Value qualitySetting = PerfQualityMode;

		// レンダープリセット（0はデフォルト）
		unsigned int renderPreset = 0;

		// レンダリングサイズに応じてリソースを再生成
		Renderer::SetInputRenderSize(inputRenderTargetSize);
		CreateDLSSResource(inputRenderTargetSize);


		// NVIDIA GPUが使用可能な場合
		if (Renderer::GetIsAbleNVIDIA())
		{
			isInitDLSS = true;

			// DLSS初期化
			if (!DLSSManager::GetInstance().InitializeDLSSFeatures(
				inputRenderTargetSize,
				outputRenderTargetSize,
				isHDR,
				depthInverted,
				true, // enableSharpening
				true, // enableAutoExposure
				qualitySetting,
				renderPreset))
			{
				std::cerr << "DLSSの初期化に失敗しました" << std::endl;
				isInitDLSS = false;
			}

		}
		else
		{
			std::cerr << "NVIDIA_GPUを使用していません" << std::endl;
		}

	}

	// 前フレームに現在のレンダリングサイズを保存
	inputLastSize = inputRenderTargetSize;
}
// ライトの更新処理
void SceneDemo::UpdateLight()
{
	// ライト方向の計算（原点を中心に回転）
	pointLight.Direction.x = cosf(radY) * sinf(radXZ);
	pointLight.Direction.y = -sinf(radY);
	pointLight.Direction.z = cosf(radY) * cosf(radXZ);
	pointLight.Direction.Normalize();

	// ライト方向の計算（回転に応じて位置を更新）
	pointLight.Position.x = lightLength * cosf(radY) * sinf(radXZ);
	pointLight.Position.y = lightLength * sinf(radY);
	pointLight.Position.z = lightLength * cosf(radY) * cosf(radXZ);

	// ライトを反映
	Renderer::SetLight(pointLight);
}



// レンダリングリソースの生成
void SceneDemo::CreateRenderResource()
{
	CreateDeferredResource();	// ディファードレンダリング
	CreateDLSSResource();		// DLSS
}



// ディファードレンダリングリソースの生成
void SceneDemo::CreateDeferredResource()
{
	auto device = Renderer::GetDevice();
	auto iRenderSize = Renderer::GetInputRenderSize();

	// 各Gバッファのフォーマット
	DXGI_FORMAT formats[GBUFFER_NUM] = {
		DXGI_FORMAT_R8G8B8A8_UNORM,		// Albedo
		DXGI_FORMAT_R16G16B16A16_FLOAT, // Normal
		DXGI_FORMAT_R32G32B32A32_FLOAT, // WorldPos
		DXGI_FORMAT_R32G32B32A32_FLOAT,	// Depth（ダミー）
		DXGI_FORMAT_R32G32_FLOAT,		// MVector
	};

	// Gバッファの生成
	for (int i = 0; i < GBUFFER_NUM; ++i) {
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = iRenderSize.x;
		textureDesc.Height = iRenderSize.y;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.SampleDesc.Count = 1;	// マルチサンプルは無し
		textureDesc.Format = formats[i];
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		// テクスチャ作成
		auto hr = device->CreateTexture2D(&textureDesc, nullptr, &m_DeferredGBufferTexs[i]);
		if (FAILED(hr)) { cout << "Gバッファテクスチャの作成に失敗しました" << endl; }
		// RTV作成
		hr = device->CreateRenderTargetView(m_DeferredGBufferTexs[i], nullptr, &m_DeferredGBufferRTVs[i]);
		if (FAILED(hr)) { cout << "GバッファRTVの作成に失敗しました" << endl; }
		// SRV作成
		hr = device->CreateShaderResourceView(m_DeferredGBufferTexs[i], nullptr, &m_DeferredGBufferSRVs[i]);
		if (FAILED(hr)) { cout << "GバッファSRVの作成に失敗しました" << endl; }
	}

	// 深度バッファ作成
	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = iRenderSize.x;
	depthDesc.Height = iRenderSize.y;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_R24G8_TYPELESS; // 深度 + ステンシル
	depthDesc.SampleDesc.Count = 1;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	// テクスチャ作成
	auto hr = device->CreateTexture2D(&depthDesc, nullptr, &m_DepthStencilBuffer);
	if (FAILED(hr)) { cout << "深度ステンシルテクスチャの作成に失敗しました" << endl; }
	// DSV 作成
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	hr = device->CreateDepthStencilView(m_DepthStencilBuffer, &dsvDesc, &m_DepthStencilView);
	if (FAILED(hr)) { cout << "深度ステンシルビューの作成に失敗しました" << endl; }
	// SRV 作成
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; // 深度値のみをSRVとして使用
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(m_DepthStencilBuffer, &srvDesc, &m_DepthStencilSRV);
	if (FAILED(hr)) { cout << "深度ステンシルSRVの作成に失敗しました" << endl; }

}
// DLSSリソースの生成（及び再生成）
void SceneDemo::CreateDLSSResource(XMUINT2 renderSize)
{
	auto device = Renderer::GetDevice();

	// 既存リソースの解放
	// Gバッファの解放
	for (int i = 0; i < GBUFFER_NUM; ++i) {
		if (m_DLSSGBufferRTVs[i]) m_DLSSGBufferRTVs[i]->Release();
		if (m_DLSSGBufferSRVs[i]) m_DLSSGBufferSRVs[i]->Release();
		if (m_DLSSGBufferTexs[i]) m_DLSSGBufferTexs[i]->Release();
	}
	// DLSS入力バッファの解放
	if (m_OffScreenRTV) m_OffScreenRTV->Release();
	if (m_OffScreenSRV) m_OffScreenSRV->Release();
	if (m_OffScreenTex) m_OffScreenTex->Release();
	// DLSS出力バッファの解放
	if (m_DLSSOutputUAV) m_DLSSOutputUAV->Release();
	if (m_DLSSOutputSRV) m_DLSSOutputSRV->Release();
	if (m_DLSSOutputTex) m_DLSSOutputTex->Release();
	// 深度バッファの解放
	if (m_DLSSDepthDSV) m_DLSSDepthDSV->Release();
	if (m_DLSSDepthSRV) m_DLSSDepthSRV->Release();
	if (m_DLSSDepthTex) m_DLSSDepthTex->Release();


	D3D11_TEXTURE2D_DESC textureDesc = {};
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

	// 各Gバッファのフォーマット
	DXGI_FORMAT formats[GBUFFER_NUM] = {
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,// Albedo（非線形空間）
		DXGI_FORMAT_R16G16B16A16_FLOAT, // Normal
		DXGI_FORMAT_R32G32B32A32_FLOAT, // WorldPos
		DXGI_FORMAT_R32_FLOAT,			// Depth
		DXGI_FORMAT_R32G32_FLOAT,		// MVector
	};

	//------------------------------------------------
	// Gバッファの生成
	//------------------------------------------------
	for (int i = 0; i < GBUFFER_NUM; ++i) {

		textureDesc.Width = renderSize.x;
		textureDesc.Height = renderSize.y;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.SampleDesc.Count = 1;	// マルチサンプルは無し
		textureDesc.Format = formats[i];
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		// テクスチャ
		device->CreateTexture2D(&textureDesc, nullptr, &m_DLSSGBufferTexs[i]);
		// RTV
		device->CreateRenderTargetView(m_DLSSGBufferTexs[i], nullptr, &m_DLSSGBufferRTVs[i]);
		// SRV
		device->CreateShaderResourceView(m_DLSSGBufferTexs[i], nullptr, &m_DLSSGBufferSRVs[i]);

	}

	//------------------------------------------------
	// 深度バッファの生成
	//------------------------------------------------
	textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	textureDesc.Width = renderSize.x;
	textureDesc.Height = renderSize.y;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE; // SRVをバインド可能に
	// テクスチャ
	device->CreateTexture2D(&textureDesc, nullptr, &m_DLSSDepthTex);
	// DSV
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView(m_DLSSDepthTex, &dsvDesc, &m_DLSSDepthDSV);
	// SRV
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; // シェーダーで深度のみを使用
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(m_DLSSDepthTex, &srvDesc, &m_DLSSDepthSRV);

	//------------------------------------------------
	// 入力先バッファの生成
	//------------------------------------------------
	textureDesc.Width = renderSize.x;
	textureDesc.Height = renderSize.y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;	// マルチサンプルは無し
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	// テクスチャ
	device->CreateTexture2D(&textureDesc, nullptr, &m_OffScreenTex);
	// RTV
	device->CreateRenderTargetView(m_OffScreenTex, nullptr, &m_OffScreenRTV);
	// SRV
	device->CreateShaderResourceView(m_OffScreenTex, nullptr, &m_OffScreenSRV);


	//------------------------------------------------
	// 出力先バッファの生成
	//------------------------------------------------
	textureDesc.Width = renderSize.x;
	textureDesc.Height = renderSize.y;
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT,	 // 出力フォーマット
	textureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	// テクスチャ
	device->CreateTexture2D(&textureDesc, nullptr, &m_DLSSOutputTex);
	// UAV
	uavDesc.Format = textureDesc.Format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	device->CreateUnorderedAccessView(m_DLSSOutputTex, &uavDesc, &m_DLSSOutputUAV);
	// SRV
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(m_DLSSOutputTex, &srvDesc, &m_DLSSOutputSRV);

}



// レンダーターゲットをセット（ディファードレンダリング）
void SceneDemo::SetDeferredGBufferRenderTarget()
{
	auto context = Renderer::GetDeviceContext();

	// Gバッファをレンダーターゲットとして設定
	ID3D11RenderTargetView* rtv[] = {
		m_DeferredGBufferRTVs[0], // Albedo
		m_DeferredGBufferRTVs[1], // Normal
		m_DeferredGBufferRTVs[2], // WorldPos
		m_DeferredGBufferRTVs[3], // Depth
		m_DeferredGBufferRTVs[4], // MVector
	};
	
	context->OMSetRenderTargets(GBUFFER_NUM, rtv, m_DepthStencilView);

	// Gバッファをクリア
	float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	for (int i = 0; i < GBUFFER_NUM; ++i) {
		context->ClearRenderTargetView(m_DeferredGBufferRTVs[i], clearColor);
	}
	context->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

}
// ディファードレンダリングリソースをセット
void SceneDemo::SetDeferredShaderResource()
{
	auto context = Renderer::GetDeviceContext();

	// SRVをピクセルシェーダーにバインド
	context->PSSetShaderResources(0, GBUFFER_NUM, m_DeferredGBufferSRVs);
	// DSVで書き込んだ深度値をバインド
	context->PSSetShaderResources(3, 1, &m_DepthStencilSRV);
}



// DLSS用のレンダーターゲットを指定
void SceneDemo::SetDLSSRenderTarget()
{
	auto context = Renderer::GetDeviceContext();

	// Gバッファをレンダーターゲットとして設定
	ID3D11RenderTargetView* rtv[] = {
		m_DLSSGBufferRTVs[0],	// Albedo
		m_DLSSGBufferRTVs[1],	// Normal
		m_DLSSGBufferRTVs[2],	// WorldPos
		m_DLSSGBufferRTVs[3],	// Depth（ダミー）
		m_DLSSGBufferRTVs[4],	// MVector
		m_OffScreenRTV,			// オフスクリーン
	};;

	// ビューの数
	unsigned int viewNum = GBUFFER_NUM + 1;

	context->OMSetRenderTargets(viewNum, rtv, m_DLSSDepthDSV);

	// Gバッファをクリア
	float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	for (int i = 0; i < viewNum; ++i) {
		context->ClearRenderTargetView(rtv[i], clearColor);
	}
	context->ClearDepthStencilView(m_DLSSDepthDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}
// DLSS用のシェーダーリソースを指定
void SceneDemo::SetDLSSShaderResource()
{
	auto context = Renderer::GetDeviceContext();

	// SRVをピクセルシェーダーにバインド
	context->PSSetShaderResources(0, GBUFFER_NUM, m_DLSSGBufferSRVs);
	// DSVで書き込んだ深度値をバインド
	context->PSSetShaderResources(3, 1, &m_DLSSDepthSRV);
}
// オフスクリーン先のレンダーターゲットを指定
void SceneDemo::SetOffScreenRenderTarget()
{
	auto context = Renderer::GetDeviceContext();


	// オフスクリーンレンダーターゲットをセット
	context->OMSetRenderTargets(1, &m_OffScreenRTV, nullptr);

	// クリア
	float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	context->ClearRenderTargetView(m_OffScreenRTV, clearColor);
}
// オフスクリーンのシェーダーリソースをバインド
void SceneDemo::SetOffScreenResource()
{
	auto context = Renderer::GetDeviceContext();

	// SRVをピクセルシェーダーにバインド
	context->PSSetShaderResources(0, 1, &m_OffScreenSRV);

}
// アップスケール後のリソースをバインド
void SceneDemo::SetDLSSOutputResource()
{
	auto context = Renderer::GetDeviceContext();

	// SRVをピクセルシェーダーにバインド
	context->PSSetShaderResources(0, 1, &m_DLSSOutputSRV);
}



