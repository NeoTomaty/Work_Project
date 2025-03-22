#include "StaticMeshRenderer.h"
#include "Application.h"

using namespace DirectX::SimpleMath;

// ������
void StaticMeshRenderer::Init(StaticMesh& mesh, float UseNormal)
{
	// ���_�o�b�t�@�ƃC���f�b�N�X�o�b�t�@�𐶐�
	MeshRenderer::Init(mesh);

	// �T�u�Z�b�g���擾
	m_Subsets = mesh.GetSubsets();

	// �}�e���A�����擾	
	std::vector<MATERIAL> materials;
	materials = mesh.GetMaterials();

	// �s��̏�����
	m_WorldMatrix.Mat = Matrix::Identity;
	m_WorldMatrix.InvMat = Matrix::Identity;
	m_WorldMatrix.PrevMat = Matrix::Identity;


	// �}�e���A���������[�v���ă}�e���A���f�[�^�𐶐�
	for (int i = 0; i < materials.size(); i++)
	{

		// �}�e���A���I�u�W�F�N�g����
		std::unique_ptr<Material> m = std::make_unique<Material>();

		// �}�e���A�������Z�b�g
		m->Create(materials[i]);

		// �}�e���A���I�u�W�F�N�g��z��ɒǉ�
		m_Materiales.push_back(std::move(m));
	}
}

// �X�V
void StaticMeshRenderer::Update()
{

}

// �`��
void StaticMeshRenderer::Draw()
{
	// �s�����O�t���[���ɕۑ�
	m_WorldMatrix.PrevMat = m_WorldMatrix.Mat;

	// ���[���h�ϊ��s����쐬
	Matrix rmtx = Matrix::CreateFromYawPitchRoll(m_Rotation.y, m_Rotation.x, m_Rotation.z);
	Matrix smtx = Matrix::CreateScale(m_Scale.x, m_Scale.y, m_Scale.z);
	Matrix tmtx = Matrix::CreateTranslation(m_Position.x, m_Position.y, m_Position.z);

	Matrix wmtx = smtx * rmtx * tmtx;

	// ���[���h�s����Z�b�g
	m_WorldMatrix.Mat = wmtx;

	// �t�s������߂�
	m_WorldMatrix.InvMat = m_WorldMatrix.Mat.Invert();

	// �萔�o�b�t�@�ɃZ�b�g
	Renderer::SetWorldMatrix(&m_WorldMatrix);

	// �C���f�b�N�X�o�b�t�@�E���_�o�b�t�@���Z�b�g
	BeforeDraw();


	// �}�e���A���������[�v 
	for (int i = 0; i < m_Subsets.size(); i++)
	{
		// �}�e���A�����Z�b�g(�T�u�Z�b�g���̒��ɂ���}�e���A���C���f�b�N���g�p����)
		m_Materiales[m_Subsets[i].MaterialIdx]->SetGPU();

		// �|���S����
		auto num = m_Subsets[i].IndexNum / 3;
		// �`�����Applicaion�Ɋi�[
		Application::CountDrawInfo(num);

		// �T�u�Z�b�g�̕`��
		DrawSubset(
			m_Subsets[i].IndexNum,							// �`�悷��C���f�b�N�X��
			m_Subsets[i].IndexBase,							// �ŏ��̃C���f�b�N�X�o�b�t�@�̈ʒu	
			m_Subsets[i].VertexBase);						// ���_�o�b�t�@�̍ŏ�����g�p
	}
}
