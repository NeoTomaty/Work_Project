#include "DLSSManager.h"
#include <assert.h>
#include <iostream>  // �f�o�b�O�p
#include <memory>
#include "Renderer.h"
#include "Application.h"


// �A�v���P�[�V����ID
#define APP_ID (231313132)

using namespace DirectX;

// NGX�̏�����
// --------------------------------------------------
// ����
// _log:���O�\�����s���t�@�C���̃p�X�i�K�{�ł͂Ȃ��j
// --------------------------------------------------
bool DLSSManager::InitializeNGX(const wchar_t* _log)
{

    // DLL�t�@�C���̃p�X���w��
    HMODULE hDLL = LoadLibrary("nvngx_dlss.dll");
    if (!hDLL) {
        std::cerr << "DLL�̃��[�h�Ɏ��s���܂����B" << std::endl;
        return -1;
    }
    std::cout << "DLL������Ƀ��[�h����܂����B" << std::endl;

    // �f�o�C�X���擾
    ID3D11Device* device = Renderer::GetDevice();

    // �f�o�C�X���Ȃ�������
    if (device == nullptr)
    {
        std::cerr << "�f�o�C�X������܂���" << std::endl;
        return false;
    }

    // NGX��������
    auto result = NVSDK_NGX_D3D11_Init(APP_ID, _log, device);

    // NGX�������̌��ʂ��i�[
    m_ngxInitialized = !NVSDK_NGX_FAILED(result);

    // �������Ԃ���Ȃ�������
    if (!m_ngxInitialized){
        // �������̎��s
        if (result == NVSDK_NGX_Result_FAIL_FeatureNotSupported || result == NVSDK_NGX_Result_FAIL_PlatformError)
            std::wcerr << L"NVIDIA NGX���g�p�ł��܂��� code = 0x"
            << std::hex << result
            << L", info: " << GetNGXResultAsString(result)
            << std::endl;
        else
            std::wcerr << L"NVIDIA NGX�̏������Ɏ��s���܂��� error code = 0x"
            << std::hex << result
            << L", info: " << GetNGXResultAsString(result)
            << std::endl;

        return false;
    }

    // NGX�@�\�̃p�����[�^���擾
    result = NVSDK_NGX_D3D11_GetCapabilityParameters(&m_ngxParameters);
    // �������Ԃ���Ȃ�������
    if (result != NVSDK_NGX_Result_Success)
    {
        // �p�����[�^�̎擾���s
        std::cerr << "�@�\�̃p�����[�^�擾�Ɏ��s���܂���" << std::endl;
        // �V���b�g�_�E��
        ShutdownNGX();
        return false;
    }

    // NVIDIA�h���C�o�̃o�[�V�����m�F
#if defined(NVSDK_NGX_Parameter_SuperSampling_NeedsUpdatedDriver)        \
    && defined (NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMajor) \
    && defined (NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMinor)

    int needsUpdatedDriver = 0;
    unsigned int minDriverVersionMajor = 0;
    unsigned int minDriverVersionMinor = 0;
    NVSDK_NGX_Result ResultUpdatedDriver = m_ngxParameters->Get(NVSDK_NGX_Parameter_SuperSampling_NeedsUpdatedDriver, &needsUpdatedDriver);
    NVSDK_NGX_Result ResultMinDriverVersionMajor = m_ngxParameters->Get(NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMajor, &minDriverVersionMajor);
    NVSDK_NGX_Result ResultMinDriverVersionMinor = m_ngxParameters->Get(NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMinor, &minDriverVersionMinor);
    if (ResultUpdatedDriver == NVSDK_NGX_Result_Success &&
        ResultMinDriverVersionMajor == NVSDK_NGX_Result_Success &&
        ResultMinDriverVersionMinor == NVSDK_NGX_Result_Success)
    {
        if (needsUpdatedDriver)
        {
            std::cerr << "�G���[: NVIDIA DLSS�����[�h�ł��܂���B�h���C�o���Â����߂ł��B�K�v�ȍŒ�h���C�o�o�[�W����: "
                << minDriverVersionMajor << "." << minDriverVersionMinor << std::endl;
            std::cout << "�@�\�̃p�����[�^�擾�Ɏ��s���܂���" << std::endl;
            ShutdownNGX();
            return false;
        }
        else
        {
            std::cout << "NVIDIA DLSS�̍Œ�h���C�o�o�[�W����: "
                << minDriverVersionMajor << "." << minDriverVersionMinor << std::endl;
        }
    }
    else
    {
        std::cout << "NVIDIA DLSS�̍Œ�h���C�o�o�[�W�����͕񍐂���܂���ł����B" << std::endl;
    }
#endif

    // DLSS���g�p�\�Ȃ̂����m�F����
    int dlssAvailable = 0;
    result = m_ngxParameters->Get(NVSDK_NGX_Parameter_SuperSampling_Available, &dlssAvailable);
    // �p�����[�^�̎擾�Ɏ��s�܂��͗��p�ł��Ȃ��n�[�h�E�F�A�������ꍇ
    if (result != NVSDK_NGX_Result_Success || !dlssAvailable)
    {
        std::wcout << L"NVIDIA DLSS ���g�p�ł��܂��� result = 0x"
            << std::hex << result
            << L", info: " << GetNGXResultAsString(result)
            << std::endl;
        // �V���b�g�_�E��
        ShutdownNGX();
        return false;
    }

    // DLSS���g�p�\�Ȃ̂Ńt���O��ύX
    m_dlssAvailable = true;

    // ����������
    std::cout << "NGX SDK�̏������ɐ������܂���" << std::endl;


    return true;
}

