#include "Camera.h"
#include "DirectInput.h"
#include <DirectXMath.h>
#include <iostream>	// デバッグ用

#define MOVESPEED	(1.0f)
#define ROTATESPEED	(0.5f)

using namespace DirectX::SimpleMath;
using namespace DirectX;

void Camera::Init()
{
	m_Position = Vector3(0.0f, 15.0f, -40.0f);
	m_Target = Vector3(0.0f, 0.0f, 0.0f);

	m_nearClip = 5.0f;
	m_farClip = 1000.0f;

	// 行列の初期化
	m_ViewMatrix.Mat = Matrix::Identity;
	m_ViewMatrix.InvMat = Matrix::Identity;
	m_ViewMatrix.PrevMat = Matrix::Identity;
	m_ProjMatrix.Mat = Matrix::Identity;
	m_ProjMatrix.InvMat = Matrix::Identity;
	m_ProjMatrix.PrevMat = Matrix::Identity;

}

void Camera::Dispose()
{

}

void Camera::Update()
{
    DirectInput& input = DirectInput::GetInstance();

	Argument arg;
	// マウス移動量を取得
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	arg.mouseMove = DirectX::XMFLOAT2((float)cursorPos.x - m_oldPos.x, (float)cursorPos.y - m_oldPos.y);
	m_oldPos = cursorPos;

	// カメラ情報を取得
	arg.vCamPos = DirectX::XMLoadFloat3(&m_Position);
	arg.vCamLook = DirectX::XMLoadFloat3(&m_Target);
	DirectX::XMVECTOR vCamUp = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&m_up));
	DirectX::XMVECTOR vFront = DirectX::XMVectorSubtract(arg.vCamLook, arg.vCamPos);

	// カメラ姿勢を取得
	arg.vCamFront = DirectX::XMVector3Normalize(vFront);
	arg.vCamSide = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(vCamUp, arg.vCamFront));
	arg.vCamUp = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(arg.vCamFront, arg.vCamSide));
	
	// フォーカス距離を取得
	DirectX::XMStoreFloat(&arg.focus, DirectX::XMVector3Length(vFront));

	// マウスの移動量 / 画面サイズ の比率から、画面全体でどれだけ回転するか指定
	float angleX = ROTATESPEED * arg.mouseMove.x;
	float angleY = ROTATESPEED * arg.mouseMove.y;

	DirectX::XMVECTOR vSideAxis = arg.vCamSide;  // 横軸の方向
	DirectX::XMVECTOR vFrontAxis = arg.vCamFront;  // 前後軸の方向

	// マウスの右クリックが押されている間
	if (input.GetMouseRButtonCheck())
	{
		// 横回転
		DirectX::XMMATRIX matUpRot = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(angleX));
		vSideAxis = DirectX::XMVector3Normalize(DirectX::XMVector3TransformCoord(arg.vCamSide, matUpRot));

		// 縦回転
		DirectX::XMMATRIX matSideRot = DirectX::XMMatrixRotationAxis(vSideAxis, DirectX::XMConvertToRadians(angleY));
		vFrontAxis = DirectX::XMVector3Normalize(DirectX::XMVector3TransformCoord(arg.vCamFront, matUpRot * matSideRot));

	}

	// キー入力で移動
	DirectX::XMVECTOR vCamMove = DirectX::XMVectorZero();
	if (input.CheckKeyBuffer(DIK_W)) vCamMove = DirectX::XMVectorAdd(vCamMove, vFrontAxis);
	if (input.CheckKeyBuffer(DIK_S)) vCamMove = DirectX::XMVectorSubtract(vCamMove, vFrontAxis);
	if (input.CheckKeyBuffer(DIK_A)) vCamMove = DirectX::XMVectorSubtract(vCamMove, vSideAxis);
	if (input.CheckKeyBuffer(DIK_D)) vCamMove = DirectX::XMVectorAdd(vCamMove, vSideAxis);
	if (input.CheckKeyBuffer(DIK_Q)) vCamMove = DirectX::XMVectorAdd(vCamMove, DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	if (input.CheckKeyBuffer(DIK_E)) vCamMove = DirectX::XMVectorAdd(vCamMove, DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f));


	// 左SHIFTが押されている間は移動量が２倍
	if (input.CheckKeyBuffer(DIK_LSHIFT)){
		vCamMove = DirectX::XMVectorScale(vCamMove, MOVESPEED*2);
	}
	else {
		vCamMove = DirectX::XMVectorScale(vCamMove, MOVESPEED);
	}

	// 座標の更新
	DirectX::XMVECTOR vCamPos = DirectX::XMVectorAdd(arg.vCamPos, vCamMove);
	DirectX::XMStoreFloat3(&m_Position, vCamPos);
	DirectX::XMStoreFloat3(&m_Target, DirectX::XMVectorAdd(vCamPos, DirectX::XMVectorScale(vFrontAxis, arg.focus)));
	DirectX::XMStoreFloat3(&m_up, DirectX::XMVector3Normalize(DirectX::XMVector3Cross(vFrontAxis, vSideAxis)));

}

void Camera::Draw()
{
	// 現在のレンダリングサイズを取得
	XMUINT2 renderSize = Renderer::GetInputRenderSize();


	// 行列情報を前フレームに保存
	m_ViewMatrix.PrevMat = m_ViewMatrix.Mat;

	// ビュー変換後列作成
	Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
	m_ViewMatrix.Mat =
		DirectX::XMMatrixLookAtLH(
			m_Position, 
			m_Target, 
			up);				// 左手系

	// 逆行列をセット
	m_ViewMatrix.InvMat = m_ViewMatrix.Mat.Invert();

	// ビュー変換行列をセット
	Renderer::SetViewMatrix(&m_ViewMatrix);


	//プロジェクション行列の生成
	// 行列情報を前フレームに保存
	m_ProjMatrix.PrevMat = m_ProjMatrix.Mat;

	// 視野角
	constexpr float fov = DirectX::XMConvertToRadians(45.0f);
	
	// アスペクト比	
	float aspectRatio = static_cast<float>(renderSize.x) / static_cast<float>(renderSize.y);

	//プロジェクション行列の生成
	m_ProjMatrix.Mat =
		DirectX::XMMatrixPerspectiveFovLH(
			fov, 
			aspectRatio, 
			m_nearClip, 
			m_farClip);	// 左手系

	// 逆行列をセット
	m_ProjMatrix.InvMat = m_ProjMatrix.Mat.Invert();

	// プロジェクション変換行列をセット
	Renderer::SetProjectionMatrix(&m_ProjMatrix);
	
}