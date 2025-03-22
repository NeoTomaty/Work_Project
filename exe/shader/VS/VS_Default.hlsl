#include "../Common.hlsli"


PS_IN main( VS_IN input )
{
    PS_IN output;
	
    float4 pos = float4(input.pos.xyz, 1.0f); // ���_���W�ix,y,z�j���擾���Aw��1.0f�ɂ���
    
    // ���݃t���[���̒��_���W
    output.pos = mul(pos, World);                   // ���[���h�s��Ə�Z
    output.worldPos = float4(output.pos.xyz, 1.0f); // ���[���h��Ԃł̒��_�ʒu��ۑ�
    output.pos = mul(output.pos, View);             // �r���[�s��Ə�Z
    output.viewPos = output.pos;                    // �r���[��Ԃł̒��_�ʒu��ۑ�
    output.pos = mul(output.pos, Proj);             // �v���W�F�N�V�����s��Ə�Z�i�N���b�v��ԁj
    
    // �O�t���[���̒��_���W
    output.prevPos = mul(pos, PrevWorld);                   // �O�t���[���̃��[���h�s��Ə�Z
    output.prevWorldPos = float4(output.prevPos.xyz, 1.0f); // �O�t���[���̃��[���h��Ԃł̒��_�ʒu��ۑ�
    output.prevPos = mul(output.prevPos, PrevView);         // �O�t���[���̃r���[�s��Ə�Z
    output.prevViewPos = output.prevPos;                    // �O�t���[���̃r���[��Ԃł̒��_�ʒu��ۑ�
    output.prevPos = mul(output.prevPos, PrevProj);         // �O�t���[���̃v���W�F�N�V�����s��Ə�Z�i�N���b�v��ԁj

	// �@���x�N�g�������[���h�s��ŕϊ�
    output.normal = mul(input.normal.xyz, (float3x3) World);
	
	// ���_�f�[�^��UV���W�s�N�Z���V�F�[�_�ւ̏o�̓f�[�^�Ƃ��Đݒ�
    output.uv = input.uv;
    
    
    return output;
}