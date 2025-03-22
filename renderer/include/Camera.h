#pragma once
#include <DirectXMath.h>
#include <SimpleMath.h>
#include "Renderer.h"

struct Argument
{
	DirectX::XMFLOAT2 mouseMove;
	DirectX::XMVECTOR vCamFront;
	DirectX::XMVECTOR vCamSide;
	DirectX::XMVECTOR vCamUp;
	DirectX::XMVECTOR vCamPos;
	DirectX::XMVECTOR vCamLook;
	float focus;
};

class Camera {
private:
	DirectX::SimpleMath::Vector3	m_Position = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
	DirectX::SimpleMath::Vector3	m_Rotation = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
	DirectX::SimpleMath::Vector3	m_Scale = DirectX::SimpleMath::Vector3(1.0f, 1.0f, 1.0f);

	DirectX::SimpleMath::Vector3	m_Target{};

	float m_nearClip = 1.0f;
	float m_farClip = 1000.0f;

	// çsóÒ
	MatrixInfo m_ViewMatrix;
	MatrixInfo m_ProjMatrix;

	POINT  m_oldPos;

	DirectX::XMFLOAT3	m_up = { 0.0f,1.0f,0.0f };

	float m_RotationSpeed = 5.0f;  // âÒì]ÇÃä¥ìx



public:
	void Init();
	void Dispose();
	void Update();
	void Draw();

	float GetNearClip() {
		return m_nearClip;
	}

	float GetFarClip() {
		return m_farClip;
	}

	DirectX::SimpleMath::Vector3 GetPosition() {
		return m_Position;
	}
};