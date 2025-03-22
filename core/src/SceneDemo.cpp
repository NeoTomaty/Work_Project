#include "SceneDemo.h"
#include "DLSSManager.h"
#include <map>

using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace std;

// �N�I���e�B���X�g
std::vector<PERF_QUALITY_ITEM> PERF_QUALITY_LIST =
{
	{NVSDK_NGX_PerfQuality_Value_MaxPerf,          "Performance", false, false},	// �����_�����O�{��2.0�{
	{NVSDK_NGX_PerfQuality_Value_Balanced,         "Balanced"   , false, false},	// �����_�����O�{��1.7�{
	{NVSDK_NGX_PerfQuality_Value_MaxQuality,       "Quality"    , false, false},	// �����_�����O�{��1.5�{
	{NVSDK_NGX_PerfQuality_Value_UltraPerformance, "UltraPerf"  , false, false},	// �����_�����O�{��3.0�{
	{NVSDK_NGX_PerfQuality_Value_DLAA,             "DLAA"       , false, false},	// �A���`�G�C���A�V���O
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

// ���I�ɕω�����p�����[�^�iImGui�ŕύX�j
static LIGHT	pointLight{};					// �|�C���g���C�g
static float	lightLength = 50.0f;			// ���_���烉�C�g�܂ł̋���
static float	radY = 45;						// ���C�g�̉�]��Y
static float	radXZ = 135;					// ���C�g�̉�]��XZ
static TEXPARAM	texParam{};						// �e�N�X�`�����
static int renderPresetIndex = 0;				// ���ݑI������Ă���v���Z�b�g�̃C���f�b�N�X
static int perfQualityIndex = 0;				// ���ݑI������Ă���p�t�H�[�}���X�i���̃C���f�b�N�X
static int bufferMode = 0;						// ���ݑI������Ă���o�b�t�@���[�h
static bool isResetResource = false;			// �~�ς��ꂽ�p�����[�^��j�����邩
static bool isInitDLSS = false;					// DLSS�@�\�̏��������o���Ă��邩�H
static bool isUseDLSSFeature = false;			// DLSS���g�p���邩�ǂ���
RenderMode g_RenderMode = RenderMode::DLSS;		// �����_�����O���[�h
AntiAliasingMode g_AntiAliasingMode				// �A���`�G�C���A�V���O���[�h
					= AntiAliasingMode::DLSS;

// DLSS�̐����ݒ�i�掿���[�h�ʁj
map<NVSDK_NGX_PerfQuality_Value, DlssRecommendedSettings> g_RecommendedSettingsMap;

// �O�t���[���̃N�H���e�B���[�h
NVSDK_NGX_PerfQuality_Value PrevQuality = 
PERF_QUALITY_LIST[perfQualityIndex].PerfQuality;
// ���t���[���̃N�H���e�B���[�h
NVSDK_NGX_PerfQuality_Value PerfQualityMode =
PERF_QUALITY_LIST[perfQualityIndex].PerfQuality;



// DLSS�f�o�b�O
void DebugDLSS() {

	// �f�B�t�@�[�h�����_�����O�O��Ȃ̂�
	// �����_�����O���[�h��DEFERRED�ȊO�̏ꍇ�͎g�p�ł��Ȃ��悤�ɂ���
	if (g_RenderMode == RenderMode::DLSS)
	{
		ImGui::Begin("DLSSSetting");

		auto input = Renderer::GetInputRenderSize();
		auto output = Renderer::GetOutputRenderSize();

		// ���݂̃����_�����O�T�C�Y��\��
		ImGui::Text("Input Render Size: %d x %d", input.x, input.y);
		ImGui::Text("Output Render Size: %d x %d", output.x, output.y);

		// DLSS�Œ~�ς������\�[�X����������ON/OFF
		if (ImGui::Checkbox("Use DLSS", &isUseDLSSFeature)) {
			// �`�F�b�N��Ԃ��ύX���ꂽ�Ƃ��ɌĂяo�����
			if (isUseDLSSFeature) {
				printf("Use DLSS Enabled\n");
			}
			else {
				printf("Use DLSS Disabled\n");
			}
		}

		// DLSS�Œ~�ς������\�[�X����������ON/OFF
		if (ImGui::Checkbox("Reset Resource", &isResetResource)) {
			// �`�F�b�N��Ԃ��ύX���ꂽ�Ƃ��ɌĂяo�����
			if (isResetResource) {
				printf("Reset Enabled\n");
			}
			else {
				printf("Reset Disabled\n");
			}
		}

		// �A���`�G�C���A�V���O���[�h�̑I��
		const char* aaModes[] = { "NONE", "DLSS" };
		int currentAAMode = static_cast<int>(g_AntiAliasingMode); // ���݂̃��[�h�𐮐��l�ɕϊ�
		if (ImGui::Combo("Anti-Aliasing Mode", &currentAAMode, aaModes, IM_ARRAYSIZE(aaModes))) {
			g_AntiAliasingMode = static_cast<AntiAliasingMode>(currentAAMode); // �I�����ꂽ���[�h�𒼐ړK�p
			printf("Selected Anti-Aliasing Mode: %s\n", aaModes[currentAAMode]);
		}

		// �p�t�H�[�}���X�i���̑I��
		std::vector<const char*> perfQualityItems;

		for (const auto& quality : PERF_QUALITY_LIST) {
			perfQualityItems.push_back(quality.PerfQualityText);
		}

		// �A���`�G�C���A�V���O���[�h��DLSS�̎�
		if (g_AntiAliasingMode == AntiAliasingMode::DLSS)
		{
			if (ImGui::Combo("DLSS Performance Quality", &perfQualityIndex, perfQualityItems.data(), static_cast<int>(perfQualityItems.size()))) {
				// �p�t�H�[�}���X�i�����ύX���ꂽ�ꍇ�ɌĂяo�����
				printf("Selected Performance Quality: %s\n", perfQualityItems[perfQualityIndex]);
			}
		}

		ImGui::End();
	}
}
// ���C�e�B���O�f�o�b�O
void DebugLight()
{
	ImGui::Begin("LightParameter");

	// ���C�g�̗L��
	bool enable = (pointLight.Enable == TRUE); // BOOL -> bool �ɕϊ�
	// �`�F�b�N�{�b�N�X�̒l���X�V
	if (ImGui::Checkbox("Light Enabled", &enable))
	{
		pointLight.Enable = (enable ? TRUE : FALSE); // bool -> BOOL �ɕϊ�
	}

	// ���C�g�����̕\��
	ImGui::Text("Direction: (%.2f, %.2f, %.2f)",
		pointLight.Direction.x,
		pointLight.Direction.y,
		pointLight.Direction.z);

	// XZ��]��Y��]
	float radXZDegrees = DirectX::XMConvertToDegrees(radXZ);	// XZ��]�i�x���@�ɕϊ��j
	if (ImGui::SliderFloat("Rotation XZ (Yaw)", &radXZDegrees, 0.0f, 360.0f)) {
		radXZ = DirectX::XMConvertToRadians(radXZDegrees);		// �x���@���烉�W�A���ɖ߂�
	}
	float radYDegrees = DirectX::XMConvertToDegrees(radY);		// Y��]�i�x���@�ɕϊ��j
	if (ImGui::SliderFloat("Rotation Y (Pitch)", &radYDegrees, 0.0f, 90.0f)) {
		radY = DirectX::XMConvertToRadians(radYDegrees);		// �x���@���烉�W�A���ɖ߂�
	}

	// ���C�g�̈ʒu
	ImGui::Text("Light Position: (%.2f, %.2f, %.2f)", pointLight.Position.x, pointLight.Position.y, pointLight.Position.z);

	// ���� (length) �̒���
	ImGui::SliderFloat("Light Distance (Length)", &lightLength, 10.0f, 100.0f);


	// �f�B�t���[�Y
	static float diffuse[3] = { pointLight.Diffuse.x, pointLight.Diffuse.y, pointLight.Diffuse.z };
	if (ImGui::ColorEdit3("Diffuse", diffuse))
	{
		pointLight.Diffuse = Color(diffuse[0], diffuse[1], diffuse[2], 1.0f);
	}

	// �A���r�G���g
	static float ambient[3] = { pointLight.Ambient.x, pointLight.Ambient.y, pointLight.Ambient.z };
	if (ImGui::ColorEdit3("Ambient", ambient))
	{
		pointLight.Ambient = Color(ambient[0], ambient[1], ambient[2], 1.0f);
	}

	ImGui::End();
}
// �����_�����O�f�o�b�O
void DebugRender()
{
	ImGui::Begin("RenderingSetting");

	// �J�����O���[�h
	static int cullmode = CULLMODE::BACK; // ���ݑI������Ă���J�����O

	// �\������I�����̃��X�g
	const char* items[] = { "BACK", "FRONT", "NONE" };

	// Combo�{�b�N�X�̕`��
	if (ImGui::Combo("Select CullMode", &cullmode, items, IM_ARRAYSIZE(items))) {
		// �I�����ꂽ���ڂ��ύX���ꂽ�ꍇ�ɌĂяo�����
		printf("Selected option: %s\n", items[cullmode]);
		Renderer::SetCullMode(static_cast<CULLMODE>(cullmode));
	}

	// �����_�����O���[�h�I��p�̐ÓI�ϐ�
	static int renderMode = static_cast<int>(g_RenderMode);

	// �����_�����O���[�h�̑I�����i������z��j
	const char* renderItems[] = { "NONE", "DEFERRED", "DEFERRED+DLSS"};

	// Combo�{�b�N�X�Ń����_�����O���[�h��I��
	if (ImGui::Combo("Select RenderMode", &renderMode, renderItems, IM_ARRAYSIZE(renderItems))) {
		// ���[�U�[���I����ύX�����ꍇ�ɌĂяo�����
		printf("Selected RenderMode: %s\n", renderItems[renderMode]);

		// �O���[�o���ϐ����X�V
		g_RenderMode = static_cast<RenderMode>(renderMode);
	}

	// DEFERRED���[�h�̏ꍇ�ɒǉ��̑I�����ڂ�\��
	if (g_RenderMode == RenderMode::DEFERRED) {

		// �o�b�t�@�I�����i������z��j
		const char* bufferItems[] = { "DEFAULT", "COLOR", "NORMAL", "WORLD", "DEPTH", "MVECTOR"};

		// Combo�{�b�N�X�Ńo�b�t�@��I��
		if (ImGui::Combo("Select Buffer", &bufferMode, bufferItems, IM_ARRAYSIZE(bufferItems))) {
			// �I�����ꂽ�o�b�t�@�ɉ����ď���
			printf("Selected Buffer: %s\n", bufferItems[bufferMode]);
		}
	}

	ImGui::End();
}



// �V�[���̏�����
void SceneDemo::SceneInit()
{
	// �J����������
	m_Camera.Init();

	// �X�N���[��������
	m_Screen.Init();

	// �t�B�[���h������
	m_Field.Init();

	// ���C�g�̏�����
	{
		pointLight.Enable = true;
		pointLight.Position = Vector3(100.0f, 100.0f, 100.0f);	// DirectionalLight�̏ꍇ�͖��g�p
		pointLight.Direction = Vector4(0.5f, -1.0f, 0.8f, 0.0f);
		pointLight.Direction.Normalize();
		pointLight.Ambient = Color(0.5f, 0.5f, 0.5f, 1.0f);
		pointLight.Diffuse = Color(1.5f, 1.5f, 1.5f, 1.0f);
		Renderer::SetLight(pointLight);
	}

	// �����_�����O���\�[�X�̐���
	CreateRenderResource();

	// Imgui�p�f�o�b�O�֐��Ăяo��
	DebugUI::RedistDebugFunction(DebugLight);
	DebugUI::RedistDebugFunction(DebugDLSS);
	DebugUI::RedistDebugFunction(DebugRender);

	// ���f���t�@�C���p�X
	std::vector<std::string> filename =
	{
		"assets/model/Cottage/cottage.fbx",
		"assets/model/SkySphere.fbx",
		"assets/model/Cube.fbx",
	};
	// �e�N�X�`���t�@�C���p�X
	std::vector<std::string> texfilename =
	{
		"assets/model/Cottage/cottage_diffuse.png",
		"assets/model/Cottage/cottage_normal.png",
		"assets/texture/bg.jpg",
	};
	// �V�F�[�_�[�t�@�C���p�X
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

	// �V�F�[�_�[
	{
		// �t�H���[�h�����_�����O���[�h
		m_ShaderPBR.Create(shaderfile[2], shaderfile[1]);				// PBR
		// �f�B�t�@�[�h�����_�����O���[�h
		m_ShaderGBuffer.Create(shaderfile[2], shaderfile[4]);			// G�o�b�t�@�o��
		m_ShaderDeferredRendering.Create(shaderfile[2], shaderfile[5]);	// �f�B�t�@�[�h�����_�����O
		// DLSS���[�h
		m_ShaderDLSSInput.Create(shaderfile[2], shaderfile[6]);			// DLSS�̓��͒l�i�I�t�X�N���[���j
		m_ShaderDLSSOutput.Create(shaderfile[2], shaderfile[7]);		// DLSS�̏o�͒l
		// ���̑��E���f���ʗp
		m_ShaderSkysphere.Create(shaderfile[2], shaderfile[3]);			// skybox

	}

	// SkyBox�ǂݍ���
	{
		std::string f = filename[1];

		// ���f���ǂݍ���
		m_Skysphere.Load(f);

		// �e�N�X�`���ǂݍ���
		f = texfilename[2];
		m_Skysphere.LoadTex(f, DIFFUSE);

		// �����_���[�Ƀ��f����n��
		m_MRSkysphere.Init(m_Skysphere);

		Vector3 pos = Vector3(0.0f, 0.0f, 0.0f);
		Vector3 rotate = Vector3(0.0f, 0.0f, 0.0f);
		Vector3 scale = Vector3(1.0f, 1.0f, 1.0f);
		m_MRSkysphere.SetPosition(pos);
		m_MRSkysphere.SetRotation(rotate);
		m_MRSkysphere.SetScale(scale);
	}

	// Cottage�ǂݍ���
	{
		std::string f = filename[0];

		// ���f���ǂݍ���
		m_Cottage.Load(f);

		// �e�N�X�`���ǂݍ���
		f = texfilename[0];
		m_Cottage.LoadTex(f, TexType::DIFFUSE);
		f = texfilename[1];
		m_Cottage.LoadTex(f, TexType::NORMAL);

		// �����_���[�Ƀ��f����n��
		m_MRCottage.Init(m_Cottage, true);

		Vector3 pos = Vector3(0.0f, 0.0f, 0.0f);
		Vector3 rotate = Vector3(0.0f, 45.0f, 0.0f);
		Vector3 scale = Vector3(1.0f, 1.0f, 1.0f);
		m_MRCottage.SetPosition(pos);
		m_MRCottage.SetRotation(rotate);
		m_MRCottage.SetScale(scale);
	}


}
// �V�[���̍X�V
void SceneDemo::SceneUpdate()
{
	// �e��I�u�W�F�N�g�X�V����
	m_Camera.Update();
	m_MRCottage.Update();
	m_Screen.Update();

	// ���C�g�̍X�V����
	UpdateLight();

	// DLSS���[�h�Ŏg�p���Ă�����
	if (g_RenderMode == RenderMode::DLSS &&
		g_AntiAliasingMode == AntiAliasingMode::DLSS)
		UpdateDLSS();	// DLSS�A�b�v�X�P�[�����O�̍X�V����

}
// �V�[���̕`��
void SceneDemo::SceneDraw()
{
	// Application�Ɋi�[����Ă���`��J�E���g�����Z�b�g
	Application::ResetDrawInfo();


	// �����_�����O���[�h�ɉ����ĕ`���ς���
	switch (g_RenderMode)
	{
	case RenderMode::NONE:
		// �ʏ�̃����_�����O�i�t�H���[�h�����_�����O�j
	{
		// �o�b�t�@���\�[�X�̃Z�b�g
		Renderer::SetSamplerState();
		Renderer::ClearDefaultRenderTarget(true);	// �f�t�H���g�����_�[�^�[�Q�b�g���w��
		Renderer::SetViewPort();					// �r���[�|�[�g���w��
		Renderer::SetDepthEnable(true);				// �[�x�X�e���V���X�e�[�g��ON
		Renderer::SetCamera(m_Camera);				// 3D�i�������e�j�J�����Z�b�g
		m_Camera.Draw();							// �o�C���h

		// �I�u�W�F�N�g�̕`��
		m_ShaderSkysphere.SetGPU();					// �V�F�[�_�[���Z�b�g�iskysphere�j
		m_Skysphere.SetTexture();					// �e�N�X�`�����Z�b�g
		m_MRSkysphere.Draw();						// skysphere�̕`��

		m_ShaderPBR.SetGPU();						// �V�F�[�_�[���Z�b�g�iPBR�j
		m_Field.Draw();								// �t�B�[���h�̕`��
		m_Cottage.SetTexture();						// �e�N�X�`���Z�b�g
		m_MRCottage.Draw();							// ���f���̕`��

	}
	break;
	case RenderMode::DEFERRED:
		// �f�B�t�@�[�h�����_�����O
	{

		// �o�b�t�@���\�[�X�̃Z�b�g
		Renderer::SetSamplerState();
		texParam.TextureType = bufferMode;			// �\������e�N�X�`���^�C�v��ݒ�
		Renderer::SetViewPort();					// �r���[�|�[�g���w��
		Renderer::SetDepthEnable(true);				// �[�x�X�e���V���X�e�[�g��ON
		Renderer::SetCamera(m_Camera);				// 3D�i�������e�j�J�����Z�b�g
		m_Camera.Draw();							// �o�C���h

		auto context = Renderer::GetDeviceContext();
		auto defaultRTV = Renderer::GetDefaultRTV();

		//---------------------------------------
		// �f�B�t�@�[�h�����_�����O
		//---------------------------------------
		SetDeferredGBufferRenderTarget();			// �f�B�t�@�[�h�����_�����O�p�����_�[�^�[�Q�b�g���w��

		// �I�u�W�F�N�g�̕`��
		m_ShaderGBuffer.SetGPU();					// �V�F�[�_�[���Z�b�g�iGBuffer�j

		m_Skysphere.SetTexture();					// �e�N�X�`�����Z�b�g
		m_MRSkysphere.Draw();						// skysphere�̕`��

		texParam.UseNormalMap = Enable::ENABLE;		// �@���}�b�v�g�p
		Renderer::SetTextureParam(texParam);		// �o�C���h
		m_Field.Draw();								// �t�B�[���h�̕`��
		m_Cottage.SetTexture();						// �e�N�X�`�����Z�b�g
		m_MRCottage.Draw();							// ���f���̕`��


		//---------------------------------------
		// �f�B�t�@�[�h�V�F�[�f�B���O
		//---------------------------------------
		
		Renderer::ClearDefaultRenderTarget(false);	// �f�t�H���g�̃����_�[�^�[�Q�b�g���Z�b�g�i�[�x�o�b�t�@�Ȃ��j
		SetDeferredShaderResource();				// GBuffer�̏����e�N�X�`���Ƃ��ăo�C���h
		m_ShaderDeferredRendering.SetGPU();			// �V�F�[�_�[�̃Z�b�g�i�f�B�t�@�[�h�����_�����O�j
		m_Screen.Draw();							// �t���X�N���[���N�A�b�h�̕`��


		//========================================================
		// ���_�P
		// �f�B�t�@�[�h�����_�����O�ƃt�H���[�h�����_�����O��
		// ������p�����`�悾�Ə�肭�`�悳��Ȃ�
		// ���[�x�o�b�t�@�ɖ�肪����H
		//========================================================


		//���������I�u�W�F�N�g��قȂ郉�C�e�B���O���s�����f���̂��߁�
		//---------------------------------------
		// �t�H���[�h�����_�����O
		//---------------------------------------
		
		// �t�H���[�h�����_�����O�͐[�x�o�b�t�@��L����
		//context->OMSetRenderTargets(1, &defaultRTV, m_DepthStencilView);

		// �[�x�o�b�t�@�͂��̂܂܎g�p���� (�K�v�Ȃ�N���A)
		// context->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

		//// �I�u�W�F�N�g�̕`��
		//m_ShaderSkysphere.SetGPU();				// �V�F�[�_�[���Z�b�g�iskysphere�j
		//m_Skysphere.SetTexture();					// �e�N�X�`�����Z�b�g
		//m_MRSkysphere.Draw();						// skysphere�̕`��



	}
		break;
	case RenderMode::DLSS:
		// DLSS�A�b�v�X�P�[��
		{

		Renderer::SetViewPort(Renderer::GetInputRenderSize());	// �r���[�|�[�g���w��
		Renderer::SetDepthEnable(true);							// �[�x�X�e���V���X�e�[�g��ON
		Renderer::SetCamera(m_Camera);							// 3D�i�������e�j�J�����Z�b�g
		m_Camera.Draw();										// �o�C���h

		//----------------------------------------------
		// �f�B�t�@�[�h�����_�����O
		//----------------------------------------------
		SetDLSSRenderTarget();						// DLSS�p�����_�[�^�[�Q�b�g���w��

		// GBuffer�p�V�F�[�_�[���Z�b�g
		m_ShaderGBuffer.SetGPU();

		// �I�u�W�F�N�g�̕`��
		texParam.UseNormalMap = Enable::DISABLE;	// �@���}�b�v���g�p
		Renderer::SetTextureParam(texParam);		// �o�C���h
		m_Skysphere.SetTexture();					// �e�N�X�`�����Z�b�g
		m_MRSkysphere.Draw();						// skysphere�̕`��

		texParam.UseNormalMap = Enable::ENABLE;		// �@���}�b�v�g�p
		Renderer::SetTextureParam(texParam);		// �o�C���h
		m_Field.Draw();								// �t�B�[���h�̕`��
		m_Cottage.SetTexture();						// �e�N�X�`�����Z�b�g
		m_MRCottage.Draw();							// ���f���̕`��


		//-----------------------------------------------------------------------------
		// �I�t�X�N���[�������_�����O�iDLSS�ɓn�����͒l���擾�j
		// ���I�t�X�N���[���̃����_�[�^�[�Q�b�g��SetDLSSRenderTarget()�Ŏw�肵�Ă���
		//-----------------------------------------------------------------------------
		SetOffScreenRenderTarget();		// �I�t�X�N���[���p�̃����_�[�^�[�Q�b�g���Z�b�g

		SetDLSSShaderResource();		// G�o�b�t�@���o�C���h
		m_ShaderDLSSInput.SetGPU();		// �I�t�X�N���[�������_�����O�p�V�F�[�_�[���Z�b�g
		m_Screen.Draw();				// �I�t�X�N���[�������_�����O


		//------------------------------------------------
		// �����_�����O���ʂ��A�b�v�X�P�[�����O�iDLSS�j
		//------------------------------------------------

		// G�o�b�t�@���瓾��ꂽ���\�[�X���擾
		ID3D11Texture2D* PreResolveColor = m_OffScreenTex;
		ID3D11Texture2D* ResolvedColor = m_DLSSOutputTex;
		ID3D11Texture2D* MotionVectors = m_DLSSGBufferTexs[4];
		ID3D11Texture2D* Depth = m_DLSSDepthTex;
		ID3D11Texture2D* Exposure = nullptr;

		// �A�b�v�X�P�[���̐����t���O
		bool isEvaluate = false;

		// DLSS�X�[�p�[�T���v�����O
		// �R�R��DLSS�T���v�����O���s��
		if (g_AntiAliasingMode == AntiAliasingMode::DLSS)
		{
			// DLSS�@�\������������Ă���
			// DLSS���g�p����t���O���o���Ă���ꍇ
			if (isInitDLSS && isUseDLSSFeature)
			{
				bool ResetScene = isResetResource;


				//==============================================================================
				// ���_�Q
				// DLAA�ȊO�̃A�b�v�X�P�[���̏������s���Ȃ�
				// ��DLAA���������Ă��邱�Ƃ���n���Ă��郊�\�[�X�Ɉȏ�͂Ȃ����Ƃ��킩��
				// �������_�����O�T�C�Y�������H���邢�͑���Ȃ����\�[�X������H
				//==============================================================================

				// DLSS�X�[�p�[�T���v�����O
				if (DLSSManager::GetInstance().EvaluateSuperSampling(
					PreResolveColor,		// ����
					ResolvedColor,			// �o��
					MotionVectors,			// ���[�V�����x�N�g��
					Depth,					// �[�x
					Exposure,				// �I�o
					ResetScene,				// ���Z�b�g�t���O
					true,					// �g��API�t���O
					{ 0.0f, 0.0f },			// �W�b�^�[�I�t�Z�b�g
					{ 1.0f, 1.0f }))		// ���[�V�����x�N�g���X�P�[��
				{
					std::cout << "�A�b�v�X�P�[���ɐ������܂���" << std::endl;
					isEvaluate = true;
				}
				else
				{
					// �X�[�p�[�T���v�����O�Ɏ��s�����ꍇ��
					// ���͑O�̃��\�[�X���g�p����
					std::cerr << "�A�b�v�X�P�[���Ɏ��s���܂���" << std::endl;
					std::cout << "�A�b�v�X�P�[���O�̉摜���g�p���܂�" << std::endl;
					isEvaluate = false;
				}
			}
			else
			{
				// DLSS�@�\�̏�����������Ă��Ȃ�������
				if (!isInitDLSS)
				{
					std::cerr << "DLSS�@�\������������Ă��܂���" << std::endl;
					std::cerr << "�A�b�v�X�P�[���O�̉摜���g�p���܂�" << std::endl;
					isEvaluate = false;
				}
			}
		}

		//---------------------------------
		// �A�b�v�X�P�[�����ʂ�`��
		//---------------------------------
		Renderer::ClearDefaultRenderTarget(true);	// �����_�[�^�[�Q�b�g�����ɖ߂�
		Renderer::SetViewPort();					// �r���[�|�[�g���E�B���h�E�T�C�Y�ɖ߂�
	
		// �X�[�p�[�T���v�����O�̉ۂɉ����ăo�C���h���郊�\�[�X��ύX
		if (isEvaluate) {
			SetDLSSOutputResource();				// �A�E�g�v�b�g���\�[�X
		}
		else {
			SetOffScreenResource();					// �I�t�X�N���[�����\�[�X
		}

		m_ShaderDLSSOutput.SetGPU();				// �o�͗p�V�F�[�_�[���Z�b�g
		m_Screen.Draw();							// ���ʂ��t���X�N���[���N�A�b�h�ɕ`��

		}
		break;
	}

}
// �V�[���̏I��
void SceneDemo::SceneDispose()
{
	// ��� 
	
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

	// DLSS�[�x�e�N�X�`��
	m_DLSSDepthTex->Release();
	m_DLSSDepthTex = nullptr;
	m_DLSSDepthDSV->Release();
	m_DLSSDepthDSV = nullptr;
	m_DLSSDepthSRV->Release();
	m_DLSSDepthSRV = nullptr;

	// DLSS�o�̓e�N�X�`��
	m_DLSSOutputTex->Release();
	m_DLSSOutputTex = nullptr;
	m_DLSSOutputUAV->Release();
	m_DLSSOutputUAV = nullptr;
	m_DLSSOutputSRV->Release();
	m_DLSSOutputSRV = nullptr;

	// DLSS���̓e�N�X�`��
	m_OffScreenTex->Release();
	m_OffScreenTex = nullptr;
	m_OffScreenRTV->Release();
	m_OffScreenRTV = nullptr;
	m_OffScreenSRV->Release();
	m_OffScreenSRV = nullptr;

	// �f�B�t�@�[�h�����_�����O
	m_DepthStencilSRV->Release();
	m_DepthStencilSRV = nullptr;
	m_DepthStencilView->Release();
	m_DepthStencilView = nullptr;
	m_DepthStencilBuffer->Release();
	m_DepthStencilBuffer = nullptr;

}



// DLSS�̍X�V����
void SceneDemo::UpdateDLSS()
{
	//-------------------------------
	// �����_�����O�T�C�Y�֘A
	//-------------------------------

	// �����_�����O�T�C�Y�i�A�b�v�X�P�[���O�j
	XMUINT2 inputRenderTargetSize = Renderer::GetInputRenderSize();
	// �ŏI�o�̓����_�����O�T�C�Y�i�A�b�v�X�P�[����j
	XMUINT2 outputRenderTargetSize = Renderer::GetOutputRenderSize();
	// ���O�i�O�t���[���j�̐��������_�����O�T�C�Y
	static XMUINT2 inputLastSize = { 0, 0 };	// ����
	static XMUINT2 outputLastSize = { 0, 0 };	// �o�� 
	// �T���v���[�X�e�[�g�ɓn��Lod�o�C�A�X
	float lodBias = 0.f;
	// DLSS�쐬���ɐݒ肵�������_�����O�T�C�Y
	XMUINT2 dlssCreationTimeRenderSize = inputRenderTargetSize;

	//-------------------------------
	// �N�I���e�B���[�h�̐ݒ�
	//-------------------------------

	// ���݂̃N�I���e�B���[�h�iUI����ݒ�\�ɂ��Ă���j
	PrevQuality = PerfQualityMode;
	PerfQualityMode = PERF_QUALITY_LIST[perfQualityIndex].PerfQuality;

	//------------------------------------------------
	// DLSS�̐����T�C�Y�ݒ�i�����_�����O�T�C�Y�j�̃N�G��
	// �ŏI�����_�����O�T�C�Y�ioutput�j����
	// ���������_�����O�T�C�Y�iinput�j�����߂�
	//------------------------------------------------

	// �A���`�G�C���A�V���O���[�h��DLSS�̏ꍇ
	if (g_AntiAliasingMode == AntiAliasingMode::DLSS)
	{
		// �ŏI�o�̓����_�����O�T�C�Y���O�t���[������X�V����Ă��Ȃ��ꍇ��
		// �œK�����_�����O�T�C�Y�̎擾�N�G���͏ȗ��o����
		if (outputLastSize.x != outputRenderTargetSize.x ||
			outputLastSize.y != outputRenderTargetSize.y)
		{
			// �e�p�t�H�[�}���X���[�h�ɂ��čœK�ݒ�̃N�G��
			for (PERF_QUALITY_ITEM& item : PERF_QUALITY_LIST)
			{
				// ���������_�����O�T�C�Y�̃N�G��
				DLSSManager::GetInstance().QueryOptimalSettings(
					outputRenderTargetSize,
					item.PerfQuality,
					&g_RecommendedSettingsMap[item.PerfQuality]);

				// �����ݒ肪�L�����m�F
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

				// �����T�C�Y�̃f�o�b�O�\��
				std::cout << "Recommended Optimal Render Size: ("
					<< g_RecommendedSettingsMap[item.PerfQuality].m_ngxRecommendedOptimalRenderSize.x << ", "
					<< g_RecommendedSettingsMap[item.PerfQuality].m_ngxRecommendedOptimalRenderSize.y << ")" << std::endl;
				std::cout << "Dynamic Maximum Render Size: ("
					<< g_RecommendedSettingsMap[item.PerfQuality].m_ngxDynamicMaximumRenderSize.x << ", "
					<< g_RecommendedSettingsMap[item.PerfQuality].m_ngxDynamicMaximumRenderSize.y << ")" << std::endl;
				std::cout << "Dynamic Minimum Render Size: ("
					<< g_RecommendedSettingsMap[item.PerfQuality].m_ngxDynamicMinimumRenderSize.x << ", "
					<< g_RecommendedSettingsMap[item.PerfQuality].m_ngxDynamicMinimumRenderSize.y << ")" << std::endl;

				// ���I�ݒ肪�\���m�F
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

			// �O�t���[���̃����_�����O�T�C�Y��
			// ���݂�DLSS�ŏI�����_�����O�T�C�Y��ۑ����Ă���
			outputLastSize = outputRenderTargetSize;

			std::cout << "DLSS�œK�ݒ芮��" << endl;
		}
	}


	//--------------------------------------------------------------------
	// �����_�����O�T�C�Y���O�t���[���Ɣ�r���ĕύX����Ă����ꍇ�̏���
	// �i��Ƀp�t�H�[�}���X���[�h�Ɉˑ����ăT�C�Y���ς��j
	//--------------------------------------------------------------------

	// �e�N�X�`����LOD�iLevel of Detail�j��X�������̃T�C�Y��������
	float texLodXDimension = { 0 };

	// ��������Ă���œK�ȃ����_�����O�T�C�Y��DLSS���������̃����_�����O�T�C�Y�ɐݒ�
	dlssCreationTimeRenderSize = g_RecommendedSettingsMap[PerfQualityMode].m_ngxRecommendedOptimalRenderSize;

	// DLSS�쐬���̃����_�����O�T�C�Y���A�b�v�X�P�[���O�̃����_�����O�T�C�Y�ɐݒ�
	inputRenderTargetSize = dlssCreationTimeRenderSize;

	// Lod�o�C�A�X�������_�����O�T�C�Y����v�Z
	texLodXDimension = inputRenderTargetSize.x;
	float ratio = (float)inputRenderTargetSize.x / (float)outputRenderTargetSize.x;
	lodBias = (std::log2f(ratio)) - 1.0f;

	// ���߂�Lod�o�C�A�X�ŃT���v���[���Z�b�g
	Renderer::SetSamplerState(lodBias);

	//-------------------------------
	// DLSS�̏�����
	//-------------------------------
	// �����_�����O�T�C�Y���O�t���[���ƈقȂ�ꍇ
	if (inputRenderTargetSize.x != inputLastSize.x ||
		inputRenderTargetSize.y != inputLastSize.y)
	{

		std::cout << "Lod Bias: " << lodBias << std::endl;

		// HDR��[�x���]�̐ݒ�i�K�v�ɉ����ĕύX�j
		bool isHDR = false;
		bool depthInverted = false;

		// DLSS�I�v�V�����i��: Quality�ݒ�j
		NVSDK_NGX_PerfQuality_Value qualitySetting = PerfQualityMode;

		// �����_�[�v���Z�b�g�i0�̓f�t�H���g�j
		unsigned int renderPreset = 0;

		// �����_�����O�T�C�Y�ɉ����ă��\�[�X���Đ���
		Renderer::SetInputRenderSize(inputRenderTargetSize);
		CreateDLSSResource(inputRenderTargetSize);


		// NVIDIA GPU���g�p�\�ȏꍇ
		if (Renderer::GetIsAbleNVIDIA())
		{
			isInitDLSS = true;

			// DLSS������
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
				std::cerr << "DLSS�̏������Ɏ��s���܂���" << std::endl;
				isInitDLSS = false;
			}

		}
		else
		{
			std::cerr << "NVIDIA_GPU���g�p���Ă��܂���" << std::endl;
		}

	}

	// �O�t���[���Ɍ��݂̃����_�����O�T�C�Y��ۑ�
	inputLastSize = inputRenderTargetSize;
}
// ���C�g�̍X�V����
void SceneDemo::UpdateLight()
{
	// ���C�g�����̌v�Z�i���_�𒆐S�ɉ�]�j
	pointLight.Direction.x = cosf(radY) * sinf(radXZ);
	pointLight.Direction.y = -sinf(radY);
	pointLight.Direction.z = cosf(radY) * cosf(radXZ);
	pointLight.Direction.Normalize();

	// ���C�g�����̌v�Z�i��]�ɉ����Ĉʒu���X�V�j
	pointLight.Position.x = lightLength * cosf(radY) * sinf(radXZ);
	pointLight.Position.y = lightLength * sinf(radY);
	pointLight.Position.z = lightLength * cosf(radY) * cosf(radXZ);

	// ���C�g�𔽉f
	Renderer::SetLight(pointLight);
}



// �����_�����O���\�[�X�̐���
void SceneDemo::CreateRenderResource()
{
	CreateDeferredResource();	// �f�B�t�@�[�h�����_�����O
	CreateDLSSResource();		// DLSS
}



// �f�B�t�@�[�h�����_�����O���\�[�X�̐���
void SceneDemo::CreateDeferredResource()
{
	auto device = Renderer::GetDevice();
	auto iRenderSize = Renderer::GetInputRenderSize();

	// �eG�o�b�t�@�̃t�H�[�}�b�g
	DXGI_FORMAT formats[GBUFFER_NUM] = {
		DXGI_FORMAT_R8G8B8A8_UNORM,		// Albedo
		DXGI_FORMAT_R16G16B16A16_FLOAT, // Normal
		DXGI_FORMAT_R32G32B32A32_FLOAT, // WorldPos
		DXGI_FORMAT_R32G32B32A32_FLOAT,	// Depth�i�_�~�[�j
		DXGI_FORMAT_R32G32_FLOAT,		// MVector
	};

	// G�o�b�t�@�̐���
	for (int i = 0; i < GBUFFER_NUM; ++i) {
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = iRenderSize.x;
		textureDesc.Height = iRenderSize.y;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.SampleDesc.Count = 1;	// �}���`�T���v���͖���
		textureDesc.Format = formats[i];
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		// �e�N�X�`���쐬
		auto hr = device->CreateTexture2D(&textureDesc, nullptr, &m_DeferredGBufferTexs[i]);
		if (FAILED(hr)) { cout << "G�o�b�t�@�e�N�X�`���̍쐬�Ɏ��s���܂���" << endl; }
		// RTV�쐬
		hr = device->CreateRenderTargetView(m_DeferredGBufferTexs[i], nullptr, &m_DeferredGBufferRTVs[i]);
		if (FAILED(hr)) { cout << "G�o�b�t�@RTV�̍쐬�Ɏ��s���܂���" << endl; }
		// SRV�쐬
		hr = device->CreateShaderResourceView(m_DeferredGBufferTexs[i], nullptr, &m_DeferredGBufferSRVs[i]);
		if (FAILED(hr)) { cout << "G�o�b�t�@SRV�̍쐬�Ɏ��s���܂���" << endl; }
	}

	// �[�x�o�b�t�@�쐬
	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = iRenderSize.x;
	depthDesc.Height = iRenderSize.y;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_R24G8_TYPELESS; // �[�x + �X�e���V��
	depthDesc.SampleDesc.Count = 1;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	// �e�N�X�`���쐬
	auto hr = device->CreateTexture2D(&depthDesc, nullptr, &m_DepthStencilBuffer);
	if (FAILED(hr)) { cout << "�[�x�X�e���V���e�N�X�`���̍쐬�Ɏ��s���܂���" << endl; }
	// DSV �쐬
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	hr = device->CreateDepthStencilView(m_DepthStencilBuffer, &dsvDesc, &m_DepthStencilView);
	if (FAILED(hr)) { cout << "�[�x�X�e���V���r���[�̍쐬�Ɏ��s���܂���" << endl; }
	// SRV �쐬
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; // �[�x�l�݂̂�SRV�Ƃ��Ďg�p
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(m_DepthStencilBuffer, &srvDesc, &m_DepthStencilSRV);
	if (FAILED(hr)) { cout << "�[�x�X�e���V��SRV�̍쐬�Ɏ��s���܂���" << endl; }

}
// DLSS���\�[�X�̐����i�y�эĐ����j
void SceneDemo::CreateDLSSResource(XMUINT2 renderSize)
{
	auto device = Renderer::GetDevice();

	// �������\�[�X�̉��
	// G�o�b�t�@�̉��
	for (int i = 0; i < GBUFFER_NUM; ++i) {
		if (m_DLSSGBufferRTVs[i]) m_DLSSGBufferRTVs[i]->Release();
		if (m_DLSSGBufferSRVs[i]) m_DLSSGBufferSRVs[i]->Release();
		if (m_DLSSGBufferTexs[i]) m_DLSSGBufferTexs[i]->Release();
	}
	// DLSS���̓o�b�t�@�̉��
	if (m_OffScreenRTV) m_OffScreenRTV->Release();
	if (m_OffScreenSRV) m_OffScreenSRV->Release();
	if (m_OffScreenTex) m_OffScreenTex->Release();
	// DLSS�o�̓o�b�t�@�̉��
	if (m_DLSSOutputUAV) m_DLSSOutputUAV->Release();
	if (m_DLSSOutputSRV) m_DLSSOutputSRV->Release();
	if (m_DLSSOutputTex) m_DLSSOutputTex->Release();
	// �[�x�o�b�t�@�̉��
	if (m_DLSSDepthDSV) m_DLSSDepthDSV->Release();
	if (m_DLSSDepthSRV) m_DLSSDepthSRV->Release();
	if (m_DLSSDepthTex) m_DLSSDepthTex->Release();


	D3D11_TEXTURE2D_DESC textureDesc = {};
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

	// �eG�o�b�t�@�̃t�H�[�}�b�g
	DXGI_FORMAT formats[GBUFFER_NUM] = {
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,// Albedo�i����`��ԁj
		DXGI_FORMAT_R16G16B16A16_FLOAT, // Normal
		DXGI_FORMAT_R32G32B32A32_FLOAT, // WorldPos
		DXGI_FORMAT_R32_FLOAT,			// Depth
		DXGI_FORMAT_R32G32_FLOAT,		// MVector
	};

	//------------------------------------------------
	// G�o�b�t�@�̐���
	//------------------------------------------------
	for (int i = 0; i < GBUFFER_NUM; ++i) {

		textureDesc.Width = renderSize.x;
		textureDesc.Height = renderSize.y;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.SampleDesc.Count = 1;	// �}���`�T���v���͖���
		textureDesc.Format = formats[i];
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		// �e�N�X�`��
		device->CreateTexture2D(&textureDesc, nullptr, &m_DLSSGBufferTexs[i]);
		// RTV
		device->CreateRenderTargetView(m_DLSSGBufferTexs[i], nullptr, &m_DLSSGBufferRTVs[i]);
		// SRV
		device->CreateShaderResourceView(m_DLSSGBufferTexs[i], nullptr, &m_DLSSGBufferSRVs[i]);

	}

	//------------------------------------------------
	// �[�x�o�b�t�@�̐���
	//------------------------------------------------
	textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	textureDesc.Width = renderSize.x;
	textureDesc.Height = renderSize.y;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE; // SRV���o�C���h�\��
	// �e�N�X�`��
	device->CreateTexture2D(&textureDesc, nullptr, &m_DLSSDepthTex);
	// DSV
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView(m_DLSSDepthTex, &dsvDesc, &m_DLSSDepthDSV);
	// SRV
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; // �V�F�[�_�[�Ő[�x�݂̂��g�p
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(m_DLSSDepthTex, &srvDesc, &m_DLSSDepthSRV);

	//------------------------------------------------
	// ���͐�o�b�t�@�̐���
	//------------------------------------------------
	textureDesc.Width = renderSize.x;
	textureDesc.Height = renderSize.y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;	// �}���`�T���v���͖���
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	// �e�N�X�`��
	device->CreateTexture2D(&textureDesc, nullptr, &m_OffScreenTex);
	// RTV
	device->CreateRenderTargetView(m_OffScreenTex, nullptr, &m_OffScreenRTV);
	// SRV
	device->CreateShaderResourceView(m_OffScreenTex, nullptr, &m_OffScreenSRV);


	//------------------------------------------------
	// �o�͐�o�b�t�@�̐���
	//------------------------------------------------
	textureDesc.Width = renderSize.x;
	textureDesc.Height = renderSize.y;
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT,	 // �o�̓t�H�[�}�b�g
	textureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	// �e�N�X�`��
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



// �����_�[�^�[�Q�b�g���Z�b�g�i�f�B�t�@�[�h�����_�����O�j
void SceneDemo::SetDeferredGBufferRenderTarget()
{
	auto context = Renderer::GetDeviceContext();

	// G�o�b�t�@�������_�[�^�[�Q�b�g�Ƃ��Đݒ�
	ID3D11RenderTargetView* rtv[] = {
		m_DeferredGBufferRTVs[0], // Albedo
		m_DeferredGBufferRTVs[1], // Normal
		m_DeferredGBufferRTVs[2], // WorldPos
		m_DeferredGBufferRTVs[3], // Depth
		m_DeferredGBufferRTVs[4], // MVector
	};
	
	context->OMSetRenderTargets(GBUFFER_NUM, rtv, m_DepthStencilView);

	// G�o�b�t�@���N���A
	float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	for (int i = 0; i < GBUFFER_NUM; ++i) {
		context->ClearRenderTargetView(m_DeferredGBufferRTVs[i], clearColor);
	}
	context->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

}
// �f�B�t�@�[�h�����_�����O���\�[�X���Z�b�g
void SceneDemo::SetDeferredShaderResource()
{
	auto context = Renderer::GetDeviceContext();

	// SRV���s�N�Z���V�F�[�_�[�Ƀo�C���h
	context->PSSetShaderResources(0, GBUFFER_NUM, m_DeferredGBufferSRVs);
	// DSV�ŏ������񂾐[�x�l���o�C���h
	context->PSSetShaderResources(3, 1, &m_DepthStencilSRV);
}



// DLSS�p�̃����_�[�^�[�Q�b�g���w��
void SceneDemo::SetDLSSRenderTarget()
{
	auto context = Renderer::GetDeviceContext();

	// G�o�b�t�@�������_�[�^�[�Q�b�g�Ƃ��Đݒ�
	ID3D11RenderTargetView* rtv[] = {
		m_DLSSGBufferRTVs[0],	// Albedo
		m_DLSSGBufferRTVs[1],	// Normal
		m_DLSSGBufferRTVs[2],	// WorldPos
		m_DLSSGBufferRTVs[3],	// Depth�i�_�~�[�j
		m_DLSSGBufferRTVs[4],	// MVector
		m_OffScreenRTV,			// �I�t�X�N���[��
	};;

	// �r���[�̐�
	unsigned int viewNum = GBUFFER_NUM + 1;

	context->OMSetRenderTargets(viewNum, rtv, m_DLSSDepthDSV);

	// G�o�b�t�@���N���A
	float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	for (int i = 0; i < viewNum; ++i) {
		context->ClearRenderTargetView(rtv[i], clearColor);
	}
	context->ClearDepthStencilView(m_DLSSDepthDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}
// DLSS�p�̃V�F�[�_�[���\�[�X���w��
void SceneDemo::SetDLSSShaderResource()
{
	auto context = Renderer::GetDeviceContext();

	// SRV���s�N�Z���V�F�[�_�[�Ƀo�C���h
	context->PSSetShaderResources(0, GBUFFER_NUM, m_DLSSGBufferSRVs);
	// DSV�ŏ������񂾐[�x�l���o�C���h
	context->PSSetShaderResources(3, 1, &m_DLSSDepthSRV);
}
// �I�t�X�N���[����̃����_�[�^�[�Q�b�g���w��
void SceneDemo::SetOffScreenRenderTarget()
{
	auto context = Renderer::GetDeviceContext();


	// �I�t�X�N���[�������_�[�^�[�Q�b�g���Z�b�g
	context->OMSetRenderTargets(1, &m_OffScreenRTV, nullptr);

	// �N���A
	float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	context->ClearRenderTargetView(m_OffScreenRTV, clearColor);
}
// �I�t�X�N���[���̃V�F�[�_�[���\�[�X���o�C���h
void SceneDemo::SetOffScreenResource()
{
	auto context = Renderer::GetDeviceContext();

	// SRV���s�N�Z���V�F�[�_�[�Ƀo�C���h
	context->PSSetShaderResources(0, 1, &m_OffScreenSRV);

}
// �A�b�v�X�P�[����̃��\�[�X���o�C���h
void SceneDemo::SetDLSSOutputResource()
{
	auto context = Renderer::GetDeviceContext();

	// SRV���s�N�Z���V�F�[�_�[�Ƀo�C���h
	context->PSSetShaderResources(0, 1, &m_DLSSOutputSRV);
}