// NGX�y��DLSS�̋@�\�̉��
void DLSSManager::ShutdownNGX()
{
    // �f�o�C�X���擾
    ID3D11Device* device = Renderer::GetDevice();

    if (device == nullptr)
    {
        std::cerr << "�f�o�C�X������܂���" << std::endl;
        return;
    }

    // NGX������������Ă�����
    if (m_ngxInitialized)
    {
        // �S�Ă�GPU��������������܂őҋ@
        Wait();

        // DLSS�@�\����łȂ������ꍇ
        if (m_dlssFeature != nullptr)
        {
            std::cerr << "�@�\�������[�X�����O��NGX���C�u������������悤�Ƃ��Ă��܂� ���݉�����Ă��܂����R�[�h���m�F����K�v������܂�" << std::endl;
            ReleaseDLSSFeatures();
        }


        NVSDK_NGX_D3D11_DestroyParameters(m_ngxParameters);
        NVSDK_NGX_D3D11_Shutdown1(nullptr);

        m_ngxInitialized = false;
    }
}


// DLSS�@�\�̏�����
// ---------------------------------------------------------------------
// ����
// OptimalRenderSize:���͉摜�̉𑜓x
// OutputRenderSize:�o�͉摜�̉𑜓x
// isHDR:HDR�o�͂��ǂ���
// depthInverted:�[�x�l�̔��]�̗L��
// depthScale:�[�x�l�i��{�I�ɂ�1.0f���Z�b�g�j
// enableSharpening:�V���[�v�j���O�i�摜�̑N�s���j��L���ɂ��邩�ǂ���
// enableAutoExposure:�����I�o�␳��L���ɂ��邩�ǂ���
// qualValue:DLSS�̃p�t�H�[�}���X�i���������p�����[�^
// renderPreset:�����_�����O�̃v���Z�b�g�ݒ�
// ---------------------------------------------------------------------
bool DLSSManager::InitializeDLSSFeatures(
    XMUINT2 OptimalRenderSize,
    XMUINT2 OutputRenderSize,
    int isHDR,
    bool depthInverted,
    bool enableSharpening,
    bool enableAutoExposure,
    NVSDK_NGX_PerfQuality_Value qualValue,
    unsigned int renderPreset)
{
    // �f�o�C�X���擾
    ID3D11Device* device = Renderer::GetDevice();
    // �R���e�L�X�g���擾
    ID3D11DeviceContext* context = Renderer::GetDeviceContext();


    // NGX������������Ă��Ȃ�������
    if (!m_ngxInitialized)
    {
        std::cerr << "NGX������������Ă��Ȃ���Ԃł�DLSS�̏������ɂȂ�܂�" << std::endl;
        return false;
    }

    NVSDK_NGX_Result ResultDLSS = NVSDK_NGX_Result_Fail;

    int MotionVectorResolutionLow = 1; // �X�j�y�b�g�i�R�[�h�Ёj�Ƀ��[�V�����x�N�g���̃A�b�v�T���v�����O��C���܂�
    // �@�\�̍쐬	
    int DlssCreateFeatureFlags = NVSDK_NGX_DLSS_Feature_Flags_None;
    DlssCreateFeatureFlags |= MotionVectorResolutionLow ? NVSDK_NGX_DLSS_Feature_Flags_MVLowRes : 0;
    DlssCreateFeatureFlags |= isHDR ? NVSDK_NGX_DLSS_Feature_Flags_IsHDR : 0;
    DlssCreateFeatureFlags |= depthInverted ? NVSDK_NGX_DLSS_Feature_Flags_DepthInverted : 0;
    DlssCreateFeatureFlags |= enableSharpening ? NVSDK_NGX_DLSS_Feature_Flags_DoSharpening : 0;
    DlssCreateFeatureFlags |= enableAutoExposure ? NVSDK_NGX_DLSS_Feature_Flags_AutoExposure : 0;

    NVSDK_NGX_DLSS_Create_Params DlssCreateParams;
    memset(&DlssCreateParams, 0, sizeof(DlssCreateParams));

    DlssCreateParams.Feature.InWidth = OptimalRenderSize.x;
    DlssCreateParams.Feature.InHeight = OptimalRenderSize.y;
    DlssCreateParams.Feature.InTargetWidth = OutputRenderSize.x;
    DlssCreateParams.Feature.InTargetHeight = OutputRenderSize.y;
    DlssCreateParams.Feature.InPerfQualityValue = qualValue;
    DlssCreateParams.InFeatureCreateFlags = DlssCreateFeatureFlags;

    //DlssCreateParams.Feature.InWidth = 1920;
    //DlssCreateParams.Feature.InHeight = 1080;
    //DlssCreateParams.Feature.InTargetWidth = 3840;
    //DlssCreateParams.Feature.InTargetHeight = 2160;
    //DlssCreateParams.Feature.InPerfQualityValue = NVSDK_NGX_PerfQuality_Value_MaxPerf;


    // �����_�[�v���Z�b�g��I��
    NVSDK_NGX_Parameter_SetUI(m_ngxParameters, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_DLAA, renderPreset);
    NVSDK_NGX_Parameter_SetUI(m_ngxParameters, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Quality, renderPreset);
    NVSDK_NGX_Parameter_SetUI(m_ngxParameters, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Balanced, renderPreset);
    NVSDK_NGX_Parameter_SetUI(m_ngxParameters, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Performance, renderPreset);
    NVSDK_NGX_Parameter_SetUI(m_ngxParameters, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraPerformance, renderPreset);

    // �@�\�̍쐬
    ResultDLSS = NGX_D3D11_CREATE_DLSS_EXT(
        context, 
        &m_dlssFeature, 
        m_ngxParameters, 
        &DlssCreateParams);

    // �쐬�Ɏ��s�����Ƃ�
    if (NVSDK_NGX_FAILED(ResultDLSS))
    {
        std::wcerr << L"Failed to create DLSS Features = 0x"
            << std::hex << ResultDLSS
            << L", info: " << GetNGXResultAsString(ResultDLSS)
            << std::endl;
        return false;
    }

    // �����������t���O�𗧂Ă�
    m_dlssAvailable = true;

    // �������������O
    std::cout << "DLSS�̏������ɐ������܂���" << std::endl;

    return m_dlssAvailable;
}

