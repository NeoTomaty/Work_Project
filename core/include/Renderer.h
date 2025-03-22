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

// �O�����C�u����
#pragma comment(lib,"directxtk.lib")
#pragma comment(lib,"d3d11.lib")

#define GBUFFER_NUM	(5)	// G�o�b�t�@�̐�


// 3D���_�f�[�^
struct VERTEX_3D
{
	DirectX::SimpleMath::Vector3	Position;
	DirectX::SimpleMath::Vector3	Normal;
	DirectX::SimpleMath::Color		Diffuse;
	DirectX::SimpleMath::Vector2	TexCoord;
	DirectX::SimpleMath::Vector3	Tangent;
};

// �}�e���A��
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

// �s��
struct MatrixInfo
{
	DirectX::SimpleMath::Matrix Mat;		// ���t���[���̍s��
	DirectX::SimpleMath::Matrix InvMat;		// ���t���[���̋t�s��
	DirectX::SimpleMath::Matrix PrevMat;	// �O�t���[���̍s��
};



// ���s����
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

// �J�����̈ʒu��萔�o�b�t�@�ɓn�����߂̍\����
struct CAMERAPARAM
{
	DirectX::SimpleMath::Vector3 Position;	// �J�����̈ʒu
	float Padding;							// 16�o�C�g�A���C�������g���m�ۂ��邽�߂̃p�f�B���O

	float			nearClip;		// �j�A�N���b�v
	float			farClip;		// �t�@�[�N���b�v 
	unsigned int	renderSizeX;	// �����_�����O�T�C�Y�iX�j
	unsigned int	renderSizeY;	// �����_�����O�T�C�Y�iY�j
};

// �e�N�X�`���p�����[�^
struct TEXPARAM
{
	int		UseNormalMap = 0;	// �@���}�b�v���g�p���邩�ǂ���
	int		TextureType = 0;	// G�o�b�t�@�Ŏg�p����e�N�X�`���^�C�v
	float	Padding2[2];
};

// ���b�V�����i�}�e���A�����ɃT�u�Z�b�g�����݂���j
struct SUBSET {
	std::string		MtrlName;						// �}�e���A����
	unsigned int	IndexNum = 0;					// �C���f�b�N�X��
	unsigned int	VertexNum = 0;					// ���_��
	unsigned int	IndexBase = 0;					// �J�n�C���f�b�N�X
	unsigned int	VertexBase = 0;					// ���_�x�[�X
	unsigned int	MaterialIdx = 0;				// �}�e���A���C���f�b�N�X
};

// �J�����O���[�h
enum CULLMODE
{
	BACK = 0,
	FRONT,
	NONE,
};

// �[�x�X�e���V���؂�ւ�
enum DEPTHSTENCIL
{
	DS_DEFAULT,			// �f�t�H���g
	DS_CASCADESHADOW,	// �J�X�P�[�h�V���h�E
};

// �����_�[�^�[�Q�b�g��؂�ւ���ۂ̐[�x�o�b�t�@�̎��
enum class DEPTHTYPE
{
	DEFAULT,	// �f�t�H���g�[�x�o�b�t�@�i�t�H���[�h�����_�����O�j
	DEFERRED,	// �f�B�t�@�[�h�����_�����O�p�[�x�o�b�t�@
	DLSS,		// DLSS�X�[�p�[�T���v�����O�p�[�x�o�b�t�@
	NONE,		// �Ȃ��inullptr�j
};

// �o�͂���e�N�X�`���^�C�v
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

// �����_���[�N���X
class Renderer : NonCopyable
{
private:

	// �p�����[�^
	static D3D_FEATURE_LEVEL		m_FeatureLevel;
	static IDXGIFactory*			m_Factory;
	static IDXGIAdapter*			m_Adapter;

	// ��{�@�\
	static ID3D11Device*			m_Device;
	static ID3D11DeviceContext*		m_DeviceContext;
	static IDXGISwapChain*			m_SwapChain;

	// �f�t�H���g�̃����_�����O���\�[�X
	static ID3D11RenderTargetView*	m_DefaultRTV;
	static ID3D11DepthStencilView*	m_DefaultDSV;

	// �萔�o�b�t�@
	static ID3D11Buffer*			m_WorldBuffer;
	static ID3D11Buffer*			m_ViewBuffer;
	static ID3D11Buffer*			m_ProjectionBuffer;
	static ID3D11Buffer*			m_MaterialBuffer;
	static ID3D11Buffer*			m_LightBuffer;
	static ID3D11Buffer*			m_CameraBuffer;
	static ID3D11Buffer*			m_TexParamBuffer;

	// �[�x�X�e���V���X�e�[�g
	static ID3D11DepthStencilState* m_DepthStateEnable;
	static ID3D11DepthStencilState* m_DepthStateDisable;

	// ���X�^���C�U�[�X�e�[�g
	static ID3D11RasterizerState* m_RsCullFront;	// �J�����O�i�\��`�悵�Ȃ��j
	static ID3D11RasterizerState* m_RsCullBack;		// �J�����O�i����`�悵�Ȃ��j
	static ID3D11RasterizerState* m_RsCullNone;		// �J�����O�i�J�����O�Ȃ��j

	static bool				m_IsDisableNvidia;		// NVIDIA_GPU���g�p�\���H
	static DirectX::XMUINT2	m_InputRenderSize;		// �A�b�v�X�P�[���O�̃����_�[�^�[�Q�b�g�T�C�Y�i�E�B���h�E�T�C�Y�j
	static DirectX::XMUINT2	m_OutputRenderSize;		// �A�b�v�X�P�[����̃����_�[�^�[�Q�b�g�T�C�Y�i3840x2160�j�i4K�j


public:

	// �`��
	static void Init();
	static void Uninit();
	static void Begin();
	static void End();

	// �[�x�l�̉�
	static void SetDepthEnable(bool Enable);

	// �e�탊�\�[�X�̃Z�b�^
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

	// �Q�b�^
	static ID3D11Device*			GetDevice()			{ return m_Device; }
	static ID3D11DeviceContext*		GetDeviceContext()	{ return m_DeviceContext; }
	static bool						GetIsAbleNVIDIA()	{ return m_IsDisableNvidia; }
	static IDXGIAdapter*			GetAdapter()		{ return m_Adapter; }
	static ID3D11RenderTargetView*	GetDefaultRTV()		{ return m_DefaultRTV; }

	// �����_�����O�T�C�Y�̃Z�b�^�E�Q�b�^
	static DirectX::XMUINT2		GetInputRenderSize()	{ return m_InputRenderSize; }
	static DirectX::XMUINT2		GetOutputRenderSize()	{ return m_OutputRenderSize; }
	static void					SetInputRenderSize(DirectX::XMUINT2 inputSize)	
														{ m_InputRenderSize = inputSize; }

	// �V�F�[�_�[�쐬
	static void					CreateVertexShader(ID3D11VertexShader** VertexShader, ID3D11InputLayout** VertexLayout, const char* FileName);
	static void					CreatePixelShader(ID3D11PixelShader** PixelShader, const char* FileName);

	// �J�����O���[�h��ύX
	static void					SetCullMode(CULLMODE type);

	// �f�t�H���g�̃����_�[�^�[�Q�b�g���N���A�E�Z�b�g
	static void					ClearDefaultRenderTarget(bool isDSV);

};