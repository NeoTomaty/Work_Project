#include "Renderer.h"
#include "Camera.h"
#include <iostream>

#pragma comment(lib, "dxgi.lib")

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

// アップスケール解像度
#define OUTPUT_RENDRERSIZE_WIDTH	(1920 * 2)
#define OUTPUT_RENDRERSIZE_HEIGHT	(1080 * 2)

// アダプター・レンダリング関連
D3D_FEATURE_LEVEL		Renderer::m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;
IDXGIFactory*			Renderer::m_Factory;
IDXGIAdapter*			Renderer::m_Adapter;
bool					Renderer::m_IsDisableNvidia = false;

// レンダリングサイズ
// アップスケール前のレンダーターゲットサイズ
XMUINT2					Renderer::m_InputRenderSize;
// アップスケール後のレンダーターゲットサイズ（3840x2160）（4K）
XMUINT2					Renderer::m_OutputRenderSize;

// 基本機能
ID3D11Device*			Renderer::m_Device{};
ID3D11DeviceContext*	Renderer::m_DeviceContext{};
IDXGISwapChain*			Renderer::m_SwapChain{};
ID3D11RenderTargetView*	Renderer::m_DefaultRTV{};
ID3D11DepthStencilView*	Renderer::m_DefaultDSV{};

// 定数バッファリソース
ID3D11Buffer*			Renderer::m_WorldBuffer{};
ID3D11Buffer*			Renderer::m_ViewBuffer{};
ID3D11Buffer*			Renderer::m_ProjectionBuffer{};
ID3D11Buffer*			Renderer::m_MaterialBuffer{};
ID3D11Buffer*			Renderer::m_LightBuffer{};
ID3D11Buffer*			Renderer::m_CameraBuffer{};
ID3D11Buffer*			Renderer::m_TexParamBuffer{};

// 深度ステンシル
ID3D11DepthStencilState* Renderer::m_DepthStateEnable{};
ID3D11DepthStencilState* Renderer::m_DepthStateDisable{};

// ラスタライザーステート
ID3D11RasterizerState* Renderer::m_RsCullFront = nullptr;	// カリング（表を描画しない）
ID3D11RasterizerState* Renderer::m_RsCullBack = nullptr;	// カリング（裏を描画しない）
ID3D11RasterizerState* Renderer::m_RsCullNone = nullptr;	// カリング（カリングなし）