// DLSS�@�\�̉��
void DLSSManager::ReleaseDLSSFeatures()
{
    ID3D11Device* device = Renderer::GetDevice();

    if (device == nullptr)
    {
        std::cerr << "�f�o�C�X������܂���" << std::endl;
        return;
    }

    if (!m_ngxInitialized)
    {
        std::cerr << "NGX�̏�����������Ă��Ȃ���Ԃł̉���ɂȂ�܂�" << std::endl;
        return;
    }

    // GPU�̑S�Ẵ��\�[�X����������܂őҋ@
    Wait();

    // DLSS�@�\�̉��
    auto result = (m_dlssFeature != nullptr) ? NVSDK_NGX_D3D11_ReleaseFeature(m_dlssFeature) : NVSDK_NGX_Result_Success;
    if (NVSDK_NGX_FAILED(result))
    {
        std::cerr << "DLSS�@�\�̉���Ɏ��s���܂���" << std::endl;
    }

    // �������
    m_dlssFeature = nullptr;

}

// DLSS�@�\�̍œK�ȃ����_�����O�ݒ�
// --------------------------------------------------------
// ����
// renderSize:�����_�����O�T�C�Y
// inQualValue:DLSS�̃p�t�H�[�}���X�i���������p�����[�^
// outRecommendedSettings:���������_�����O�ݒ�p�����[�^
// --------------------------------------------------------
bool DLSSManager::QueryOptimalSettings(
    XMUINT2 inDisplaySize,
    NVSDK_NGX_PerfQuality_Value inQualValue,
    DlssRecommendedSettings* outRecommendedSettings)
{
    
    // NGX������������Ă��Ȃ�������
    if (!m_ngxInitialized)
    {
        outRecommendedSettings->m_ngxRecommendedOptimalRenderSize = inDisplaySize;
        outRecommendedSettings->m_ngxDynamicMaximumRenderSize = inDisplaySize;
        outRecommendedSettings->m_ngxDynamicMinimumRenderSize = inDisplaySize;

        std::cerr << "NGX������������Ă��Ȃ���Ԃł̌Ăяo���ł�" << std::endl;
        return false;
    }

    // �œK�ݒ��K�p
    NVSDK_NGX_Result result = NGX_DLSS_GET_OPTIMAL_SETTINGS(m_ngxParameters,
        inDisplaySize.x, inDisplaySize.y, 
        inQualValue,
        &outRecommendedSettings->m_ngxRecommendedOptimalRenderSize.x, &outRecommendedSettings->m_ngxRecommendedOptimalRenderSize.y,
        &outRecommendedSettings->m_ngxDynamicMaximumRenderSize.x, &outRecommendedSettings->m_ngxDynamicMaximumRenderSize.y,
        &outRecommendedSettings->m_ngxDynamicMinimumRenderSize.x, &outRecommendedSettings->m_ngxDynamicMinimumRenderSize.y,
        &outRecommendedSettings->m_ngxRecommendedSharpness);

    // �ݒ�Ɏ��s�����Ƃ�
    if (NVSDK_NGX_FAILED(result))
    {
        outRecommendedSettings->m_ngxRecommendedOptimalRenderSize = inDisplaySize;
        outRecommendedSettings->m_ngxDynamicMaximumRenderSize = inDisplaySize;
        outRecommendedSettings->m_ngxDynamicMinimumRenderSize = inDisplaySize;
        outRecommendedSettings->m_ngxRecommendedSharpness = 0.0f;

        std::wcerr << L"Optimal Setting�Ɏ��s���܂��� code = 0x"
            << std::hex << result
            << L", info: " << GetNGXResultAsString(result)
            << std::endl;
        return false;
    }

    return true;
}

