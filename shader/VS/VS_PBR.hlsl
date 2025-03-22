#include "../Common.hlsli"

PS_IN main(in VS_IN input)
{
    PS_IN output;

    // ���f����� �� ���[���h��ԕϊ�
    float4 pos = float4(input.pos.xyz, 1.0f); // ���_���W�ix,y,z�j���擾���Aw��1.0f�ɂ���
    output.pos = mul(pos, World);                   // ���[���h�s��Ə�Z
    output.worldPos = float4(output.pos.xyz, 1.0f); // ���[���h��Ԃł̒��_�ʒu��ۑ�
    
    // ���[���h��� �� �r���[��� �� �N���b�v��ԕϊ�
    output.pos = mul(output.pos, View);             // �r���[�s��Ə�Z
    output.viewPos = output.pos;                    // �r���[��Ԃł̒��_�ʒu��ۑ�
    output.pos = mul(output.pos, Proj);             // �v���W�F�N�V�����s��Ə�Z�i�N���b�v��ԁj
    
    // ���f����Ԃ̖@�������[���h��Ԃɕϊ�
    output.normal = normalize(mul(input.normal.xyz, (float3x3) World));

    // �ڐ��ƃo�C�m�[�}�������[���h��Ԃɕϊ�
    output.tangent = normalize(mul(input.tangent, (float3x3) World));
    output.biNormal = normalize(cross(output.normal, output.tangent));
    
    // ���̑��̃f�[�^�����̂܂ܓn��
    output.uv = input.uv;
    output.color = input.color;

    return output;
   
}