void Renderer::Init()
{
	HRESULT hr = S_OK;

	// レンダリングサイズ
	m_InputRenderSize =			// アップスケール前のレンダーターゲットサイズ（ウィンドウサイズ）
	{ Application::GetWidth(),Application::GetHeight() };
	m_OutputRenderSize =		// アップスケール後のレンダーターゲットサイズ（3840x2160）（4K）
	{ OUTPUT_RENDRERSIZE_WIDTH, OUTPUT_RENDRERSIZE_HEIGHT };

	//=====================================================
	// NVIDIAのGPUを探す
	//=====================================================
	CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&m_Factory);
	for (UINT i = 0; m_Factory->EnumAdapters(i, &m_Adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
		DXGI_ADAPTER_DESC desc;
		m_Adapter->GetDesc(&desc);

		// NVIDIAを使用したくないとき
		//break;

		// NVIDIA GPUを選択する条件
		if (wcsstr(desc.Description, L"NVIDIA")) {
			std::cout << "NVIDIA GPUを使用します" << std::endl;
			std::wcout << L"GPU Detected: " << desc.Description << std::endl;
			m_IsDisableNvidia = true;
			break; // 見つかったNVIDIA GPUを使用
		}

		m_Adapter->Release();
	}

	//=====================================================
	// デバイス・スワップチェーン作成
	//=====================================================
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.Width = Application::GetWidth();
	swapChainDesc.BufferDesc.Height = Application::GetHeight();
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = Application::GetWindow();
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = TRUE;

	// 生成
	hr = D3D11CreateDeviceAndSwapChain(m_Adapter,D3D_DRIVER_TYPE_UNKNOWN,NULL,0,NULL,0,
		D3D11_SDK_VERSION,&swapChainDesc,&m_SwapChain,&m_Device,&m_FeatureLevel,&m_DeviceContext);


	//=====================================================
	// レンダーターゲット
	//=====================================================
	
	ID3D11Texture2D* tempTexture{};

	// デフォルトレンダーターゲット
	// デフォルトのレンダーターゲットをバックバッファに指定
	m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&tempTexture);
	// デフォルトレンダーターゲットビュー作成

	m_Device->CreateRenderTargetView(tempTexture, nullptr, &m_DefaultRTV);
	tempTexture->Release();


	//=====================================================
	// 深度ステンシル
	//=====================================================
	ID3D11Texture2D* depthStencile{};
	D3D11_TEXTURE2D_DESC textureDesc = {};


	// デフォルト深度バッファ
	textureDesc.Width = m_InputRenderSize.x;
	textureDesc.Height = m_InputRenderSize.y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_D16_UNORM;
	textureDesc.SampleDesc = swapChainDesc.SampleDesc;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	hr = m_Device->CreateTexture2D(&textureDesc, NULL, &depthStencile);
	if (FAILED(hr)) { cout << "デフォルト深度ステンシルテクスチャの作成に失敗しました" << endl; }

	// 深度ステンシルビュー作成
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = textureDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = 0;
	hr = m_Device->CreateDepthStencilView(depthStencile, &depthStencilViewDesc, &m_DefaultDSV);
	if (FAILED(hr)) { cout << "デフォルト深度ステンシルビューの作成に失敗しました" << endl; }

	depthStencile->Release();


	//=====================================================
	// ラスタライザステート設定
	//=====================================================
	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_FRONT;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.SlopeScaledDepthBias = 4.0f; // スロープスケールド深度バイアス
	rasterizerDesc.DepthBias = 100;             // 深度バイアス
	// 生成
	m_Device->CreateRasterizerState(&rasterizerDesc, &m_RsCullFront);
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	m_Device->CreateRasterizerState(&rasterizerDesc, &m_RsCullBack);
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	m_Device->CreateRasterizerState(&rasterizerDesc, &m_RsCullNone);

	m_DeviceContext->RSSetState(m_RsCullBack);


	//=====================================================
	// デプスステンシルステート設定
	//=====================================================
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthStencilDesc.StencilEnable = FALSE;

	m_Device->CreateDepthStencilState( &depthStencilDesc, &m_DepthStateEnable );//深度有効ステート


	depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ZERO;
	m_Device->CreateDepthStencilState( &depthStencilDesc, &m_DepthStateDisable );//深度無効ステート

	m_DeviceContext->OMSetDepthStencilState( m_DepthStateEnable, NULL );


	//=====================================================
	// サンプラーステート設定
	//=====================================================

	// サンプラーステートの作成・セット
	SetSamplerState();

	//=====================================================
	// ビューポート設定
	//=====================================================

	// ビューポートのセット
	SetViewPort();

	//=====================================================
	// 定数バッファの生成
	//=====================================================

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(MatrixInfo);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = sizeof(float);

	// カメラ
	m_Device->CreateBuffer( &bufferDesc, NULL, &m_WorldBuffer );
	m_DeviceContext->VSSetConstantBuffers( 0, 1, &m_WorldBuffer);

	m_Device->CreateBuffer( &bufferDesc, NULL, &m_ViewBuffer );
	m_DeviceContext->VSSetConstantBuffers( 1, 1, &m_ViewBuffer );

	m_Device->CreateBuffer( &bufferDesc, NULL, &m_ProjectionBuffer );
	m_DeviceContext->VSSetConstantBuffers( 2, 1, &m_ProjectionBuffer );


	// マテリアル
	bufferDesc.ByteWidth = sizeof(MATERIAL);

	m_Device->CreateBuffer( &bufferDesc, NULL, &m_MaterialBuffer );
	m_DeviceContext->VSSetConstantBuffers( 3, 1, &m_MaterialBuffer );
	m_DeviceContext->PSSetConstantBuffers( 3, 1, &m_MaterialBuffer );


	// ライト
	bufferDesc.ByteWidth = sizeof(LIGHT);

	m_Device->CreateBuffer( &bufferDesc, NULL, &m_LightBuffer );
	m_DeviceContext->VSSetConstantBuffers( 4, 1, &m_LightBuffer );
	m_DeviceContext->PSSetConstantBuffers( 4, 1, &m_LightBuffer );


	// カメラ（座標）
	bufferDesc.ByteWidth = sizeof(CAMERAPARAM);

	m_Device->CreateBuffer(&bufferDesc, NULL, &m_CameraBuffer);
	m_DeviceContext->VSSetConstantBuffers(5, 1, &m_CameraBuffer);
	m_DeviceContext->PSSetConstantBuffers(5, 1, &m_CameraBuffer);


	// テクスチャパラメータ
	bufferDesc.ByteWidth = sizeof(TEXPARAM);

	m_Device->CreateBuffer(&bufferDesc, NULL, &m_TexParamBuffer);
	m_DeviceContext->VSSetConstantBuffers(6, 1, &m_TexParamBuffer);
	m_DeviceContext->PSSetConstantBuffers(6, 1, &m_TexParamBuffer);


	// マテリアル初期化
	MATERIAL material{};
	material.Diffuse = Color(1.0f, 1.0f, 1.0f, 1.0f);
	material.Ambient = Color(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

}


