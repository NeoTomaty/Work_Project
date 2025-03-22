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


#define GBUFFER_NUM	(5)	// G�o�b�t�@�̐��iAlbedo, Normal, WorldPos, Depth, MVector�j

// ON/OFF��
enum Enable
{
	DISABLE,
	ENABLE,
};

// �����_�����O���[�h��
enum class RenderMode {
	NONE,		// �Ȃ��i�t�H���[�h�����_�����O�j
	DEFERRED,	// �f�B�t�@�[�h�����_�����O
	DLSS,		// DLSS�X�[�p�[�T���v�����O
};

// �o�b�t�@���[�h��
enum class BufferMode {
	DEFAULT,
	COLOR,
	NORMAL,
	WORLD,
	DEPTH,
	MVECTOR,
};

// �A���`�G�C���A�V���O���[�h��
enum class  AntiAliasingMode {
	NONE,
	//TEMPORAL,
	DLSS,
};


// �V�[���f���N���X
class SceneDemo
{
public:		// �����o�֐�

	// �V�[���`��
	void SceneUpdate();		// �X�V
	void SceneInit();		// ������
	void SceneDraw();		// �`��
	void SceneDispose();	// �I��

private:

	// �e��X�V
	void UpdateDLSS();		// DLSS�̍X�V����
	void UpdateLight();		// ���C�g�̍X�V����

	// ���\�[�X����
	void CreateRenderResource();								// �t�H���[�h�����_�����O
	void CreateDeferredResource();								// �f�B�t�@�[�h�����_�����O
	void CreateDLSSResource(DirectX::XMUINT2 renderSize =		// DLSS
		{ Application::GetWidth(),Application::GetHeight() });

	// ���\�[�X�̃Z�b�g
	void SetDeferredGBufferRenderTarget();	// GBuffer�p�̃����_�[�^�[�Q�b�g
	void SetDeferredShaderResource();		// �f�B�t�@�[�h�����_�����O�p�̃V�F�[�_�[���\�[�X
	void SetDLSSRenderTarget();				// DLSS�p�̃����_�[�^�[�Q�b�g
	void SetDLSSShaderResource();			// DLSS�p�̃V�F�[�_�[���\�[�X
	void SetOffScreenRenderTarget();		// �I�t�X�N���[����̃����_�[�^�[�Q�b�g
	void SetOffScreenResource();			// �I�t�X�N���[���̃V�F�[�_�[���\�[�X
	void SetDLSSOutputResource();			// �A�b�v�X�P�[���̏o�͌���

private:	// �����o�ϐ�
	//-------------------------------------
	// �V�F�[�_�[
	//-------------------------------------
	Shader m_ShaderPBR;					// PBR�\��
	Shader m_ShaderSkysphere;			// skysphere�p
	Shader m_ShaderDeferredRendering;	// �f�B�t�@�[�h�����_�����O
	Shader m_ShaderGBuffer;				// GBuffer�o��
	Shader m_ShaderDLSSInput;			// DLSS�ɓn�����́i�I�t�X�N���[���j
	Shader m_ShaderDLSSOutput;			// DLSS�A�b�v�X�P�[����̏o��


	//-------------------------------------
	// �I�u�W�F�N�g
	//-------------------------------------
	Camera		m_Camera;	// �J����
	Field		m_Field;	// �t�B�[���h
	Polygon2D	m_Screen;	// �X�N���[���N�A�b�h

	StaticMesh			m_Skysphere;	// SkySphere
	StaticMeshRenderer	m_MRSkysphere;	// �����_���[

	StaticMesh			m_Cottage;		// Cottage
	StaticMeshRenderer	m_MRCottage;	// �����_���[

	StaticMesh			m_Pole;		// Pole
	StaticMeshRenderer	m_MRPole;	// �����_���[

	//-------------------------------------
	// �����_�[�^�[�Q�b�g�iRTV�j
	//-------------------------------------
	ID3D11RenderTargetView*		m_DeferredGBufferRTVs[GBUFFER_NUM] = { nullptr };	// G�o�b�t�@�i�f�B�t�@�[�h�����_�����O�j
	ID3D11RenderTargetView*		m_DLSSGBufferRTVs[GBUFFER_NUM] = { nullptr };		// G�o�b�t�@�iDLSS�j
	ID3D11RenderTargetView*		m_OffScreenRTV = nullptr;							// �I�t�X�N���[���iDLSS���͉摜�j

	//-------------------------------------
	// �V�F�[�_�[���\�[�X�iSRV�j
	//-------------------------------------
	ID3D11ShaderResourceView*	m_DeferredGBufferSRVs[GBUFFER_NUM] = { nullptr };	// G�o�b�t�@�i�f�B�t�@�[�h�����_�����O�j
	ID3D11ShaderResourceView*	m_DLSSGBufferSRVs[GBUFFER_NUM] = { nullptr };		// G�o�b�t�@�iDLSS�j
	ID3D11ShaderResourceView*	m_OffScreenSRV = nullptr;							// �I�t�X�N���[���iDLSS���͉摜�j
	ID3D11ShaderResourceView*	m_DLSSOutputSRV = nullptr;							// DLSS�o�͒l


	ID3D11ShaderResourceView*	m_DepthStencilSRV = nullptr;						// �[�x���
	ID3D11ShaderResourceView*	m_DLSSDepthSRV = nullptr;							// DLSS�[�x���

	//-------------------------------------
	// �[�x�X�e���V���iDSV�j
	//-------------------------------------
	ID3D11DepthStencilView*		m_DepthStencilView = nullptr;						// �[�x���
	ID3D11DepthStencilView*		m_DLSSDepthDSV = nullptr;							// DLSS�[�x���

	//-------------------------------------
	// �������݃��\�[�X�iUAV�j
	//-------------------------------------
	ID3D11UnorderedAccessView*	m_DLSSOutputUAV = nullptr;							// DLSS�o�͒l

	//-------------------------------------
	// �o�b�t�@���\�[�X�iTexture�j
	//-------------------------------------
	ID3D11Texture2D*			m_DeferredGBufferTexs[GBUFFER_NUM]= { nullptr };	// G�o�b�t�@�i�f�B�t�@�[�h�����_�����O�j
	ID3D11Texture2D*			m_DLSSGBufferTexs[GBUFFER_NUM]	= { nullptr };		// G�o�b�t�@�iDLSS�j
	ID3D11Texture2D*			m_OffScreenTex = nullptr;							// �I�t�X�N���[���i��DLSS���͒l�j
	ID3D11Texture2D*			m_DLSSOutputTex = nullptr;							// DLSS�o�͒l

	ID3D11Texture2D*			m_DepthStencilBuffer = nullptr;						// �[�x���
	ID3D11Texture2D*			m_DLSSDepthTex = nullptr;							// DLSS�[�x���

};


