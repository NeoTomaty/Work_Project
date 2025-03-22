#include "Camera.h"
#include "DirectInput.h"
#include <DirectXMath.h>
#include <iostream>	// �f�o�b�O�p

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

	// �s��̏�����
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
	// �}�E�X�ړ��ʂ��擾
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	arg.mouseMove = DirectX::XMFLOAT2((float)cursorPos.x - m_oldPos.x, (float)cursorPos.y - m_oldPos.y);
	m_oldPos = cursorPos;

	// �J���������擾
	arg.vCamPos = DirectX::XMLoadFloat3(&m_Position);
	arg.vCamLook = DirectX::XMLoadFloat3(&m_Target);
	DirectX::XMVECTOR vCamUp = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&m_up));
	DirectX::XMVECTOR vFront = DirectX::XMVectorSubtract(arg.vCamLook, arg.vCamPos);

	// �J�����p�����擾
	arg.vCamFront = DirectX::XMVector3Normalize(vFront);
	arg.vCamSide = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(vCamUp, arg.vCamFront));
	arg.vCamUp = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(arg.vCamFront, arg.vCamSide));
	
	// �t�H�[�J�X�������擾
	DirectX::XMStoreFloat(&arg.focus, DirectX::XMVector3Length(vFront));

	// �}�E�X�̈ړ��� / ��ʃT�C�Y �̔䗦����A��ʑS�̂łǂꂾ����]���邩�w��
	float angleX = ROTATESPEED * arg.mouseMove.x;
	float angleY = ROTATESPEED * arg.mouseMove.y;

	DirectX::XMVECTOR vSideAxis = arg.vCamSide;  // �����̕���
	DirectX::XMVECTOR vFrontAxis = arg.vCamFront;  // �O�㎲�̕���

	// �}�E�X�̉E�N���b�N��������Ă����
	if (input.GetMouseRButtonCheck())
	{
		// ����]
		DirectX::XMMATRIX matUpRot = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(angleX));
		vSideAxis = DirectX::XMVector3Normalize(DirectX::XMVector3TransformCoord(arg.vCamSide, matUpRot));

		// �c��]
		DirectX::XMMATRIX matSideRot = DirectX::XMMatrixRotationAxis(vSideAxis, DirectX::XMConvertToRadians(angleY));
		vFrontAxis = DirectX::XMVector3Normalize(DirectX::XMVector3TransformCoord(arg.vCamFront, matUpRot * matSideRot));

	}

	// �L�[���͂ňړ�
	DirectX::XMVECTOR vCamMove = DirectX::XMVectorZero();
	if (input.CheckKeyBuffer(DIK_W)) vCamMove = DirectX::XMVectorAdd(vCamMove, vFrontAxis);
	if (input.CheckKeyBuffer(DIK_S)) vCamMove = DirectX::XMVectorSubtract(vCamMove, vFrontAxis);
	if (input.CheckKeyBuffer(DIK_A)) vCamMove = DirectX::XMVectorSubtract(vCamMove, vSideAxis);
	if (input.CheckKeyBuffer(DIK_D)) vCamMove = DirectX::XMVectorAdd(vCamMove, vSideAxis);
	if (input.CheckKeyBuffer(DIK_Q)) vCamMove = DirectX::XMVectorAdd(vCamMove, DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	if (input.CheckKeyBuffer(DIK_E)) vCamMove = DirectX::XMVectorAdd(vCamMove, DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f));


	// ��SHIFT��������Ă���Ԃ͈ړ��ʂ��Q�{
	if (input.CheckKeyBuffer(DIK_LSHIFT)){
		vCamMove = DirectX::XMVectorScale(vCamMove, MOVESPEED*2);
	}
	else {
		vCamMove = DirectX::XMVectorScale(vCamMove, MOVESPEED);
	}

	// ���W�̍X�V
	DirectX::XMVECTOR vCamPos = DirectX::XMVectorAdd(arg.vCamPos, vCamMove);
	DirectX::XMStoreFloat3(&m_Position, vCamPos);
	DirectX::XMStoreFloat3(&m_Target, DirectX::XMVectorAdd(vCamPos, DirectX::XMVectorScale(vFrontAxis, arg.focus)));
	DirectX::XMStoreFloat3(&m_up, DirectX::XMVector3Normalize(DirectX::XMVector3Cross(vFrontAxis, vSideAxis)));

}

void Camera::Draw()
{
	// ���݂̃����_�����O�T�C�Y���擾
	XMUINT2 renderSize = Renderer::GetInputRenderSize();


	// �s�����O�t���[���ɕۑ�
	m_ViewMatrix.PrevMat = m_ViewMatrix.Mat;

	// �r���[�ϊ����쐬
	Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
	m_ViewMatrix.Mat =
		DirectX::XMMatrixLookAtLH(
			m_Position, 
			m_Target, 
			up);				// ����n

	// �t�s����Z�b�g
	m_ViewMatrix.InvMat = m_ViewMatrix.Mat.Invert();

	// �r���[�ϊ��s����Z�b�g
	Renderer::SetViewMatrix(&m_ViewMatrix);


	//�v���W�F�N�V�����s��̐���
	// �s�����O�t���[���ɕۑ�
	m_ProjMatrix.PrevMat = m_ProjMatrix.Mat;

	// ����p
	constexpr float fov = DirectX::XMConvertToRadians(45.0f);
	
	// �A�X�y�N�g��	
	float aspectRatio = static_cast<float>(renderSize.x) / static_cast<float>(renderSize.y);

	//�v���W�F�N�V�����s��̐���
	m_ProjMatrix.Mat =
		DirectX::XMMatrixPerspectiveFovLH(
			fov, 
			aspectRatio, 
			m_nearClip, 
			m_farClip);	// ����n

	// �t�s����Z�b�g
	m_ProjMatrix.InvMat = m_ProjMatrix.Mat.Invert();

	// �v���W�F�N�V�����ϊ��s����Z�b�g
	Renderer::SetProjectionMatrix(&m_ProjMatrix);
	
}