// リソースの解放処理
void Renderer::Uninit()
{

	// 定数バッファ
	m_WorldBuffer->Release();
	m_WorldBuffer = nullptr;
	m_ViewBuffer->Release();
	m_ViewBuffer = nullptr;
	m_ProjectionBuffer->Release();
	m_ProjectionBuffer = nullptr;
	m_LightBuffer->Release();
	m_LightBuffer = nullptr;
	m_MaterialBuffer->Release();
	m_MaterialBuffer = nullptr;
	m_CameraBuffer->Release();
	m_CameraBuffer = nullptr;
	m_TexParamBuffer->Release();
	m_TexParamBuffer = nullptr;

	// 基礎機能
	m_DeviceContext->ClearState();
	m_DefaultRTV->Release();
	m_DefaultRTV = nullptr;
	m_DefaultDSV->Release();
	m_DefaultDSV = nullptr;
	m_SwapChain->Release();
	m_SwapChain = nullptr;
	m_Adapter->Release();
	m_Adapter = nullptr;
	m_Factory->Release();
	m_Factory = nullptr;

	// ラスタライザステート
	m_DepthStateEnable->Release();
	m_DepthStateEnable = nullptr;
	m_DepthStateDisable->Release();
	m_DepthStateDisable = nullptr;
	m_RsCullFront->Release();
	m_RsCullFront = nullptr;
	m_RsCullBack->Release();
	m_RsCullBack = nullptr;
	m_RsCullNone->Release();
	m_RsCullNone = nullptr;

	// デバイス関連
	m_DeviceContext->Release();
	m_DeviceContext = nullptr;
	m_Device->Release();
	m_Device = nullptr;
}



// 描画前の処理
void Renderer::Begin()
{
	// レンダーターゲットのクリア
	ClearDefaultRenderTarget(true);
}
// スワップ処理（描画終了前の処理）
void Renderer::End()
{
	m_SwapChain->Present( 1, 0 );
}



// 深度値のON/OFF
void Renderer::SetDepthEnable( bool Enable )
{
	if( Enable )
		m_DeviceContext->OMSetDepthStencilState( m_DepthStateEnable, NULL );
	else
		m_DeviceContext->OMSetDepthStencilState( m_DepthStateDisable, NULL );

}



// 平行投影用のカメラ
void Renderer::SetWorldViewProjection2D()
{
	Matrix world;
	world = Matrix::Identity;			// 単位行列にする
	world = world.Transpose();			// 転置
	m_DeviceContext->UpdateSubresource(m_WorldBuffer, 0, NULL, &world, 0, 0);

	Matrix view;
	view = Matrix::Identity;			// 単位行列にする
	view = view.Transpose();			// 転置
	m_DeviceContext->UpdateSubresource(m_ViewBuffer, 0, NULL, &view, 0, 0);

	Matrix projection;

	// 2D描画を左上原点にする
	projection = DirectX::XMMatrixOrthographicOffCenterLH(
		0.0f,
		static_cast<float>(Application::GetWidth()),		// ビューボリュームの最小Ｘ
		static_cast<float>(Application::GetHeight()),		// ビューボリュームの最小Ｙ
		0.0f,												// ビューボリュームの最大Ｙ
		0.0f,
		1.0f);

	projection = projection.Transpose();


	m_DeviceContext->UpdateSubresource( m_ProjectionBuffer, 0, NULL, &projection, 0, 0 );
}



