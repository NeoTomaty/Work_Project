#include <d3d11.h>
#include "nvsdk_ngx/nvsdk_ngx.h"
#include "nvsdk_ngx/nvsdk_ngx_helpers.h"
#include "nvsdk_ngx/nvsdk_ngx_defs.h"
#include "nvsdk_ngx/nvsdk_ngx_params.h"
#include <iostream>
#include <DirectXMath.h>

#ifdef NDEBUG
#pragma comment(lib, "nvsdk_ngx/nvsdk_ngx_s.lib")
#else
#pragma comment(lib, "nvsdk_ngx/nvsdk_ngx_s_dbg.lib")
#endif

// NGX�Œ񋟂����p�t�H�[�}���X�̐ݒ�E���䂷��\����
struct PERF_QUALITY_ITEM
{
    NVSDK_NGX_PerfQuality_Value PerfQuality;
    const char*                 PerfQualityText;
    bool                        PerfQualityAllowed;
    bool                        PerfQualityDynamicAllowed;
};

// NGX���񋟂���DLSS�̃����_�����O�v���Z�b�g���Ǘ�����\����
struct RENDER_PRESET_ITEM
{
    NVSDK_NGX_DLSS_Hint_Render_Preset RenderPreset;
    const char* RenderPresetText;
};

// DLSS�̐����ݒ�\����
struct DlssRecommendedSettings  // ~(0u) : �����̒l�iunsigned int�^�S�r�b�g0�̔��]�j
{
    float     m_ngxRecommendedSharpness = 0.0f;                             // ngx sdk 3.1�ł͔p�~�����X�e�[�^�X
    DirectX::XMUINT2 m_ngxRecommendedOptimalRenderSize = { ~(0u), ~(0u) };  // ���������œK�����_�����O�𑜓x
    DirectX::XMUINT2 m_ngxDynamicMaximumRenderSize = { ~(0u), ~(0u) };      // ���I�ɐݒ�\�ȍő僌���_�����O�𑜓x
    DirectX::XMUINT2 m_ngxDynamicMinimumRenderSize = { ~(0u), ~(0u) };      // ���I�ɐݒ�\�ȍŏ������_�����O�𑜓x
};

class DLSSManager
{
private:

    NVSDK_NGX_Parameter* m_ngxParameters = nullptr; // NGX�̃p�����[�^�Ǘ�
    NVSDK_NGX_Handle* m_dlssFeature = nullptr;      // DLSS�@�\�I�u�W�F�N�g
    bool m_ngxInitialized = false;                  // NGX�̏��������
    bool m_dlssAvailable = false;                   // DLSS�̎g�p�ۏ��
    NVSDK_NGX_FeatureDiscoveryInfo dis;             // �@�\���o�p�f�[�^
public:

    // NGX�̏�����
    bool InitializeNGX(const wchar_t* _log);

    // NGX�̃V���b�g�_�E��
    void ShutdownNGX();

    // DLSS�@�\�̏�����
    bool InitializeDLSSFeatures(
        DirectX::XMUINT2 iRenderSize,
        DirectX::XMUINT2 oRenderSize,
        int isHDR,
        bool depthInverted,
        bool enableSharpening,
        bool enableAutoExposure,
        NVSDK_NGX_PerfQuality_Value qualValue,
        unsigned int renderPreset);

    // DLSS�@�\�̉��
    void ReleaseDLSSFeatures();

    // �œK�Ȑݒ�N�G��
    bool QueryOptimalSettings(
        DirectX::XMUINT2 renderSize,
        NVSDK_NGX_PerfQuality_Value inQualValue, 
        DlssRecommendedSettings* outRecommendedSettings);

    // �ҋ@
    void Wait();

    // DLSS�@�\�]��
    bool EvaluateSuperSampling(
        ID3D11Texture2D* unresolvedColor,
        ID3D11Texture2D* resolvedColor,
        ID3D11Texture2D* motionVectors,
        ID3D11Texture2D* depth,
        ID3D11Texture2D* exposure,
        bool bResetAccumulation,
        bool bUseNgxSdkExtApi,
        DirectX::XMFLOAT2 jitterOffset = { 0.0f, 0.0f },
        DirectX::XMFLOAT2 mVScale = { 1.0f, 1.0f });

    // �V���O���g���p�^�[��
    static DLSSManager& GetInstance() {
        static DLSSManager Instance;
        return Instance;
    }
};
