// ���[���h�ϊ��s��
cbuffer WorldBuffer : register(b0)
{
    matrix World;
    matrix InvWorld;
    matrix PrevWorld;
}

// �r���[�ϊ��s��
cbuffer ViewBuffer : register(b1)
{
    matrix View;
    matrix InvView;
    matrix PrevView;
}

// �v���W�F�N�V�����ϊ��s��
cbuffer ProjectionBuffer : register(b2)
{
    matrix Proj;
    matrix InvProj;
    matrix PrevProj;
}

// �}�e���A�����
struct MATERIAL
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emission;
    float Shininess;
    float Alpha;
    float2 Dummy;
};

// �}�e���A���萔
cbuffer MaterialBuffer : register(b3)
{
    MATERIAL Material;
}

// ���C�g���
struct LIGHT
{
    bool Enable;        // �g�p���邩�ۂ�
    bool3 Dummy;        // PADDING
    float3 Position;    // ���C�g�̍��W
    float Dummy2;       // PADDING
    float4 Direction;   // ����
    float4 Diffuse;     // �g�U���˗p�̌��̋���
    float4 Ambient;     // �����p�̌��̋���
};

// ���C�g�萔
cbuffer LightBuffer : register(b4)
{
    LIGHT Light;
};

// �J�����p�̒萔�o�b�t�@�[
cbuffer CameraParam : register(b5)
{
    float3 eyePos;  // �J�����̈ʒu
    float padding;  // �p�f�B���O
    
    float           nearClip;       // �j�A�N���b�v
    float           farClip;        // �t�@�[�N���b�v 
    unsigned int    renderSizeX;    // �����_�����O�T�C�Y�iX�j
    unsigned int    renderSizeY;    // �����_�����O�T�C�Y�iY�j
};

// �e�N�X�`���p�p�����[�^
cbuffer TextureParam : register(b6)
{
    int UseNormalMap;  // �@���}�b�v���g�p���邩�ǂ���
    int TextureType;    // �f�B�t�@�[�h�����_�����O�Ŏg�p����e�N�X�`���^�C�v
    float2 padding2;    // �p�f�B���O�p
}

Texture2D g_Texture : register(t0);     // �A���x�h
Texture2D g_Normal : register(t1);      // �@��
Texture2D g_World : register(t2);       // ���[���h���W
Texture2D g_Depth : register(t3);       // �[�x
Texture2D g_MVector : register(t4);     // ���[�V�����x�N�g��

SamplerState g_SamplerState : register(s0);

struct VS_IN
{
    float4 pos : POSITION0;
    float4 normal : NORMAL0;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    float3 tangent : TANGENT0;
};

struct PS_IN
{
    float4 pos : SV_POSITION0;          // �v���W�F�N�V�����ϊ�����W
    float4 prevPos : POSITION0;         // �O�t���[���̃v���W�F�N�V�����ϊ�����W
    float4 worldPos : POSITION1;        // ���[���h�ϊ�����W
    float4 prevWorldPos : POSITION2;    // �O�t���[���̃��[���h�ϊ�����W
    float4 viewPos : POSITION3;         // �r���[�ϊ�����W
    float4 prevViewPos : POSITION4;     // �O�t���[���̃r���[�ϊ�����W

    
    float3 normal : NORMAL0;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    float3 tangent : TANGENT0;
    float3 biNormal : BINORMAL;
};

// G�o�b�t�@�p�o�͌���
struct PS_OUT
{
    float4 Albedo : SV_TARGET0;    // ��1�̃����_�[�^�[�Q�b�g�iRTV 0�j�ւ̏o��
    float4 Normal : SV_TARGET1;    // ��2�̃����_�[�^�[�Q�b�g�iRTV 1�j�ւ̏o��
    float4 World : SV_TARGET2;     // ��3�̃����_�[�^�[�Q�b�g�iRTV 2�j�ւ̏o��
    float4 Depth : SV_TARGET3;     // ��4�̃����_�[�^�[�Q�b�g�iRTV 3�j�ւ̏o��
    float4 MVector : SV_TARGET4;   // ��5�̃����_�[�^�[�Q�b�g�iRTV 4�j�ւ̏o��
};

// �f�B�t�@�[�h�����_�����O�̍ŏI�o�͌��ʂ�
// �ۑ����邽�߂̃����_�����O��
struct PS_OUT_2
{
    float4 OutputColor : SV_TARGET5; // ��6�̃����_�[�^�[�Q�b�g�iRTV 5�j�ւ̏o��
};