// ワールド変換行列のセット
void Renderer::SetWorldMatrix( MatrixInfo* WorldMatrix )
{
	MatrixInfo world;

	// 転置
	world.Mat = WorldMatrix->Mat.Transpose();
	world.InvMat = WorldMatrix->InvMat.Transpose();
	world.PrevMat = WorldMatrix->PrevMat.Transpose();

	m_DeviceContext->UpdateSubresource(m_WorldBuffer, 0, NULL, &world, 0, 0);

}
// ビュー変換行列のセット
void Renderer::SetViewMatrix( MatrixInfo* ViewMatrix )
{
	MatrixInfo view;

	// 転置
	view.Mat = ViewMatrix->Mat.Transpose();
	view.InvMat = ViewMatrix->InvMat.Transpose();
	view.PrevMat = ViewMatrix->PrevMat.Transpose();

	m_DeviceContext->UpdateSubresource(m_ViewBuffer, 0, NULL, &view, 0, 0);
}
// プロジェクション変換行列のセット
void Renderer::SetProjectionMatrix( MatrixInfo* ProjectionMatrix )
{
	MatrixInfo projection;

	// 転置
	projection.Mat = ProjectionMatrix->Mat.Transpose();
	projection.InvMat = ProjectionMatrix->InvMat.Transpose();
	projection.PrevMat = ProjectionMatrix->PrevMat.Transpose();


	m_DeviceContext->UpdateSubresource(m_ProjectionBuffer, 0, NULL, &projection, 0, 0);
}
// マテリアルのセット
void Renderer::SetMaterial( MATERIAL Material )
{
	m_DeviceContext->UpdateSubresource( m_MaterialBuffer, 0, NULL, &Material, 0, 0 );
}
// ライトのセット
void Renderer::SetLight( LIGHT Light )
{
	m_DeviceContext->UpdateSubresource(m_LightBuffer, 0, NULL, &Light, 0, 0);
}
// カメラのセット
void Renderer::SetCamera(Camera camera)
{
	CAMERAPARAM param;

	param.Position = camera.GetPosition();
	param.nearClip = camera.GetNearClip();
	param.farClip = camera.GetFarClip();
	param.renderSizeX = m_InputRenderSize.x;
	param.renderSizeY = m_InputRenderSize.y;

	m_DeviceContext->UpdateSubresource(m_CameraBuffer, 0, NULL, &param, 0, 0);
}
// テクスチャパラメータのセット
void Renderer::SetTextureParam(TEXPARAM param)
{
	m_DeviceContext->UpdateSubresource(m_TexParamBuffer, 0, NULL, &param, 0, 0);
}



// サンプラーステートのセット
void Renderer::SetSamplerState(float LodBias)
{
	// LodBiasを動的に変更できるように
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = 16;  // 異方性フィルタの強度
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerDesc.MipLODBias = LodBias;

	// サンプラーステートの作成・セット
	ID3D11SamplerState* samplerState{};
	m_Device->CreateSamplerState(&samplerDesc, &samplerState);
	m_DeviceContext->PSSetSamplers(0, 1, &samplerState);

	// 解放
	samplerState->Release();
	samplerState = nullptr;
}
// ビューポートのセット
void Renderer::SetViewPort(DirectX::XMUINT2 renderSize)
{
	D3D11_VIEWPORT viewport;
	viewport.Width = (FLOAT)renderSize.x;
	viewport.Height = (FLOAT)renderSize.y;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	m_DeviceContext->RSSetViewports(1, &viewport);

}



// 頂点シェーダーの作成
void Renderer::CreateVertexShader( ID3D11VertexShader** VertexShader, ID3D11InputLayout** VertexLayout, const char* FileName )
{

	FILE* file;
	long int fsize;

	fopen_s(&file,FileName, "rb");
	assert(file);

	fsize = _filelength(_fileno(file));
	unsigned char* buffer = new unsigned char[fsize];
	fread(buffer, fsize, 1, file);
	fclose(file);

	m_Device->CreateVertexShader(buffer, fsize, NULL, VertexShader);


	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0,	0,		D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,		0,	4 * 3,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	4 * 6,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0,	4 * 10, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(layout);

	m_Device->CreateInputLayout(layout,
		numElements,
		buffer,
		fsize,
		VertexLayout);

	delete[] buffer;
}
// ピクセルシェーダーの作成
void Renderer::CreatePixelShader( ID3D11PixelShader** PixelShader, const char* FileName )
{
	FILE* file;
	long int fsize;

	fopen_s(&file, FileName, "rb");
	assert(file);

	fsize = _filelength(_fileno(file));
	unsigned char* buffer = new unsigned char[fsize];
	fread(buffer, fsize, 1, file);
	fclose(file);

	m_Device->CreatePixelShader(buffer, fsize, NULL, PixelShader);

	delete[] buffer;
}



// カリングモードを設定
void Renderer::SetCullMode(CULLMODE type)
{
	switch (type)
	{
	case CULLMODE::BACK:
		m_DeviceContext->RSSetState(m_RsCullBack);
		break;
	case CULLMODE::FRONT:
		m_DeviceContext->RSSetState(m_RsCullFront);
		break;
	case CULLMODE::NONE:
		m_DeviceContext->RSSetState(m_RsCullNone);
		break;
	}

}
// デフォルトのレンダーターゲットをクリア・セット
void Renderer::ClearDefaultRenderTarget(bool isDSV)
{
	auto dsv = m_DefaultDSV;

	// DSVを使用しない場合はnullptr
	if (!isDSV)
	{
		dsv = nullptr;
	}

	// レンダーターゲットのセット
	m_DeviceContext->OMSetRenderTargets(1, &m_DefaultRTV, dsv);

	// 青色でクリア
	float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };

	// RTV及びDSVをクリア
	m_DeviceContext->ClearRenderTargetView(m_DefaultRTV, clearColor);
	m_DeviceContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

}