// �ҋ@
void DLSSManager::Wait()
{
    ID3D11Device* device = Renderer::GetDevice();
    ID3D11DeviceContext* context = Renderer::GetDeviceContext();

    // �K�v�Ȃ�� GPU ��������������܂ł̃N�G����ݒ�
    D3D11_QUERY_DESC queryDesc = { D3D11_QUERY_EVENT, 0 };
    ID3D11Query* query = nullptr;
    device->CreateQuery(&queryDesc, &query);

    context->End(query);
    while (context->GetData(query, nullptr, 0, 0) == S_FALSE) {
        // ��������������܂őҋ@
    }

    query->Release();
}


// DLSS�X�[�p�[�T���v�����O�]��
// --------------------------------------------------------------------------------
// ����
// unresolvedColor:�����O�̃����_�����O�^�[�Q�b�g�i�ʏ�̃J���[�o�́j
// resolvedColor:DLSS�ŃA�b�v�X�P�[�����ꂽ�ŏI�I�ȉ𑜓x�̃����_�����O�^�[�Q�b�g
// motionVectors:���[�V�����x�N�^�[���i�[����e�N�X�`��
// depth:�[�x�����i�[����e�N�X�`��
// bResetAccumulation:�ݐσo�b�t�@�����Z�b�g����t���O�iTAA��DLSS�̍ď������j
// bUseNgxSdkExtApi:�g��API���g�p���邩�̃t���O
// jitterOffset:�W�b�^�[�I�t�Z�b�g�iTAA��DLSS�Ŏg�p�����T�u�s�N�Z���ʒu�j
// mVScale:���[�V�����x�N�^�[�̃X�P�[���l
// --------------------------------------------------------------------------------
bool DLSSManager::EvaluateSuperSampling(
    ID3D11Texture2D* unresolvedColor,
    ID3D11Texture2D* resolvedColor,
    ID3D11Texture2D* motionVectors,
    ID3D11Texture2D* depth,
    ID3D11Texture2D* exposure,
    bool bResetAccumulation,
    bool bUseNgxSdkExtApi,
    XMFLOAT2 jitterOffset,
    XMFLOAT2 mVScale)
{

    // DLSS���g�p�ł��Ȃ���ԂȂ��
    if (!m_dlssAvailable)
    {
        std::cerr << ("DLSS���g�p�ł��܂���@�X�[�p�[�T���v�����O�@�\�������ł�") << std::endl;
        return false;
    }

    // �e���\�[�X�̏�Ԃ��m�F
    if (m_dlssFeature == nullptr)
        std::cerr << "m_dlssFeature is nullptr" << std::endl;
    if (m_ngxParameters == nullptr)
        std::cerr << "m_ngxParameters is nullptr" << std::endl;
    if (unresolvedColor == nullptr)
    std::cerr << "unresolvedColor is nullptr" << std::endl;
    if (resolvedColor == nullptr)
    std::cerr << "resolvedColor is nullptr" << std::endl;
    if (motionVectors == nullptr) 
    std::cerr << "motionVectors is nullptr" << std::endl;
    if (depth == nullptr) 
    std::cerr << "depth is nullptr" << std::endl;


    // �ݐσ��Z�b�g�t���O�𐮐��Ƃ��Đݒ�i1:���Z�b�g�A0:���Z�b�g�Ȃ��j
    int Reset = bResetAccumulation ? 1 : 0;

    // �����_�����O�T�C�Y���擾
    auto renderSize = Renderer::GetInputRenderSize();

    // ���݂̃r���[�|�[�g�̃����_�����O�I�t�Z�b�g��ݒ�i�r���[�|�[�g�̍���j
    // ���݂�[0,0]�ŌŒ�̂��ߒ萔��
    NVSDK_NGX_Coordinates renderingOffset = { 0,0 };

    // ���݂̃r���[�|�[�g�̃����_�����O�T�C�Y��ݒ�i�r���[�|�[�g�̕��ƍ����j
    NVSDK_NGX_Dimensions  renderingSize = 
        { renderSize.x - renderingOffset.X,
          renderSize.y - renderingOffset.Y };

    // Direct3D 11 �̃f�o�C�X�R���e�L�X�g�ƃ��\�[�X���擾
    NVSDK_NGX_Result Result;
    auto deviceContext = Renderer::GetDeviceContext();

    ID3D11Resource* unresolvedColorResource = static_cast<ID3D11Resource*>(unresolvedColor);
    ID3D11Resource* motionVectorsResource = static_cast<ID3D11Resource*>(motionVectors);
    ID3D11Resource* resolvedColorResource = static_cast<ID3D11Resource*>(resolvedColor);
    ID3D11Resource* depthResource = static_cast<ID3D11Resource*>(depth);


    D3D11_TEXTURE2D_DESC desc;
    unresolvedColor->GetDesc(&desc);
    std::cout << "unresolvedColor Format: " << desc.Format << std::endl;
    std::cout << "unresolvedColor BindFlags: " << desc.BindFlags << std::endl;
    std::cout << "unresolvedColor Size: " << desc.Width << " x " << desc.Height << std::endl;

    resolvedColor->GetDesc(&desc);
    std::cout << "resolvedColor Format: " << desc.Format << std::endl;
    std::cout << "resolvedColor BindFlags: " << desc.BindFlags << std::endl;
    std::cout << "resolvedColor Size: " << desc.Width << " x " << desc.Height << std::endl;

    motionVectors->GetDesc(&desc);
    std::cout << "motionVectors Format: " << desc.Format << std::endl;
    std::cout << "motionVectors BindFlags: " << desc.BindFlags << std::endl;
    std::cout << "motionVectors Size: " << desc.Width << " x " << desc.Height << std::endl;

    depth->GetDesc(&desc);
    std::cout << "depth Format: " << desc.Format << std::endl;
    std::cout << "depth BindFlags: " << desc.BindFlags << std::endl;
    std::cout << "depth Size: " << desc.Width << " x " << desc.Height << std::endl;

    std::cout << "Motion Vector Scale: (" << mVScale.x << ", " << mVScale.y << ")" << std::endl;

    // DLSS�]���p�����[�^�\���̂�������
    NVSDK_NGX_D3D11_DLSS_Eval_Params D3D11DlssEvalParams;
    memset(&D3D11DlssEvalParams, 0, sizeof(D3D11DlssEvalParams));

    // �p�����[�^�Ƀ��\�[�X��ݒ�
    D3D11DlssEvalParams.Feature.pInColor            = unresolvedColorResource;
    D3D11DlssEvalParams.Feature.pInOutput           = resolvedColorResource;
    D3D11DlssEvalParams.pInDepth                    = depthResource;
    D3D11DlssEvalParams.pInMotionVectors            = motionVectorsResource;
    //D3D11DlssEvalParams.pInExposureTexture          = exposure;

    D3D11DlssEvalParams.InJitterOffsetX             = jitterOffset.x;
    D3D11DlssEvalParams.InJitterOffsetY             = jitterOffset.y;
    D3D11DlssEvalParams.InReset                     = Reset;
    D3D11DlssEvalParams.InMVScaleX                  = mVScale.x;
    D3D11DlssEvalParams.InMVScaleY                  = mVScale.y;
    D3D11DlssEvalParams.InColorSubrectBase          = renderingOffset;
    D3D11DlssEvalParams.InDepthSubrectBase          = renderingOffset;
    D3D11DlssEvalParams.InTranslucencySubrectBase   = renderingOffset;
    D3D11DlssEvalParams.InMVSubrectBase             = renderingOffset;
    D3D11DlssEvalParams.InRenderSubrectDimensions   = renderingSize;

    // �X�[�p�[�T���v�����O���s
    Result = NGX_D3D11_EVALUATE_DLSS_EXT(
        deviceContext,
        m_dlssFeature, 
        m_ngxParameters, 
        &D3D11DlssEvalParams);

    // ���s�����ꍇ
    if (NVSDK_NGX_FAILED(Result))
    {
        std::wcerr << L"DLSS�X�[�p�[�T���v�����O�Ɏ��s���܂��� code = 0x"
            << std::hex << Result << L", info: "
            << GetNGXResultAsString(Result) << std::endl;
        return false;
    }

    return true;
}